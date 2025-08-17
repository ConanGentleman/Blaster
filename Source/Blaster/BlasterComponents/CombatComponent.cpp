// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatComponent.h"
#include "Blaster/Weapon/Weapon.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Components/SphereComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Camera/CameraComponent.h"
#include "TimerManager.h"
#include "Sound/SoundCue.h"
#include "Blaster/Character/BlasterAnimInstance.h"
#include "Blaster/Weapon/Projectile.h"

// Sets default values for this component's properties
UCombatComponent::UCombatComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	//设为false则表示每帧不会执行TickComponent
	PrimaryComponentTick.bCanEverTick = true;
	//bReplicates = true;//由于改变量为私有 无法直接在代码更改，原视频中在角色蓝图中没有说明要勾选CombatComponent组件的“组件复制”,
	//不然服务器无法将复制变量复制到各个客户端

	BaseWalkSpeed = 600.f;
	AimWalkSpeed = 450.f;
}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UCombatComponent, EquippedWeapon);
	DOREPLIFETIME(UCombatComponent, SecondaryWeapon);
	DOREPLIFETIME(UCombatComponent, bAiming);
	//可使用DOREPLIFETIME_CONDITION来指定向那些客户端复制变量
	//DOREPLIFETIME_CONDITION参数：指定角色类（具有复制变量的类），复制变量，条件（这里COND_OwnerOnly，如果你在机器上控制Pawn，那么就是Pawn的Owner
	//当条件设为COND_OwnerOnly，意味着携带的子弹数量将仅复制到当前控制的BlasterCharacter所对应的客户端
	DOREPLIFETIME_CONDITION(UCombatComponent, CarriedAmmo, COND_OwnerOnly);
	DOREPLIFETIME(UCombatComponent, CombatState);
	DOREPLIFETIME(UCombatComponent, Grenades);
}

/// <summary>
/// 霰弹枪子弹装填（蓝图中通过动画通知调用）
/// </summary>
void UCombatComponent::ShotgunShellReload()
{
	if (Character && Character->HasAuthority())
	{
		UpdateShotgunAmmoValues();
	}

}

/// <summary>
/// 捡起子弹 通过AmmoPickup调用
/// </summary>
/// <param name="WeaponType">子弹类型</param>
/// <param name="AmmoAmount">子弹数量</param>
void UCombatComponent::PickupAmmo(EWeaponType WeaponType, int32 AmmoAmount)
{
	//如果包含武器，则更新子弹数量
	if (CarriedAmmoMap.Contains(WeaponType))
	{
		CarriedAmmoMap[WeaponType] = FMath::Clamp(CarriedAmmoMap[WeaponType] + AmmoAmount, 0, MaxCarriedAmmo);
		UpdateCarriedAmmo();
	}
	//如果正好对应类型的武器的弹夹是空的，装填一下子弹
	if (EquippedWeapon && EquippedWeapon->IsEmpty() && EquippedWeapon->GetWeaponType() == WeaponType)
	{
		Reload();
	}
}

// Called when the game starts
void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();
	
	if (Character) {//设置行走速度
		Character->GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
		//如果角色跟随相机存在
		if (Character->GetFollowCamera())
		{
			//则获取相机的默认视野
			DefaultFOV = Character->GetFollowCamera()->FieldOfView;
			CurrentFOV = DefaultFOV;
		}
		//只能由服务器来设置
		if (Character->HasAuthority())
		{
			//初始化角色携带的子弹数量
			InitializeCarriedAmmo();
		}
	}
}

// Called every frame
void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);


	if (Character && Character->IsLocallyControlled())
	{
		//每帧绘制一下枪的线便于观察枪口情况
		FHitResult HitResult;
		TraceUnderCrosshairs(HitResult);
		
		HitTarget = HitResult.ImpactPoint;

		//设置准星的贴图并用于BlasterHUD绘制
		SetHUDCrosshairs(DeltaTime);
		//插值改变相机视野
		InterpFOV(DeltaTime);
	}
}

/// <summary>
/// 按下开火键，调用开火，并且进行射线检测
/// </summary>
/// <param name="bPressed"></param>
void UCombatComponent::FireButtonPressed(bool bPressed)
{
	bFireButtonPressed = bPressed;

	if (bFireButtonPressed) {
		//FHitResult HitResult;
		////准星绘制及射线检测
		//TraceUnderCrosshairs(HitResult);
		Fire();
	}

}

/// <summary>
/// 武器开火
/// </summary>
void UCombatComponent::Fire()
{
	if (CanFire())
	{
		//在下次开火计时完成之前 无法再次开火，所以设置为false
		bCanFire = false;
		ServerFire(HitTarget);
		LocalFire(HitTarget);
		if (EquippedWeapon)//如果开火并且装备了武器，则让准星收到射击因素的影响为.75f
		{
			CrosshairShootingFactor = .75f;//射击因素直接赋值
		}
		StartFireTimer();
	}
}

