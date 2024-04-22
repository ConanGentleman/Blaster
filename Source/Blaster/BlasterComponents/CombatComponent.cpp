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
//#include "Blaster/HUD/BlasterHUD.h"
#include "Camera/CameraComponent.h"

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
	DOREPLIFETIME(UCombatComponent, bAiming);
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
/// 用于装备武器的变量复制前的调用
/// </summary>
void UCombatComponent::OnRep_EquippedWeapon()
{
	if (EquippedWeapon && Character) {
		//为true时，朝向跟移动方向一致，也就是说角色不会横着走
		Character->GetCharacterMovement()->bOrientRotationToMovement = false;
		//为true,设置角色朝向和Controller的朝向一致。也是朝向和相机一致
		Character->bUseControllerRotationYaw = true;
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
		FHitResult HitResult;
		//准星绘制及射线检测
		TraceUnderCrosshairs(HitResult);

		ServerFire(HitResult.ImpactPoint);

		if (EquippedWeapon)//如果开火并且装备了武器，则让准星收到射击因素的影响为.75f
		{
			CrosshairShootingFactor = .75f;//射击因素直接赋值
		}
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
	if (EquippedWeapon == nullptr) return;
	if (Character) {
		Character->PlayFireMontage(bAiming);
		EquippedWeapon->Fire(TraceHitTarget);
	}
}

/// <summary>
/// 装备(捡起)武器
/// </summary>
/// <param name="WeaponToEquip"></param>
void UCombatComponent::EquipWeapon(AWeapon* WeaponToEquip)
{
	if (Character == nullptr || WeaponToEquip == nullptr) return;

	EquippedWeapon = WeaponToEquip;
	//设置武器状态
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	
	//先获取人物网格的武器插槽
	const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName("RightHandSocket"));
	
	if (HandSocket) {
		//将武器放在人物网格插槽处
		HandSocket->AttachActor(EquippedWeapon, Character->GetMesh());
	}
	//设置武器的所有者
	EquippedWeapon->SetOwner(Character);
	//为true时，朝向跟移动方向一致，也就是说角色不会横着走
	Character->GetCharacterMovement()->bOrientRotationToMovement = false;
	//为true,设置角色朝向和Controller的朝向一致。也是朝向和相机一致
	Character->bUseControllerRotationYaw = true;
}