/// <summary>
/// 启动开火计时器
/// </summary>
void UCombatComponent::StartFireTimer()
{
	if (EquippedWeapon == nullptr || Character == nullptr) return;
	//设置一个定时器，用于开火定时。
	Character->GetWorldTimerManager().SetTimer(
		FireTimer, //定时句柄
		this,    //调用执行（回调）函数的对象
		&UCombatComponent::FireTimerFinished, //执行（回调）函数
		EquippedWeapon->FireDelay //函数执行的时间间隔(也就是武器射速
	);
}

/// <summary>
/// 开火计时器完成函数回调
/// </summary>
void UCombatComponent::FireTimerFinished()
{
	if (EquippedWeapon == nullptr) return;
	bCanFire = true;
	if (bFireButtonPressed && EquippedWeapon->bAutomatic)
	{
		Fire();
	}
	//如果子弹打空了，则自动换弹
	ReloadEmptyWeapon();
}

/// <summary>
/// 开火RPC。用于客户端或服务器调用，服务器执行的武器开火函数。
/// </summary>
/// <param name="TraceHitTarget">用于传递开火后射线检测到的目标位置传递到服务器。FVector_NetQuantize是为了便于网络传输对FVector的封装（序列化），截断小数点，四舍五入取整，使消息大小降低。这里当成正常的FVector即可。</param>
void UCombatComponent::ServerFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	MulticastFire(TraceHitTarget);
}
/// <summary>
/// 服务器执行的多播RPC 开火函数。在服务器上执行多播RPC，那么将在服务器以及所有客户端上调用。（RPC函数的特点：如果客户端调用则在服务器上执行，如果服务器上调用也在服务器上执行，但仅在服务器上执行）。
/// 多播RPC被执行时，会在服务器和所有客户端上执行函数
/// </summary>
/// <param name="TraceHitTarget">用于同步开火后射线检测到的目标位置到服务器盒所有客户端。FVector_NetQuantize是为了便于网络传输对FVector的封装（序列化），截断小数点，四舍五入取整，使消息大小降低。这里当成正常的FVector即可。</param>
void UCombatComponent::MulticastFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	//如果本地角色为客户端角色则返回，
	if (Character && Character->IsLocallyControlled() && !Character->HasAuthority()) return;
	LocalFire(TraceHitTarget);
}

/// <summary>
/// 开火后相关的调用（播放开火蒙太奇、生成子弹、播放音效）
/// </summary>
/// <param name="TraceHitTarget"></param>
void UCombatComponent::LocalFire(const FVector_NetQuantize& TraceHitTarget)
{
	if (EquippedWeapon == nullptr) return;
	//霰弹枪在装弹状态下也能够开火
	if (Character && CombatState == ECombatState::ECS_Reloading && EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Shotgun)
	{
		Character->PlayFireMontage(bAiming);
		EquippedWeapon->Fire(TraceHitTarget);
		CombatState = ECombatState::ECS_Unoccupied;
		return;
	}
	if (Character && CombatState == ECombatState::ECS_Unoccupied) {//保证是空闲状态才能开火
		Character->PlayFireMontage(bAiming);
		EquippedWeapon->Fire(TraceHitTarget);
	}
}

/// <summary>
/// 装备(捡起或替换)武器
/// </summary>
/// <param name="WeaponToEquip"></param>
void UCombatComponent::EquipWeapon(AWeapon* WeaponToEquip)
{
	if (Character == nullptr || WeaponToEquip == nullptr) return;
	if (CombatState != ECombatState::ECS_Unoccupied) return;
	if (EquippedWeapon != nullptr && SecondaryWeapon == nullptr)
	{
		//如果主武器有了则装备到副武器上
		EquipSecondaryWeapon(WeaponToEquip);
	}
	else
	{
		EquipPrimaryWeapon(WeaponToEquip);
	}

	//为true时，朝向跟移动方向一致，也就是说角色不会横着走
	Character->GetCharacterMovement()->bOrientRotationToMovement = false;
	//为true,设置角色朝向和Controller的朝向一致。也是朝向和相机一致
	Character->bUseControllerRotationYaw = true;
}

/// <summary>
/// 切换主副武器
/// </summary>
void UCombatComponent::SwapWeapons()
{
	//不处于非战斗状态则无法切换武器
	if (CombatState != ECombatState::ECS_Unoccupied) return;
	//交互主副武器
	AWeapon* TempWeapon = EquippedWeapon;
	EquippedWeapon = SecondaryWeapon;
	SecondaryWeapon = TempWeapon;
	//交换后的武器附加在右手
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	AttachActorToRightHand(EquippedWeapon);
	EquippedWeapon->SetHUDAmmo();
	UpdateCarriedAmmo();
	PlayEquipWeaponSound(EquippedWeapon);
	//交换后的副武器附加在后背
	SecondaryWeapon->SetWeaponState(EWeaponState::EWS_EquippedSecondary);
	AttachActorToBackpack(SecondaryWeapon);
}

void UCombatComponent::EquipPrimaryWeapon(AWeapon* WeaponToEquip)
{
	if (WeaponToEquip == nullptr) return;
	//丢弃已经装备的武器
	DropEquippedWeapon();
	EquippedWeapon = WeaponToEquip;
	//设置武器状态
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	//将武器网格添加到角色骨骼右手上
	AttachActorToRightHand(EquippedWeapon);

	//设置武器的所有者
	EquippedWeapon->SetOwner(Character);
	//装备武器更新子弹信息显示
	EquippedWeapon->SetHUDAmmo();
	UpdateCarriedAmmo();
	PlayEquipWeaponSound(WeaponToEquip);
	ReloadEmptyWeapon();
}

void UCombatComponent::EquipSecondaryWeapon(AWeapon* WeaponToEquip)
{
	if (WeaponToEquip == nullptr) return;
	SecondaryWeapon = WeaponToEquip;
	SecondaryWeapon->SetWeaponState(EWeaponState::EWS_EquippedSecondary);
	AttachActorToBackpack(WeaponToEquip);
	PlayEquipWeaponSound(WeaponToEquip);

	SecondaryWeapon->SetOwner(Character);

}

void UCombatComponent::DropEquippedWeapon()
{
	if (EquippedWeapon)//如果装备了武器
	{
		EquippedWeapon->Dropped();
	}
}

/// <summary>
/// 将某个actor添加到角色骨骼的右手上（如将武器添加到右手
/// </summary>
/// <param name="ActorToAttach"></param>
void UCombatComponent::AttachActorToRightHand(AActor* ActorToAttach)
{
	if (Character == nullptr || Character->GetMesh() == nullptr || ActorToAttach == nullptr) return;
	//先获取人物网格的武器插槽
	const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName("RightHandSocket"));

	if (HandSocket) {
		//将武器放在人物网格插槽处
		HandSocket->AttachActor(ActorToAttach, Character->GetMesh());
	}
}

/// <summary>
/// 将某个actor添加到角色骨骼的左手上
/// </summary>
/// <param name="ActorToAttach"></param>
void UCombatComponent::AttachActorToLeftHand(AActor* ActorToAttach)
{
	if (Character == nullptr || Character->GetMesh() == nullptr || ActorToAttach == nullptr || EquippedWeapon == nullptr) return;
	bool bUsePistolSocket =
		EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Pistol ||
		EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SubmachineGun; //冲锋枪和手枪使用PistolSocket插槽
	FName SocketName = bUsePistolSocket ? FName("PistolSocket") : FName("LeftHandSocket");
	const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(SocketName);
	if (HandSocket)
	{
		HandSocket->AttachActor(ActorToAttach, Character->GetMesh());
	}
}

/// <summary>
/// 将某个actor添加到角色后背骨骼上
/// </summary>
/// <param name="ActorToAttach"></param>
void UCombatComponent::AttachActorToBackpack(AActor* ActorToAttach)
{
	if (Character == nullptr || Character->GetMesh() == nullptr || ActorToAttach == nullptr) return;
	const USkeletalMeshSocket* BackpackSocket = Character->GetMesh()->GetSocketByName(FName("BackpackSocket"));
	if (BackpackSocket)
	{
		BackpackSocket->AttachActor(ActorToAttach, Character->GetMesh());
	}
}

void UCombatComponent::UpdateCarriedAmmo()
{
	if (EquippedWeapon == nullptr) return;
	//判断是否存在该武器类型，存在则获取携带的子弹数量
	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
	}
	//获取控制器
	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
	if (Controller)
	{
		//设置HUD上的携带子弹数
		Controller->SetHUDCarriedAmmo(CarriedAmmo);
	}
}


void UCombatComponent::PlayEquipWeaponSound(AWeapon* WeaponToEquip)
{
	if (Character && WeaponToEquip && WeaponToEquip->EquipSound)
	{
		//播放装备音效
		UGameplayStatics::PlaySoundAtLocation(
			this,
			WeaponToEquip->EquipSound,
			Character->GetActorLocation()
		);
	}
}

/// <summary>
/// 如果子弹打空了，则自动换弹
/// </summary>
void UCombatComponent::ReloadEmptyWeapon()
{
	//如果子弹打空了，则自动换弹
	if (EquippedWeapon && EquippedWeapon->IsEmpty())
	{
		Reload();
	}
}

/// <summary>
/// 换弹（服务器或客户端都可调用）
/// </summary>
void UCombatComponent::Reload()
{
	//携带子弹大于0且没有装满且处于空闲状态 才调用RPC。不然可能存在一直按R 的情况
	if (CarriedAmmo > 0 && CombatState == ECombatState::ECS_Unoccupied && EquippedWeapon && !EquippedWeapon->IsFull())
	{
		ServerReload();
	}
}
/// <summary>
/// 换弹RPC。仅在服务器执行
/// </summary>
void UCombatComponent::ServerReload_Implementation()
{
	if (Character == nullptr || EquippedWeapon == nullptr) return;

	CombatState = ECombatState::ECS_Reloading;
	HandleReload();
}

/// <summary>
/// 完成换弹调用
/// </summary>
void UCombatComponent::FinishReloading()
{
	if (Character == nullptr) return;
	//由权限才能变换状态
	if (Character->HasAuthority())
	{
		//换弹完成变换状态
		CombatState = ECombatState::ECS_Unoccupied;
		//换弹动作完成才修改武器子弹和携带子弹量
		UpdateAmmoValues();
	}
	//换弹完成后尝试开一下火，有点像预输入
	if (bFireButtonPressed)
	{
		Fire();
	}
}

/// <summary>
/// 换弹更新子弹和携带子弹的值
/// </summary>
void UCombatComponent::UpdateAmmoValues()
{
	if (Character == nullptr || EquippedWeapon == nullptr) return;
	//获取计算能够换弹多少
	int32 ReloadAmount = AmountToReload();
	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		//携带子弹减少
		CarriedAmmoMap[EquippedWeapon->GetWeaponType()] -= ReloadAmount;
		//获取减少后携带的子弹量
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
	}
	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
	if (Controller)
	{
		//更新携带子弹量
		Controller->SetHUDCarriedAmmo(CarriedAmmo);
	}
	//更新武器子弹量
	EquippedWeapon->AddAmmo(-ReloadAmount);
}

/// <summary>
/// 换弹更新子弹和携带子弹的值（一颗一颗装填，用于霰弹枪）
/// </summary>
void UCombatComponent::UpdateShotgunAmmoValues()
{
	if (Character == nullptr || EquippedWeapon == nullptr) return;
	//获取对应类型携带的子弹数
	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		//每次装1颗
		CarriedAmmoMap[EquippedWeapon->GetWeaponType()] -= 1;
		//获取减少后携带的子弹量
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
	}
	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
	if (Controller)
	{
		//更新携带子弹量
		Controller->SetHUDCarriedAmmo(CarriedAmmo);
	}
	//更新武器子弹量
	EquippedWeapon->AddAmmo(-1);
	//换弹期间可以开火
	bCanFire = true;
	//判断是否装满或者携带的子弹数为0，则停止装弹
	if (EquippedWeapon->IsFull() || CarriedAmmo == 0)
	{
		JumpToShotgunEnd();
	}
}

/// <summary>
/// 变量Grenades被赋值时触发(其他调用UpdateHUDGrenades函数的地方都只在服务器，所以这里用复制变量的方式通知客户端去调一下以刷新UI
/// </summary>
void UCombatComponent::OnRep_Grenades()
{
	UpdateHUDGrenades();
}

/// <summary>
/// 停止装弹动画（因为霰弹枪的子弹上限是4，因此将4次一颗一颗装弹分成4次装弹动画，当不能再装弹时，则停止播放装弹动画
/// </summary>
void UCombatComponent::JumpToShotgunEnd()
{
	// Jump to ShotgunEnd section
	UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance();
	if (AnimInstance && Character->GetReloadMontage())
	{
		//将动画跳到停止动画处进行播放
		AnimInstance->Montage_JumpToSection(FName("ShotgunEnd"));
	}
}

/// <summary>
/// 完成投掷手榴弹回调
/// </summary>
void UCombatComponent::ThrowGrenadeFinished()
{
	//完成手榴弹投掷后则将战斗状态改为空闲
	CombatState = ECombatState::ECS_Unoccupied;
	//用于投掷手榴弹动作开启时会将武器绑定到左手，因此当投掷完成后绑定回右手
	AttachActorToRightHand(EquippedWeapon);
}

/// <summary>
/// 发射手榴弹（由grenadetoss中的动画通知调用）
/// </summary>
void UCombatComponent::LaunchGrenade()
{
	//隐藏手上的手榴弹
	ShowAttachedGrenade(false);
	//准星目标是跟本地操控的玩家有关
	if (Character && Character->IsLocallyControlled())
	{
		ServerLaunchGrenade(HitTarget);
	}
}

/// <summary>
/// 发射手榴弹RPC
/// RPC用于客户端调用服务器执行的函数，保证客户端同样能够获取到目标来生成并投出手榴弹
/// </summary>
/// <param name="Target">FVector_NetQuantize 类型用于网络同步，且精度没有FVector高但是网络带宽占用较小 UPROPERTY(Replicated) 或其他复制逻辑（如 RPC）使用</param>
void UCombatComponent::ServerLaunchGrenade_Implementation(const FVector_NetQuantize & Target)
{
	if (Character && Character->HasAuthority() && GrenadeClass && Character->GetAttachedGrenade())
	{
		const FVector StartingLocation = Character->GetAttachedGrenade()->GetComponentLocation(); //手榴弹开始的位置
		FVector ToTarget = Target - StartingLocation; //射线检测到的目标位置（准星瞄准方向
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = Character; //拥有者
		SpawnParams.Instigator = Character; //引发者（造成影响的人，造成伤害者
		UWorld* World = GetWorld();
		if (World)
		{
			World->SpawnActor<AProjectile>( //生成手榴弹
				GrenadeClass,
				StartingLocation,
				ToTarget.Rotation(),
				SpawnParams
				);
		}
	}
}

/// <summary>
/// 战斗状态变量复制时调用
/// </summary>
void UCombatComponent::OnRep_CombatState()
{
	//根据状态类型做不同行为
	switch (CombatState)
	{
	case ECombatState::ECS_Reloading:
		HandleReload();
		break;
	case ECombatState::ECS_Unoccupied:
		//如果换弹完成并且按着左键，则开火
		if (bFireButtonPressed)
		{
			Fire();
		}
		break;
	case ECombatState::ECS_ThrowingGrenade: 
		if (Character && !Character->IsLocallyControlled())//如果是本地控制的角色肯定已经播放过玩家的投掷动画了，因此只处理不是本地控制的角色播放动画蒙太奇
		{
			Character->PlayThrowGrenadeMontage();
			//投掷手榴弹会用右手投掷，但装备的武器也是绑定在右手，因此当投掷手榴弹时将武器绑定到左手去，并在投掷完成后通过ThrowGrenadeFinished将武器绑定回右手
			AttachActorToLeftHand(EquippedWeapon);
			//投掷手榴弹时显示手榴弹
			ShowAttachedGrenade(true);
		}
		break;
	}
}

/// <summary>
/// 处理发生在所有机器上的事情。（如播放换弹蒙太奇动画
/// </summary>
void UCombatComponent::HandleReload()
{
	Character->PlayReloadMontage();
}

/// <summary>
/// 计算武器换弹能换多少子弹量
/// </summary>
/// <returns></returns>
int32 UCombatComponent::AmountToReload()
{
	if (EquippedWeapon == nullptr) return 0;
	//容量-当前子弹量=可装填的子弹量
	int32 RoomInMag = EquippedWeapon->GetMagCapacity() - EquippedWeapon->GetAmmo();
	//携带有同类型的子弹
	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		//获取所携带的子弹量
		int32 AmountCarried = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
		//比较携带的和所需的那个小
		int32 Least = FMath::Min(RoomInMag, AmountCarried);
		//获得最终可换弹的子弹数量
		return FMath::Clamp(RoomInMag, 0, Least);
	}
	return 0;
}

/// <summary>
/// 投掷手榴弹 
/// </summary>
void UCombatComponent::ThrowGrenade()
{
	if (Grenades == 0) return;
	if (CombatState != ECombatState::ECS_Unoccupied || EquippedWeapon == nullptr) return;
	//将当前战斗状态变成投掷状态
	CombatState = ECombatState::ECS_ThrowingGrenade;
	if (Character)
	{
		Character->PlayThrowGrenadeMontage(); //播放蒙太奇动画
		//投掷手榴弹会用右手投掷，但装备的武器也是绑定在右手，因此当投掷手榴弹时将武器绑定到左手去，并在投掷完成后通过ThrowGrenadeFinished将武器绑定回右手
		AttachActorToLeftHand(EquippedWeapon);
		//投掷手榴弹时显示手榴弹
		ShowAttachedGrenade(true);
	}
	if (Character && !Character->HasAuthority())//如果是客户端则调用投掷手榴弹RPC，让服务器也执行一下
	{
		ServerThrowGrenade();
	}
	if (Character && Character->HasAuthority())
	{
		Grenades = FMath::Clamp(Grenades - 1, 0, MaxGrenades);
		UpdateHUDGrenades();
	}
}

/// <summary>
/// 投掷手榴弹RPC  用于客户端调用服务器执行的函数
/// </summary>
void UCombatComponent::ServerThrowGrenade_Implementation()
{
	if (Grenades == 0) return;
	CombatState = ECombatState::ECS_ThrowingGrenade;
	if (Character)
	{
		Character->PlayThrowGrenadeMontage();
		//投掷手榴弹会用右手投掷，但装备的武器也是绑定在右手，因此当投掷手榴弹时将武器绑定到左手去，并在投掷完成后通过ThrowGrenadeFinished将武器绑定回右手
		AttachActorToLeftHand(EquippedWeapon);
		//投掷手榴弹时显示手榴弹
		ShowAttachedGrenade(true);
	}
	Grenades = FMath::Clamp(Grenades - 1, 0, MaxGrenades);
	UpdateHUDGrenades();
}

void UCombatComponent::UpdateHUDGrenades()
{
	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
	if (Controller)
	{
		Controller->SetHUDGrenades(Grenades);
	}
}

bool UCombatComponent::ShouldSwapWeapons()
{
	return (EquippedWeapon != nullptr && SecondaryWeapon != nullptr);
}

/// <summary>
/// 设置手上手榴弹显示or隐藏（一般是扔手榴弹的时候显示，扔完就隐藏掉
/// </summary>
/// <param name="bShowGrenade"></param>
void UCombatComponent::ShowAttachedGrenade(bool bShowGrenade)
{
	if (Character && Character->GetAttachedGrenade())
	{
		Character->GetAttachedGrenade()->SetVisibility(bShowGrenade);
	}
}


/// <summary>
/// 用于装备武器的变量复制前的调用
/// </summary>
void UCombatComponent::OnRep_EquippedWeapon()
{
	if (EquippedWeapon && Character) {
		//武器状态和装备武器都属于复制变量，这里我们无法确保到底哪个复制会到达各个客户端，由于武器状态的修改会导致武器本身物理现象的改变（在Weapon中的SetWeaponState可知，装备时是没有物理的，没装备时是有物理的）就有可能与当前情况不匹配，导致装备武器的交互出现差错。
		//因此为了避免伤处情况。选择在OnRep_EquippedWeapon将武器附加到客户端上。因为武器状态处理碰撞属性，如果视图捡起已经掉落的武器，他会进行物理模拟和启用物理功能，则无法将装备武器的附加到角色上。
		//所以先在客户端设置装备状态
		EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
		AttachActorToRightHand(EquippedWeapon);

		//为true时，朝向跟移动方向一致，也就是说角色不会横着走
		Character->GetCharacterMovement()->bOrientRotationToMovement = false;
		//为true,设置角色朝向和Controller的朝向一致。也是朝向和相机一致
		Character->bUseControllerRotationYaw = true;

		//播放装备音效
		PlayEquipWeaponSound(EquippedWeapon);
		//装备武器禁用其深度颜色（轮廓）
		EquippedWeapon->EnableCustomDepth(false);
		EquippedWeapon->SetHUDAmmo();
	}
}

/// <summary>
/// 用于装备武器的变量复制前的调用（副武器用）
/// </summary>
void UCombatComponent::OnRep_SecondaryWeapon()
{
	if (SecondaryWeapon && Character)
	{
		SecondaryWeapon->SetWeaponState(EWeaponState::EWS_EquippedSecondary);
		// 附加到背骨骼上
		AttachActorToBackpack(SecondaryWeapon);
		//播放装备音效
		PlayEquipWeaponSound(EquippedWeapon);
		
	}
}

/// <summary>
/// 从屏幕中心发射射线，进行用于射击的射线检测
/// </summary>
/// <param name="TraceHitResult">命中信息</param>
void UCombatComponent::TraceUnderCrosshairs(FHitResult& TraceHitResult)
{
	//为了从屏幕中心进行追踪，需要视口大小
	FVector2D ViewportSize;
	if (GEngine && GEngine->GameViewport)//检查游戏视口是否有效
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}
	//准星中心屏幕坐标位置 
	FVector2D CrosshairLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;
	//将准星的屏幕坐标转换为世界坐标位置。 返回值：转换是否成功
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(this, 0),//当前控制的玩家。参数：世界；第几个控制器
		CrosshairLocation,
		CrosshairWorldPosition,
		CrosshairWorldDirection
	);
	//进行射线检测
	if (bScreenToWorld)
	{
		//射线起始点 
		FVector Start = CrosshairWorldPosition;


		if (Character)
		{
			//由于射线检测的起始点为相机，所以当敌方角色出现在相机和控制的角色之间时，也会被检测到使得准心变色。
			//因此将射线检测的起始点加上相机与角色的距离，以使得射线检测以人物为起始点进行发射
			float DistanceToCharacter = (Character->GetActorLocation() - Start).Size();
			//这里+100.f是为了不让你射线检测角色自己
			Start += CrosshairWorldDirection * (DistanceToCharacter + 100.f);
			////调试绘制查看起始点的位置
			//DrawDebugSphere(GetWorld(), Start, 16.f, 12, FColor::Red, false);
		}

		//射线终点
		FVector End = Start + CrosshairWorldDirection * TRACE_LENGTH;
		//调用射线检测。参数：检测结果，开始位置，结束位置，碰撞通道
		GetWorld()->LineTraceSingleByChannel(
			TraceHitResult,
			Start,
			End,
			ECollisionChannel::ECC_Visibility
		);
		if (!TraceHitResult.bBlockingHit) //如果没有碰撞到
		{
			TraceHitResult.ImpactPoint = End; //设置撞击点就是End
		}
		//判断是否射线检测击中了玩家角色，并且角色是否实现了UInteractWithCrosshairsInterface接口
		if (TraceHitResult.GetActor() && TraceHitResult.GetActor()->Implements<UInteractWithCrosshairsInterface>())
		{
			//将准星变为红色
			HUDPackage.CrosshairsColor = FLinearColor::Red;
		}
		else
		{
			HUDPackage.CrosshairsColor = FLinearColor::White;
		}
	}
}

/// <summary>
/// 设置准星的贴图并用于BlasterHUD绘制
/// </summary>
/// <param name="DeltaTime"></param>
void UCombatComponent::SetHUDCrosshairs(float DeltaTime)
{
	if (Character == nullptr || Character->Controller == nullptr) return;
	//获取玩家控制器
	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
	if (Controller)
	{
		//获取控制器对应的HUD
		HUD = HUD == nullptr ? Cast<ABlasterHUD>(Controller->GetHUD()) : HUD;
		if (HUD)
		{
			//设置准星贴图结构体变量
			if (EquippedWeapon)
			{
				HUDPackage.CrosshairsCenter = EquippedWeapon->CrosshairsCenter;
				HUDPackage.CrosshairsLeft = EquippedWeapon->CrosshairsLeft;
				HUDPackage.CrosshairsRight = EquippedWeapon->CrosshairsRight;
				HUDPackage.CrosshairsBottom = EquippedWeapon->CrosshairsBottom;
				HUDPackage.CrosshairsTop = EquippedWeapon->CrosshairsTop;
			}
			else
			{
				HUDPackage.CrosshairsCenter = nullptr;
				HUDPackage.CrosshairsLeft = nullptr;
				HUDPackage.CrosshairsRight = nullptr;
				HUDPackage.CrosshairsBottom = nullptr;
				HUDPackage.CrosshairsTop = nullptr;
			}
			///计算准星分布

			//角色速度范围
			FVector2D WalkSpeedRange(0.f, Character->GetCharacterMovement()->MaxWalkSpeed);
			//速度映射范围
			FVector2D VelocityMultiplierRange(0.f, 1.f);
			//获取当前角色速度
			FVector Velocity = Character->GetVelocity();
			//不关心z轴方向速度
			Velocity.Z = 0.f;

			//将速度映射为0到1之间的数。
			CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(WalkSpeedRange, VelocityMultiplierRange, Velocity.Size());
			
			if (Character->GetCharacterMovement()->IsFalling())//判断是否在空中。
			{
				//FInterpTo参数：当前值，目标值，每帧时间，插值速度
				CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 2.25f, DeltaTime, 2.25f);
			}
			else//没在空中则需要还原到0
			{
				CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 0.f, DeltaTime, 30.f);
			}
			////将速度和空中两个影响的扩散进行叠加得出最终的影响结果（由于新增了瞄准和射击的因素 因此被弃用
			//HUDPackage.CrosshairSpread = CrosshairVelocityFactor + CrosshairInAirFactor;
			if (bAiming)
			{
				//如果正在瞄准，则插值瞄准因素
				CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.58f, DeltaTime, 30.f);
			}
			else
			{
				//没在瞄准，则插值还原为0
				CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.f, DeltaTime, 30.f);
			}
			//插值射击变为0
			CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.f, DeltaTime, 40.f);
			//将速度、跳跃、瞄准、射击四个对准星缩放的影响进行总和
			HUDPackage.CrosshairSpread =
				0.5f + //0.5是因为瞄准的因素会使准星缩放过于小了，因此添加一个基准值避免这种情况发生
				CrosshairVelocityFactor +
				CrosshairInAirFactor - //准星是缩小，所以用-
				CrosshairAimFactor +
				CrosshairShootingFactor;


			//设置准星贴图
			HUD->SetHUDPackage(HUDPackage);
		}
	}
}

/// <summary>
/// 插值改变视野
/// </summary>
/// <param name="DeltaTime"></param>
void UCombatComponent::InterpFOV(float DeltaTime)
{
	if (EquippedWeapon == nullptr) return;
	
	if (bAiming)
	{
		//装备并瞄准时，插值改变视野
		CurrentFOV = FMath::FInterpTo(CurrentFOV, EquippedWeapon->GetZoomedFOV(), DeltaTime, EquippedWeapon->GetZoomInterpSpeed());
	}
	else
	{
		//插值还原为视野
		CurrentFOV = FMath::FInterpTo(CurrentFOV, DefaultFOV, DeltaTime, ZoomInterpSpeed);
	}
	if (Character && Character->GetFollowCamera())
	{
		//设置相机视野
		Character->GetFollowCamera()->SetFieldOfView(CurrentFOV);
	}
}


void UCombatComponent::SetAiming(bool bIsAiming)
{
	if (Character == nullptr || EquippedWeapon == nullptr) return;
	bAiming = bIsAiming;
	//UE_LOG(LogTemp, Warning, TEXT("bAiming : %d"), bAiming);
	//如果没有服务器权限，则表示是客户端调用的，因此需要调用RPC函数。
	// 由于RPC函数的特性，在服务器上调用ServerSetAiming也只是是在服务器上执行ServerSetAiming（相当于就是一个函数调用）；在客户端调用也是在服务器上执行ServerSetAiming
	//所以就直接去掉了HasAuthority()的判断
	//把上一行bAiming = bIsAiming;留下来的原因是，在客户端调用RPC必须等待RPC到达服务器并且将bAiming复制回来，存在一定时间，我们可以立即将其设置，反正复制回来的Aiming的值是一致的
	ServerSetAiming(bIsAiming);
	//当设置瞄准时修改人物移动速度
	if (Character) {//设置行走速度 (应该是
		Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
	}
	if (Character->IsLocallyControlled() && EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SniperRifle) //如果角色受本地控制，且装备的是狙击枪
	{
		Character->ShowSniperScopeWidget(bIsAiming);
	}
}
/// <summary>
/// 用于客户端调用服务器执行的函数
/// </summary>
/// <param name="bIsAiming"></param>
void UCombatComponent::ServerSetAiming_Implementation(bool bIsAiming)
{
	bAiming = bIsAiming;
	//UE_LOG(LogTemp, Warning, TEXT("bAimingServer : %d"), bAiming);
	//当设置瞄准时修改人物移动速度
	if (Character) {//设置行走速度 (应该是
		Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
	}
}


/// <summary>
/// 判断是否能够开火
/// </summary>
/// <returns></returns>
bool UCombatComponent::CanFire()
{
	if (EquippedWeapon == nullptr) return false;
	//霰弹枪在装弹过程中仍然可以开枪射击
	if (!EquippedWeapon->IsEmpty() && bCanFire && CombatState == ECombatState::ECS_Reloading && EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Shotgun) return true;
	//判断武器子弹是否不为空 且 能开火 且处于空闲状态（即不运行在装弹时开火）
	return !EquippedWeapon->IsEmpty() && bCanFire && CombatState == ECombatState::ECS_Unoccupied;
}
/// <summary>
/// 携带子弹数量复制时调用
/// </summary>
void UCombatComponent::OnRep_CarriedAmmo()
{
	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
	if (Controller)
	{
		Controller->SetHUDCarriedAmmo(CarriedAmmo);
		bool bJumpToShotgunEnd =
			CombatState == ECombatState::ECS_Reloading && //处于装弹状态
			EquippedWeapon != nullptr &&
			EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Shotgun &&//武器是霰弹枪
			CarriedAmmo == 0;
		if (bJumpToShotgunEnd) //携带子弹数量为0时，停止播放装弹动画（霰弹枪用）
		{
			JumpToShotgunEnd();
		}
	}
}

/// <summary>
/// 初始化携带的子弹数量
/// </summary>
void UCombatComponent::InitializeCarriedAmmo()
{
	//想map中添加武器类型及携带的子弹数量
	CarriedAmmoMap.Emplace(EWeaponType::EWT_AssaultRifle, StartingARAmmo);

	CarriedAmmoMap.Emplace(EWeaponType::EWT_RocketLauncher, StartingRocketAmmo);

	CarriedAmmoMap.Emplace(EWeaponType::EWT_Pistol, StartingPistolAmmo);

	CarriedAmmoMap.Emplace(EWeaponType::EWT_SubmachineGun, StartingSMGAmmo);

	CarriedAmmoMap.Emplace(EWeaponType::EWT_Shotgun, StartingShotgunAmmo);

	CarriedAmmoMap.Emplace(EWeaponType::EWT_SniperRifle, StartingShotgunAmmo);

	CarriedAmmoMap.Emplace(EWeaponType::EWT_GrenadeLauncher, StartingGrenadeLauncherAmmo);
}