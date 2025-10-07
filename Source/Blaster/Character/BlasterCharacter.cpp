// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/WidgetComponent.h"
#include "Net/UnrealNetwork.h"
#include "Blaster/Weapon/Weapon.h"
#include "Blaster/BlasterComponents/CombatComponent.h"
#include "Blaster/BlasterComponents/BuffComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "BlasterAnimInstance.h"
#include "Blaster/Blaster.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Blaster/GameMode/BlasterGameMode.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Particles/ParticleSystemComponent.h"
#include "Blaster/PlayerState/BlasterPlayerState.h"
#include "Blaster/Weapon/WeaponTypes.h"
#include "Components/BoxComponent.h"
#include "Blaster/BlasterComponents/LagCompensationComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Blaster/GameState/BlasterGameState.h"

// Sets default values
ABlasterCharacter::ABlasterCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true; 
	////这样设置是保证在重生点发生碰撞重合时，仍然生成角色。 视频这里添加后上除了，而在角色蓝图中进行的设置
	//SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	//创建默认对象并命名为CameraBoom
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	//将弹簧臂连接到网格组件（通常是跟组件，但是人物有蹲下操作，会改变胶囊体的大小，这样tan'huan
	CameraBoom->SetupAttachment(GetMesh());
	//设计弹簧臂长
	CameraBoom->TargetArmLength = 600.f;
	//是否由Pawn控制旋转 (这样用于鼠标输入控制
	CameraBoom->bUsePawnControlRotation = true;

	//创建相机
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	//将相机连接到弹簧臂上的相机插槽上
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	//不需要使用Pawn控制旋转，因为连接到了
	FollowCamera->bUsePawnControlRotation = false;

	//不让角色随着控制器一起旋转(以免后续武器瞄准更为复杂）
	bUseControllerRotationYaw = false;
	//让角色朝向自己的运动方向
	GetCharacterMovement()->bOrientRotationToMovement = true;
	//将信息界面显示在角色上
	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	//添加到跟组件上
	OverheadWidget->SetupAttachment(RootComponent);

	Combat1 = CreateDefaultSubobject<UCombatComponent>(TEXT("Combat1"));
	///使其可复制，由于组件有些特殊，因此并不需要在getlifetimereplicatedprops中注册。直接设置即可
	Combat1->SetIsReplicated(true);

	//创建buff组件
	Buff = CreateDefaultSubobject<UBuffComponent>(TEXT("BuffComponent"));
	//设置为复制
	Buff->SetIsReplicated(true);

	//创建延迟补偿组件
	LagCompensation = CreateDefaultSubobject<ULagCompensationComponent>(TEXT("LagCompensation"));

	//将该变量设置为true才能正常蹲下，因为Crouch函数会依据这个
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
	///下面两行防止两个角色相交时，胶囊体碰撞导致相机视角改变
	//解决胶囊与相机碰撞的问题
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	//设置碰撞类型为自定义类型。 由于在子弹类中，设置了只与地面、可见性和自定义类型ECC_SkeletalMesh会发生碰撞。
	//因此将角色网格体（3d模型）的object对象类型设为ECC_SkeletalMesh，使得子弹与角色模型而不是角色的胶囊体进行碰撞。
	//这样就可以进一步区分碰撞的位置（如爆头、身子、脚）来做差异化处理
	GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh);
	//解决角色网格与相机的碰撞问题
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	//让这样设置的目的是为了让射线检测能够检测到角色网格，以便准星变色
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	//设置角色的旋转速度
	GetCharacterMovement()->RotationRate = FRotator(0.f, 0.f, 850.f);
	//初始化转向为不转
	TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	//设置网络更新的频率（也就是每秒总共发送多少帧数据）
	NetUpdateFrequency = 66.f; //默认是100；
	//设置最小网络更新频率（用于决定在复制属性很少发生变化时的节流率
	MinNetUpdateFrequency = 33.f;//默认是2

	//创建溶解时间轴组件
	DissolveTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("DissolveTimelineComponent"));

	//创建手榴弹网格
	AttachedGrenade = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Attached Grenade"));
	AttachedGrenade->SetupAttachment(GetMesh(), FName("GrenadeSocket"));
	// 设置手榴弹碰撞
	AttachedGrenade->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	/**
	* 服务器端倒带的命中框(碰撞体）  命名大部分跟骨骼名一致
	* 即用于延迟补偿计算的碰撞框
	* 关闭碰撞检查，只适用于延迟补偿时，对某时刻角色身上相应位置记录来进行射线检测
	*/

	head = CreateDefaultSubobject<UBoxComponent>(TEXT("head"));
	//附加到网格体组件上名字为head的插槽上
	head->SetupAttachment(GetMesh(), FName("head"));
	HitCollisionBoxes.Add(FName("head"), head);

	pelvis = CreateDefaultSubobject<UBoxComponent>(TEXT("pelvis"));
	pelvis->SetupAttachment(GetMesh(), FName("pelvis"));
	HitCollisionBoxes.Add(FName("pelvis"), pelvis);

	spine_02 = CreateDefaultSubobject<UBoxComponent>(TEXT("spine_02"));
	spine_02->SetupAttachment(GetMesh(), FName("spine_02"));
	HitCollisionBoxes.Add(FName("spine_02"), spine_02);

	spine_03 = CreateDefaultSubobject<UBoxComponent>(TEXT("spine_03"));
	spine_03->SetupAttachment(GetMesh(), FName("spine_03"));
	HitCollisionBoxes.Add(FName("spine_03"), spine_03);

	upperarm_l = CreateDefaultSubobject<UBoxComponent>(TEXT("upperarm_l"));
	upperarm_l->SetupAttachment(GetMesh(), FName("upperarm_l"));
	HitCollisionBoxes.Add(FName("upperarm_l"), upperarm_l);

	upperarm_r = CreateDefaultSubobject<UBoxComponent>(TEXT("upperarm_r"));
	upperarm_r->SetupAttachment(GetMesh(), FName("upperarm_r"));
	HitCollisionBoxes.Add(FName("upperarm_r"), upperarm_r);

	lowerarm_l = CreateDefaultSubobject<UBoxComponent>(TEXT("lowerarm_l"));
	lowerarm_l->SetupAttachment(GetMesh(), FName("lowerarm_l"));
	HitCollisionBoxes.Add(FName("lowerarm_l"), lowerarm_l);

	lowerarm_r = CreateDefaultSubobject<UBoxComponent>(TEXT("lowerarm_r"));
	lowerarm_r->SetupAttachment(GetMesh(), FName("lowerarm_r"));
	HitCollisionBoxes.Add(FName("lowerarm_r"), lowerarm_r);

	hand_l = CreateDefaultSubobject<UBoxComponent>(TEXT("hand_l"));
	hand_l->SetupAttachment(GetMesh(), FName("hand_l"));
	HitCollisionBoxes.Add(FName("hand_l"), hand_l);

	hand_r = CreateDefaultSubobject<UBoxComponent>(TEXT("hand_r"));
	hand_r->SetupAttachment(GetMesh(), FName("hand_r"));
	HitCollisionBoxes.Add(FName("hand_r"), hand_r);

	blanket = CreateDefaultSubobject<UBoxComponent>(TEXT("blanket"));
	blanket->SetupAttachment(GetMesh(), FName("backpack"));
	HitCollisionBoxes.Add(FName("blanket"), blanket);

	backpack = CreateDefaultSubobject<UBoxComponent>(TEXT("backpack"));
	backpack->SetupAttachment(GetMesh(), FName("backpack"));
	HitCollisionBoxes.Add(FName("backpack"), backpack);

	thigh_l = CreateDefaultSubobject<UBoxComponent>(TEXT("thigh_l"));
	thigh_l->SetupAttachment(GetMesh(), FName("thigh_l"));
	HitCollisionBoxes.Add(FName("thigh_l"), thigh_l);

	thigh_r = CreateDefaultSubobject<UBoxComponent>(TEXT("thigh_r"));
	thigh_r->SetupAttachment(GetMesh(), FName("thigh_r"));
	HitCollisionBoxes.Add(FName("thigh_r"), thigh_r);

	calf_l = CreateDefaultSubobject<UBoxComponent>(TEXT("calf_l"));
	calf_l->SetupAttachment(GetMesh(), FName("calf_l"));
	HitCollisionBoxes.Add(FName("calf_l"), calf_l);

	calf_r = CreateDefaultSubobject<UBoxComponent>(TEXT("calf_r"));
	calf_r->SetupAttachment(GetMesh(), FName("calf_r"));
	HitCollisionBoxes.Add(FName("calf_r"), calf_r);

	foot_l = CreateDefaultSubobject<UBoxComponent>(TEXT("foot_l"));
	foot_l->SetupAttachment(GetMesh(), FName("foot_l"));
	HitCollisionBoxes.Add(FName("foot_l"), foot_l);

	foot_r = CreateDefaultSubobject<UBoxComponent>(TEXT("foot_r"));
	foot_r->SetupAttachment(GetMesh(), FName("foot_r"));
	HitCollisionBoxes.Add(FName("foot_r"), foot_r);


	for (auto Box : HitCollisionBoxes)
	{
		if (Box.Value)
		{
			Box.Value->SetCollisionObjectType(ECC_HitBox);
			//忽略除ECC_HitBox外所有频道的碰撞相应
			Box.Value->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			Box.Value->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);

			Box.Value->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}
}
/// <summary>
/// 函数内部是注册要replicated（复制）的变量的地方。便于将服务器上的replicated变量同步到各个客户端
/// </summary>
void ABlasterCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	//注册要replicated（复制）的OverlappingWeapon.
	//DOREPLIFETIME参数：指定角色类（具有复制变量的类），复制变量
	//在角色这里设置OverlappingWeapon为空，并在武器类中的重叠函数中使用本类中的SetOverlappingWeapon设置它,
	//因此一旦重叠武器（OverlappingWeapon）的值发生变化，他就会在各个客户端上复制，
	//DOREPLIFETIME(ABlasterCharacter, OverlappingWeapon);
// 
	//可使用DOREPLIFETIME_CONDITION来指定向那些客户端复制变量
	//DOREPLIFETIME_CONDITION参数：指定角色类（具有复制变量的类），复制变量，条件（这里COND_OwnerOnly，如果你在机器上控制Pawn，那么就是Pawn的Owner
	//当条件设为COND_OwnerOnly，意味着重叠武器将仅复制到当前控制的BlasterCharacter所对应的客户端
	DOREPLIFETIME_CONDITION(ABlasterCharacter, OverlappingWeapon,COND_OwnerOnly);

	DOREPLIFETIME(ABlasterCharacter, Health);
	DOREPLIFETIME(ABlasterCharacter, Shield);
	DOREPLIFETIME(ABlasterCharacter, bDisableGameplay);
}

void ABlasterCharacter::OnRep_ReplicatedMovement()
{
	Super::OnRep_ReplicatedMovement();
	SimProxiesTurn();
	TimeSinceLastMovementReplication = 0.f;
}

/// <summary>
/// 角色淘汰或死亡的处理函数。（仅用于服务器上调用）。
/// 可以在这里处理一些游戏模式相关的东西（例如重生）。因为游戏模式仅存于服务器上。
/// </summary>
void ABlasterCharacter::Elim(bool bPlayerLeftGame)
{
	DropOrDestroyWeapons();
	MulticastElim(bPlayerLeftGame);
}

/// <summary>
/// 角色淘汰（死亡）。多播RPC，客户端调用，服务器执行并让各个客户端同步执行。
/// </summary>
void ABlasterCharacter::MulticastElim_Implementation(bool bPlayerLeftGame)
{
	bLeftGame = bPlayerLeftGame;
	if (BlasterPlayerController)
	{
		//设置子弹数量
		BlasterPlayerController->SetHUDWeaponAmmo(0);
	}
	bElimmed = true;
	PlayElimMontage();
	//启用角色材质溶解
	if (DissolveMaterialInstance)
	{
		//创建动态材质实例
		DynamicDissolveMaterialInstance = UMaterialInstanceDynamic::Create(DissolveMaterialInstance, this);
		//设置动态材质到网格的上索引为0的位置
		GetMesh()->SetMaterial(0, DynamicDissolveMaterialInstance);
		//设置动态材质的起始两个参数。一个负责溶解，一个负责发光。
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"), 0.55f);
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Glow"), 200.f);
	}
	//启用溶解
	StartDissolve();

	// 死亡溶解时禁用角色移动等输入操作
	bDisableGameplay = true; 
	// 死亡时让其停止移动，不然会受重力影响
	GetCharacterMovement()->DisableMovement();
	if (Combat1)
	{
		Combat1->FireButtonPressed(false);//死亡时，取消开火，避免一直处于开火状态
	}
	// 溶解时使碰撞体无效，禁用碰撞
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);//禁用胶囊体碰撞
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);//禁用网格碰撞

	// 生成 淘汰机器人
	if (ElimBotEffect)
	{
		//淘汰机器人生成的位置。在角色正上方200处
		FVector ElimBotSpawnPoint(GetActorLocation().X, GetActorLocation().Y, GetActorLocation().Z + 200.f);
		//在某个位置产生特效，并将返回值存储
		ElimBotComponent = UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(),
			ElimBotEffect,
			ElimBotSpawnPoint,
			GetActorRotation()//使机器人与角色有相同的旋转
		);
	}
	if (ElimBotSound)
	{
		//在某个位置生成音效
		UGameplayStatics::SpawnSoundAtLocation(
			this,
			ElimBotSound,
			GetActorLocation()
		);
	}
	//玩家死亡时隐藏开镜效果
	bool bHideSniperScope = IsLocallyControlled() && 
		Combat1 &&
		Combat1->bAiming &&
		Combat1->EquippedWeapon &&
		Combat1->EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SniperRifle;
	if (bHideSniperScope)
	{
		ShowSniperScopeWidget(false);
	}
	if (CrownComponent)
	{
		//死亡时销毁皇冠
		CrownComponent->DestroyComponent();
	}
	//设置一个计时器，用于玩家淘汰后一段时间内重生
	GetWorldTimerManager().SetTimer(
		ElimTimer,
		this,
		&ABlasterCharacter::ElimTimerFinished,
		ElimDelay
	);
}
/// <summary>
/// 角色淘汰计时器完成后调用的函数。（这里用于复活角色
/// </summary>
void ABlasterCharacter::ElimTimerFinished()
{
	ABlasterGameMode* BlasterGameMode = GetWorld()->GetAuthGameMode<ABlasterGameMode>();
	if (BlasterGameMode && !bLeftGame)
	{
		//调用游戏模式的角色重生
		BlasterGameMode->RequestRespawn(this, Controller);
	}
	//倒计时结束后如果玩家处于退出游戏状态，则广播一下，加上IsLocallyControlled是确保只会在试图离开的客户端上进行广播
	if (bLeftGame && IsLocallyControlled())
	{
		OnLeftGame.Broadcast();
	}
}

/// <summary>
/// 角色离开（退出游戏）RPC
/// </summary>
void ABlasterCharacter::ServerLeaveGame_Implementation()
{
	ABlasterGameMode* BlasterGameMode = GetWorld()->GetAuthGameMode<ABlasterGameMode>();
	BlasterPlayerState = BlasterPlayerState == nullptr ? GetPlayerState<ABlasterPlayerState>() : BlasterPlayerState;
	if (BlasterGameMode && BlasterPlayerState)
	{
		BlasterGameMode->PlayerLeftGame(BlasterPlayerState);
	}
}

void ABlasterCharacter::DropOrDestroyWeapon(AWeapon* Weapon)
{
	if (Weapon == nullptr) return;
	///如果武器是初始默认生成的，当角色淘汰或者死亡时应该被销毁而不是掉落（以防地图上武器太多
	if (Weapon->bDestroyWeapon)
	{
		Weapon->Destroy();
	}
	else
	{
		Weapon->Dropped();
	}
}

void ABlasterCharacter::DropOrDestroyWeapons()
{
	if (Combat1)
	{
		if (Combat1->EquippedWeapon)
		{
			DropOrDestroyWeapon(Combat1->EquippedWeapon);
		}
		if (Combat1->SecondaryWeapon)
		{
			DropOrDestroyWeapon(Combat1->SecondaryWeapon);
		}
	}
}

/// <summary>
/// 角色生命周期函数，在角色销毁时调用,在服务器上销毁一个复制的actor的行为会传播到所有客户端。用以销毁淘汰机器人粒子组件
/// </summary>
void ABlasterCharacter::Destroyed()
{
	Super::Destroyed();

	if (ElimBotComponent)
	{
		ElimBotComponent->DestroyComponent();
	}
	//获取游戏状态（最高得分等信息）
	ABlasterGameMode* BlasterGameMode = Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(this));
	//获取游戏匹配状态（如果不是正在游戏则销毁武器
	bool bMatchNotInProgress = BlasterGameMode && BlasterGameMode->GetMatchState() != MatchState::InProgress;
	if (Combat1 && Combat1->EquippedWeapon && bMatchNotInProgress)
	{
		Combat1->EquippedWeapon->Destroy();//销毁武器
	}
}

void ABlasterCharacter::MulticastGainedTheLead_Implementation()
{
	if (CrownSystem == nullptr) return;
	if (CrownComponent == nullptr)
	{
		//生成特效
		CrownComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
			CrownSystem,
			GetCapsuleComponent(),//附加到角色胶囊上
			FName(),
			GetActorLocation() + FVector(0.f, 0.f, 110.f),//偏移位置加在头顶
			GetActorRotation(),
			EAttachLocation::KeepWorldPosition,
			false
		);
	}
	if (CrownComponent)
	{
		//启用特效
		CrownComponent->Activate();
	}
}

void ABlasterCharacter::MulticastLostTheLead_Implementation()
{
	if (CrownComponent)
	{
		//销毁特效
		CrownComponent->DestroyComponent();
	}
}


// Called when the game starts or when spawned
void ABlasterCharacter::BeginPlay()
{
	Super::BeginPlay();

	SpawDefaultWeapon();
	// 生成默认武器后也应该更新一下子弹数量
	UpdateHUDAmmo();
	UpdateHUDHealth();
	UpdateHUDShield();
	//只在服务器上进行伤害计算，因此只在服务器上注册函数回调
	if (HasAuthority())
	{
		//当演员以任何方式损坏时调用回调函数
		OnTakeAnyDamage.AddDynamic(this, &ABlasterCharacter::ReceiveDamage);
	}
	if (AttachedGrenade)
	{
		//手上手榴弹可见性（隐藏）
		AttachedGrenade->SetVisibility(false);
	}
}
// Called every frame
void ABlasterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	RotateInPlace(DeltaTime);//玩家就地旋转
	HideCameraIfCharacterClose();
	////目前先使用该方法来做显隐。一旦服务器上设置了重叠武器，基于复制变量的效果，
	////所有的客户端上也会进行重叠武器的复制，使得所有客户端上的武器均不为空，因此能够显示文字提示
	//if (OverlappingWeapon) {
	//	//设置武器的提示文字的显隐
	//	OverlappingWeapon->ShowPickupWidget(true);
	//}
	
	//每帧去看玩家状态是否有效（也就是角色是否重生完成）
	PollInit();
}

void ABlasterCharacter::RotateInPlace(float DeltaTime)
{
	if (bDisableGameplay)//游戏冷却状态时，禁止对角色本身进行原地旋转
	{
		bUseControllerRotationYaw = false;
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		return;
	}
	//如果控制权大于模拟代理（由于是枚举比较的就是数字大小，实际上指的就是如果role为ROLE_AutonomousProxy、ROLE_Authority、ROLE_Max）并且是本地控制的则直接进行瞄准偏移
	if (GetLocalRole() > ENetRole::ROLE_SimulatedProxy && IsLocallyControlled())
	{
		AimOffset(DeltaTime);
	}
	else//否则就对上次运动复制进行计时，计时超过时间则直接调用运动复制通知
	{
		TimeSinceLastMovementReplication += DeltaTime;
		if (TimeSinceLastMovementReplication > 0.25f)//时间如果超过饿0.25说明还没有复制运动，此时直接调用复制运动
		{
			OnRep_ReplicatedMovement();
		}
		//计算每一帧的pitch
		CalculateAO_Pitch();
	}
}

// Called to bind functionality to input
void ABlasterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	//绑定自带的跳跃函数
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ABlasterCharacter::Jump);

	//对应的输入名称，对象，函数地址
	PlayerInputComponent->BindAxis("MoveForward", this, &ABlasterCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ABlasterCharacter::MoveRight);
	PlayerInputComponent->BindAxis("Turn", this, &ABlasterCharacter::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &ABlasterCharacter::LookUp);
	//绑定输入和函数
	PlayerInputComponent->BindAction("Equip", IE_Pressed, this, &ABlasterCharacter::EquipButtonPressed);
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ABlasterCharacter::CrouchButtonPressed);
	PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &ABlasterCharacter::AimButtonPressed);
	PlayerInputComponent->BindAction("Aim", IE_Released, this, &ABlasterCharacter::AimButtonReleased);
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ABlasterCharacter::FireButtonPressed);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &ABlasterCharacter::FireButtonReleased);
	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &ABlasterCharacter::ReloadButtonPressed);
	PlayerInputComponent->BindAction("ThrowGrenade", IE_Pressed, this, &ABlasterCharacter::GrenadeButtonPressed);
}
/// <summary>
/// 在创建对象并且其所有组件都已注册和初始化时调用
/// </summary>
void ABlasterCharacter::PostInitializeComponents() {
	Super::PostInitializeComponents();
	if (Combat1)
	{
		Combat1->Character = this;
	}
	if (Buff)
	{
		Buff->Character = this;
		//设置正常的移速buff的相关参数（方便buff结束后重置为正常的移速
		Buff->SetInitialSpeeds(
			GetCharacterMovement()->MaxWalkSpeed,
			GetCharacterMovement()->MaxWalkSpeedCrouched
		);
		//设置正常的跳跃速度给到buff组件，以便buff持续时间到了只会恢复到正常的跳跃速度
		Buff->SetInitialJumpVelocity(GetCharacterMovement()->JumpZVelocity);
	}
	if (LagCompensation)
	{
		LagCompensation->Character = this;
		if (Controller)
		{
			LagCompensation->Controller = Cast<ABlasterPlayerController>(Controller);
		}
	}
}

/// <summary>
/// 播放开火蒙太奇动画
/// </summary>
void ABlasterCharacter::PlayFireMontage(bool bAiming)
{
	if (Combat1 == nullptr || Combat1->EquippedWeapon == nullptr) return;
	///获取动画实例
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && FireWeaponMontage) {
		//播放蒙太奇动画
		AnimInstance->Montage_Play(FireWeaponMontage);
		//用来选择播放蒙太奇动画里面瞄准的开火还是不瞄准的开火动画
		FName SectionName;
		//选择蒙太奇中对应的动画标签
		SectionName = bAiming ? FName("RifleAim") : FName("RifleHip");
		//转换到对应蒙太奇动画的蒙太奇片段上
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void ABlasterCharacter::PlayReloadMontage()
{
	if (Combat1 == nullptr || Combat1->EquippedWeapon == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ReloadMontage)
	{
		AnimInstance->Montage_Play(ReloadMontage);
		FName SectionName;
		//根据不同的武器类型播放不同的换弹蒙太奇
		switch (Combat1->EquippedWeapon->GetWeaponType())
		{
		case EWeaponType::EWT_AssaultRifle:
			SectionName = FName("Rifle");
			break;
		case EWeaponType::EWT_RocketLauncher:
			SectionName = FName("RocketLauncher");
			break;
		case EWeaponType::EWT_Pistol:
			SectionName = FName("Pistol");
			break;
		case EWeaponType::EWT_SubmachineGun:
			SectionName = FName("Pistol");
			break;
		case EWeaponType::EWT_Shotgun:
			SectionName = FName("Shotgun");
			break;
		case EWeaponType::EWT_SniperRifle:
			SectionName = FName("SniperRifle");
			break;
		case EWeaponType::EWT_GrenadeLauncher:
			SectionName = FName("GrenadeLauncher");
			break;
		}
		//跳转到蒙太奇指定点播放动画
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}


/// <summary>
/// 播放角色淘汰（死亡）动画
/// </summary>
void ABlasterCharacter::PlayElimMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ElimMontage)
	{
		//只有一个部分，因此不需要跳转到某个蒙太奇片段上
		AnimInstance->Montage_Play(ElimMontage);
	}
}

/// <summary>
/// 播放角色投掷手榴弹动画
/// </summary>
void ABlasterCharacter::PlayThrowGrenadeMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ThrowGrenadeMontage)
	{
		AnimInstance->Montage_Play(ThrowGrenadeMontage);
	}
}

/// <summary>
/// 播放切换武器动画
/// </summary>
void ABlasterCharacter::PlaySwapMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && SwapMontage)
	{
		AnimInstance->Montage_Play(SwapMontage);
	}
}

/// <summary>
/// 用于播放角色受击动画.
/// </summary>
void ABlasterCharacter::PlayHitReactMontage()
{
	//由于受击动画资源是拿着装备的，因此需要判断是否装备了装备
	if (Combat1 == nullptr || Combat1->EquippedWeapon == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HitReactMontage)
	{
		AnimInstance->Montage_Play(HitReactMontage);
		//选择蒙太奇中对应的动画标签
		FName SectionName("FromFront");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

/// <summary>
/// 按下投掷按钮牛触发 投掷手榴弹
/// </summary>
void ABlasterCharacter::GrenadeButtonPressed()
{
	if (Combat1)
	{
		Combat1->ThrowGrenade();
	}
}

/// <summary>
/// 用于接收来自子弹类（如ProjectileBullet）中的ApplyDamage委托的回调函数。
/// </summary>
/// <param name="DamagedActor"></param>
/// <param name="Damage"></param>
/// <param name="DamageType"></param>
/// <param name="InstigatorController">煽动者（施加伤害的控制器）</param>
/// <param name="DamageCauser"></param>
void ABlasterCharacter::ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatorController, AActor* DamageCauser)
{
	//如果玩家死亡了，则不继续作用伤害了
	if (bElimmed) return;

	float DamageToHealth = Damage;
	//如果有护盾先扣护盾
	if (Shield > 0.f)
	{
		if (Shield >= Damage)
		{
			Shield = FMath::Clamp(Shield - Damage, 0.f, MaxShield);
			DamageToHealth = 0.f;
		}
		else
		{
			Shield = 0.f;
			DamageToHealth = FMath::Clamp(DamageToHealth - Shield, 0.f, Damage);
		}
	}
	//受伤修改血量。由于Health是复制变量，修改后会同步到各个客户端
	Health = FMath::Clamp(Health - DamageToHealth, 0.f, MaxHealth);

	//更新HUD上的血量信息（仅在服务器上更新，客户端的更新则通过OnRep_Health进行更新）
	UpdateHUDHealth();
	//更新HUD上的护盾信息（仅在服务器上更新，客户端的更新则通过OnRep_Shield进行更新）
	UpdateHUDShield();
	//播放角色受击动画（仅在服务器上播放，客户端的播放则通过OnRep_Health播放）
	PlayHitReactMontage();

	//血量为0，则调用游戏模式中的淘汰函数
	if (Health == 0.f)
	{
		//获取游戏模式
		ABlasterGameMode* BlasterGameMode = GetWorld()->GetAuthGameMode<ABlasterGameMode>();
		if (BlasterGameMode)
		{
			//获取当前角色（受害者）控制
			BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController;
			//获取煽动者的控制器
			ABlasterPlayerController* AttackerController = Cast<ABlasterPlayerController>(InstigatorController);
			BlasterGameMode->PlayerEliminated(this, BlasterPlayerController, AttackerController);
		}
	}

}


void ABlasterCharacter::MoveForward(float Value)
{
	if (bDisableGameplay) return; //游戏冷却状态时，禁止移动
	if (Controller != nullptr && Value != 0.f) {
		//找到控制器的旋转输入，拿到绕z轴旋转的方向也就是控制器的朝向方向
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		//通过FRotationMatrix(YawRotation)创建旋转矩阵。
		//实际上就是将人物目前的pitch和roll角归零，以保证向量平行于地面，与控制器方向一直
		//以便获取得到人物此时的前向量
		//https://blog.csdn.net/weixin_51940803/article/details/130467309
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X));
		//增加输入
		//当我们摁下A或D的时候，Value的值就不是0，就会按照摄像机的方向的左右进行移动
		//当我们摁下W或S的时候也是。
		AddMovementInput(Direction, Value);
		//这边输入后在BlasterAnimInstance中通过GetVelocity就能够获取到人物的移动速度
	}
}

void ABlasterCharacter::MoveRight(float Value)
{
	if (bDisableGameplay) return; //游戏冷却状态时，禁止
	if (Controller != nullptr && Value != 0.f) {
		//找到控制器的旋转输入，拿到绕z轴旋转的方向也就是控制器的朝向方向
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		//通过FRotationMatrix(YawRotation)创建旋转矩阵。
		//实际上就是将人物目前的pitch和roll角归零，以保证向量平行于地面，与控制器方向一直
		//以便获取得到人物此时的前向量
		//https://blog.csdn.net/weixin_51940803/article/details/130467309
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y));
		//增加输入
		//当我们摁下A或D的时候，Value的值就不是0，就会按照摄像机的方向的左右进行移动
		//当我们摁下W或S的时候也是。
		AddMovementInput(Direction, Value);
		//这边输入后在BlasterAnimInstance中通过GetVelocity就能够获取到人物的移动速度
	}
}

void ABlasterCharacter::Turn(float Value)
{
	AddControllerYawInput(Value);
}

void ABlasterCharacter::LookUp(float Value)
{
	AddControllerPitchInput(Value);
}
/// <summary>
/// 装备武器和捡起武器和切换武器（需要服务器来负责处理
/// </summary>
void ABlasterCharacter::EquipButtonPressed()
{
	if (bDisableGameplay) return; //游戏冷却状态时，禁止武器操作
	if (Combat1) {
		//if (HasAuthority()) {
		//	Combat1->EquipWeapon(OverlappingWeapon);
		//}
		//else {
		//	//没有服务器权限，说明是由客户端调用的武器装备，使用RPC来调用服务器执行
		//	ServerEquipButtonPressed();

		//}

		if (Combat1->CombatState == ECombatState::ECS_Unoccupied) ServerEquipButtonPressed();
		//是否能够播放切换武器蒙太奇动画
		bool bSwap = Combat1->ShouldSwapWeapons() &&
			!HasAuthority() &&
			Combat1->CombatState == ECombatState::ECS_Unoccupied &&
			OverlappingWeapon == nullptr;

		if (bSwap)
		{
			PlaySwapMontage();
			Combat1->CombatState = ECombatState::ECS_SwappingWeapons;
			bFinishedSwapping = false;
		}
	}
}
/// <summary>
/// 由于在装备武器时，使用if (HasAuthority())进行判断有服务器权限才能装备，基于此方法则只能在服务器上装备
/// 因此使用RPC来解决这样的问题
/// RPC 用于客户端调用服务器执行的函数。函数命名会比.h中的函数名多_Implementation
/// ------------（20250810更新下方说明）---------------
/// RPC的作用是，如果是当前是客户端，则会调用服务器执行该函数，如果是服务器则也会执行该函数
/// ，总之无论如何都只会在服务器执行该函数
/// </summary>
void ABlasterCharacter::ServerEquipButtonPressed_Implementation()
{
	if (Combat1) {//去除 HasAuthority() 因为RPC只会在服务器上执行
		//如果站在了地上的武器上，则捡起武器
		if (OverlappingWeapon)
		{
			Combat1->EquipWeapon(OverlappingWeapon);
		}//如果没有则切换主副武器
		else if (Combat1->ShouldSwapWeapons())
		{
			Combat1->SwapWeapons();
		}
	}
}
/// <summary>
/// 蹲起
/// </summary>
void ABlasterCharacter::CrouchButtonPressed()
{
	if (bDisableGameplay) return; //游戏冷却状态时，禁止蹲下
	//Character自带有蹲下和蹲起的函数，蹲下成功会将变量bWantsToCrouch设置为true,并且自动调整胶囊体的大小，bWantsToCrouch是表示将要开始蹲下。
	//此外，Character还存在有bIsCrouched变量，且为复制变量，且能通知OnRep_IsCrouched，因此每当服务器上设置bIsCrouched，该值就恢复知道客户端
	//因此可以在动画蓝图中使用bIsCrouched
	if (bIsCrouched) {//这里为bIsCrouched是代表当前状态为蹲下，但是又按了蹲下建 因此要把状态改为蹲起
		UnCrouch();//蹲起
	}
	else {
		Crouch();//蹲下
	}
	
}

void ABlasterCharacter::ReloadButtonPressed()
{
	if (bDisableGameplay) return; //游戏冷却状态时，禁止装填弹药
	if (Combat1)
	{
		Combat1->Reload();
	}
}

/// <summary>
/// 瞄准
/// </summary>
void ABlasterCharacter::AimButtonPressed()
{
	if (bDisableGameplay) return; //游戏冷却状态时，禁止瞄准
	if (Combat1) {
		Combat1->SetAiming(true);
	}
}
/// <summary>
/// 取消瞄准
/// </summary>
void ABlasterCharacter::AimButtonReleased()
{
	if (bDisableGameplay) return; //游戏冷却状态时，禁止取消瞄准
	if (Combat1) {
		Combat1->SetAiming(false);
	}
}
/// <summary>
/// 计算角色速度
/// </summary>
/// <returns></returns>
float ABlasterCharacter::CalculateSpeed()
{
	FVector Velocity = GetVelocity();
	//不关心z值
	Velocity.Z = 0.f;
	return Velocity.Size();
}

/// <summary>
/// 动画偏移（动画叠加），这里用于获取角色枪口方向的值AO_Yaw和AO_Pitch来复制到BlasterAnimInstance中。 DeltaTime用于插值过渡动画
/// 视频规定：只会在角色静止（没有在跑或者跳跃）且装备武器时影响角色动画。（但个人不合理，后续有时间进行修改）
/// </summary>
/// <param name="DeltaTime"></param>
void ABlasterCharacter::AimOffset(float DeltaTime)
{
	if (Combat1 && Combat1->EquippedWeapon == nullptr) return;
	//获取速度
	float Speed = CalculateSpeed();
	bool bIsInAir = GetCharacterMovement()->IsFalling();

	if (Speed == 0.f && !bIsInAir) {//速度为0，并且没有在跳跃
		//静止不懂和跳跃是需要旋转根骨骼的
		bRotateRootBone = true;
		///！！！！！经过多次确认发现GetBaseAimRotation().Yaw无法进行服务器到客户端的同步，但是GetBaseAimRotation().Pitch是同步的，估计视频后续会有解决方案？
		FRotator CurrentAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);

		//与静止前的旋转角的差
		FRotator DeltaAimRotation = UKismetMathLibrary::NormalizedDeltaRotator(CurrentAimRotation, StartingAimRotation);
		AO_Yaw = DeltaAimRotation.Yaw;
		if (TurningInPlace == ETurningInPlace::ETIP_NotTurning) {
			InterpAO_Yaw = AO_Yaw; //人物静止时 记录角色转向角度
		}

		bUseControllerRotationYaw = true;
		TurnInPlace(DeltaTime);
	}
	if (Speed > 0.f || bIsInAir) { //在保持角色静止前
		//在运动时则不需要旋转根骨骼
		bRotateRootBone = false;
		StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f); //静止前的旋转情况
		AO_Yaw = 0.f; //一旦移动或者跳跃就设置为0
		bUseControllerRotationYaw = true;
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	}

	CalculateAO_Pitch();


}
/// <summary>
/// 计算角色上下朝向角度
/// </summary>
void ABlasterCharacter::CalculateAO_Pitch()
{
	//当前的任何状态不影响上下朝向
	AO_Pitch = GetBaseAimRotation().Pitch;
	//虚幻通过网络发送Yaw和Pitch时进行了压缩，转变为了无符号整型类型。因此，我们需要对超过90的情况进行处理，不然两端会产生不一致的动作
	if (AO_Pitch > 90.f && !IsLocallyControlled()) {
		//映射[270,360) 到 [-90,0) 的 pitch
		FVector2D InRange(270.f, 360.f);
		FVector2D OutRange(-90.f, 0.f);
		//使用自带的映射函数
		AO_Pitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, AO_Pitch);
	}
	//if (!HasAuthority() && IsLocallyControlled()) { //该条件表示客户端在客户端计算机上本地控制的角色。也就是客户端上的角色
	//	UE_LOG(LogTemp,Warning, TEXT("AO_Pitch: %f"), AO_Pitch);
	//}
	//if (HasAuthority() && !IsLocallyControlled()) { //该条件表示服务器看到的从客户端计算机上控制的角色。也就是服务器上模拟的客户端的角色
	//	UE_LOG(LogTemp, Warning, TEXT("AO_Pitch: %f"), AO_Pitch);
	//}
}
/// <summary>
/// 用于处理模拟代理（服务器上）的转向。用来解决角色转向同步时，由于（服务器上的）模拟代理角色旋转根骨骼并同步到其他客户端角色上导致转向抖动的问题
/// 因为动画蓝图同步并不一定是每帧都在执行的。解决方式就是对于（服务器上的）模拟代理角色不采用旋转根骨骼，并且重写OnRep_ReplicatedMovement函数，在Tick计时，如果超过一定时间，则自行进行运动同步通知函数调用
/// </summary>
void ABlasterCharacter::SimProxiesTurn()
{
	if (Combat1 == nullptr || Combat1->EquippedWeapon == nullptr) return;
	//如果是模拟代理，则不旋转根骨骼
	bRotateRootBone = false;
	float Speed = CalculateSpeed();
	//速度大于0则设置为不转向状态
	if (Speed > 0.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		return;
	}

	ProxyRotationLastFrame = ProxyRotation;
	ProxyRotation = GetActorRotation();
	//计算代理转向的偏移量
	ProxyYaw = UKismetMathLibrary::NormalizedDeltaRotator(ProxyRotation, ProxyRotationLastFrame).Yaw;

	//UE_LOG(LogTemp, Warning, TEXT("ProxyYaw: %f"), ProxyYaw);

	//根据增量与阈值比较，更新角色当前转向状态
	if (FMath::Abs(ProxyYaw) > TurnThreshold)
	{
		if (ProxyYaw > TurnThreshold)
		{
			TurningInPlace = ETurningInPlace::ETIP_Right;
		}
		else if (ProxyYaw < -TurnThreshold)
		{
			TurningInPlace = ETurningInPlace::ETIP_Left;
		}
		else
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		}
		return;
	}
	TurningInPlace = ETurningInPlace::ETIP_NotTurning;

}


void ABlasterCharacter::Jump()
{
	if (bDisableGameplay) return; //游戏冷却状态时，禁止跳跃
	if (bIsCrouched) { //如果在蹲下的时候跳跃
		UnCrouch();
	}
	else {
		Super::Jump();
	}
}
void ABlasterCharacter::FireButtonPressed() 
{
	if (bDisableGameplay) return; //游戏冷却状态时，禁止开火
	if (Combat1) {
		Combat1->FireButtonPressed(true);
	}
}
void ABlasterCharacter::FireButtonReleased()
{
	if (bDisableGameplay) return; //游戏冷却状态时，禁止开火
	if (Combat1) {
		Combat1->FireButtonPressed(false);
	}
}
void ABlasterCharacter::TurnInPlace(float DeltaTime)
{
	UE_LOG(LogTemp, Warning, TEXT("AO_Yaw : %f"), AO_Yaw);
	if (AO_Yaw > 90.f) {
		TurningInPlace = ETurningInPlace::ETIP_Right;
	}
	else if (AO_Yaw < -90.f) {
		TurningInPlace = ETurningInPlace::ETIP_Left;
	}
	if (TurningInPlace != ETurningInPlace::ETIP_NotTurning) { //如果正在左转或者右转
		InterpAO_Yaw = FMath::FInterpTo(InterpAO_Yaw, 0.f, DeltaTime, 4.f);
		AO_Yaw = InterpAO_Yaw;
		//如果转动角度不超过15度，则不属于转动状态，即不转动整个角色，而是只使用动画蓝图，播放上半身动画，也就是把转动状态设置为未转动
		if (FMath::Abs(AO_Yaw) < 15.f) 
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
			StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		}
	}
}

/* 现在通过受伤的委托来进行角色的受击动画播放了，因此不再使用该方法进行受击播放了
///// <summary>
///// 用于同步角色受击动画的 多播RPC。客户端调用，服务器执行的函数。如果在服务器上执行多播RPC，那么将在服务器以及所有客户端上调用。在定义时需在函数名后补充_Implementation
///// </summary>
//void ABlasterCharacter::MulticastHit_Implementation()
//{
//	PlayHitReactMontage();
//}
*/

/// <summary>
/// 用于当角色靠墙时，角色模型就会挡住视野，靠墙时隐藏角色
/// </summary>
void ABlasterCharacter::HideCameraIfCharacterClose()
{
	//只对本地角色有效
	if (!IsLocallyControlled()) return;
	//计算相机到角色的距离，如果小于某个阈值，则进行角色隐藏
	if ((FollowCamera->GetComponentLocation() - GetActorLocation()).Size() < CameraThreshold)
	{
		//隐藏角色网格
		GetMesh()->SetVisibility(false);
		//隐藏武器网格
		if (Combat1 && Combat1->EquippedWeapon && Combat1->EquippedWeapon->GetWeaponMesh())
		{
			Combat1->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = true;
		}
	}
	else
	{
		//还原
		GetMesh()->SetVisibility(true);
		if (Combat1 && Combat1->EquippedWeapon && Combat1->EquippedWeapon->GetWeaponMesh())
		{
			Combat1->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = false;
		}
	}
}

/// <summary>
/// （在服务器上）血量变化时，通知各客户端调用的回调函数。
/// </summary>
void ABlasterCharacter::OnRep_Health(float LastHealth)
{
	UpdateHUDHealth();

	/// 血量减少才播放受击动画
	if (Health < LastHealth)
	{
		PlayHitReactMontage();
	}
}

/// <summary>
/// （在服务器上）护盾变化时，通知各客户端调用的回调函数。
/// </summary>
void ABlasterCharacter::OnRep_Shield(float LastShield)
{
	UpdateHUDShield();
	if (Shield < LastShield)
	{
		/// 护盾减少播放受击动画
		PlayHitReactMontage();
	}
}


/// <summary>
/// 更新角色HUD的血量信息
/// </summary>
void ABlasterCharacter::UpdateHUDHealth()
{
	//获取角色控制器
	BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController;
	if (BlasterPlayerController)
	{
		//更新角色HUD的血量信息
		BlasterPlayerController->SetHUDHealth(Health, MaxHealth);
	}
}

/// <summary>
/// 更新角色HUD的护盾信息
/// </summary>
void ABlasterCharacter::UpdateHUDShield()
{
	BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController;
	if (BlasterPlayerController)
	{
		BlasterPlayerController->SetHUDShield(Shield, MaxShield);
	}
}

/// <summary>
/// 生成武器后也应该更新子弹数量
/// </summary>
void ABlasterCharacter::UpdateHUDAmmo()
{
	BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController;
	if (BlasterPlayerController && Combat1 && Combat1->EquippedWeapon)
	{
		BlasterPlayerController->SetHUDCarriedAmmo(Combat1->CarriedAmmo);
		BlasterPlayerController->SetHUDWeaponAmmo(Combat1->EquippedWeapon->GetAmmo());
	}
}

/// <summary>
/// 生成默认初始武器
/// </summary>
void ABlasterCharacter::SpawDefaultWeapon()
{
	ABlasterGameMode* BlasterGameMode = Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(this));
	UWorld* World = GetWorld();
	//只在BlasterGameMode游戏模式下生成初始武器，在开始大厅不会
	if (BlasterGameMode && World && !bElimmed && DefaultWeaponClass)
	{
		AWeapon* StartingWeapon = World->SpawnActor<AWeapon>(DefaultWeaponClass);
		StartingWeapon->bDestroyWeapon = true;
		if (Combat1)
		{
			Combat1->EquipWeapon(StartingWeapon);
		}
	}
}

/// <summary>
/// 轮询任何相关类并初始化我们的HUD。也就是当角色死亡重生后，需要将其玩家状态类的信息用以初始化HUD上的信息。
/// </summary>
void ABlasterCharacter::PollInit()
{
	if (BlasterPlayerState == nullptr)
	{
		BlasterPlayerState = GetPlayerState<ABlasterPlayerState>();
		if (BlasterPlayerState)
		{
			//传递的参数是增加的得分，因为是重生所以没得分，分数只能+0
			BlasterPlayerState->AddToScore(0.f);
			//死亡同理
			BlasterPlayerState->AddToDefeats(0);

			ABlasterGameState* BlasterGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this));

			//玩家死亡重生后看一下是否重新获得皇冠
			if (BlasterGameState && BlasterGameState->TopScoringPlayers.Contains(BlasterPlayerState))
			{
				MulticastGainedTheLead();
			}
		}
	}
}

/// <summary>
/// 材质溶解时间轴的回调函数（随着时间轴的播放，将在每一帧调用这个函数
/// </summary>
void ABlasterCharacter::UpdateDissolveMaterial(float DissolveValue)
{
	if (DynamicDissolveMaterialInstance)
	{
		//设置溶解参数
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"), DissolveValue);
	}
}
/// <summary>
/// 启动材质溶解时间轴
/// </summary>
void ABlasterCharacter::StartDissolve()
{
	//绑定溶解完成的回调
	DissolveTrack.BindDynamic(this, &ABlasterCharacter::UpdateDissolveMaterial);
	if (DissolveCurve && DissolveTimeline)
	{
		//将曲线加入到时间轴中。 参数：曲线、轨道
		DissolveTimeline->AddInterpFloat(DissolveCurve, DissolveTrack);
		//启用时间线
		DissolveTimeline->Play();
	}
}

/// <summary>
/// 客户端和服务器都会调用
/// 用于在武器类中设置复制变量OverlappingWeapon
/// </summary>
/// <param name="Weapon"></param>
void ABlasterCharacter::SetOverlappingWeapon(AWeapon* Weapon) {
	//如果OverlappingWeapon为空(其实就是代表上一次武器不为空，这次就为空了)，则隐藏提示
	if (OverlappingWeapon) {
		OverlappingWeapon->ShowPickupWidget(false);
	}
	//这里设置后会导致OnRep_OverleappingWeapon被调用！！！！，我输出看了一下好像是复制完才调用的OnRep_OverlappingWeapon，但是OnRep_OverlappingWeapon的参数又是复制前的OverlappingWeapon。
	OverlappingWeapon = Weapon;
	//UE_LOG(LogTemp, Warning, TEXT("OverlappingWeapon : %d"), OverlappingWeapon != nullptr);
	//判断是否是本地控制。主要是由于变量赋值的条件为COND_OwnerOnly，因此检测重叠只有在客户端上才会显示pickup，而服务器上无法显示，即OnRep_OverleappingWeapon没有被调用
	if (IsLocallyControlled()) {//由于SetOverlappingWeapon是在服务器上才会被调用，因此只有当控制者是服务器时IsLocallyControlled会返回true
		if (OverlappingWeapon) {
			//设置提示文字显隐
			OverlappingWeapon->ShowPickupWidget(true);
		}
	}
}

/// <summary>
/// （服务器通知客户端调用的函数）
/// 当绑定的重叠武器 复制前 调用的函数可以无参也可以有一个参数（该参数为复制变量）
/// </summary>
/// <param name="LastWeapon">为变量被复制之前的最后一个值</param>
void ABlasterCharacter::OnRep_OverlappingWeapon(AWeapon* LastWeapon)
{
	//UE_LOG(LogTemp, Warning, TEXT("OverlappingWeapon2 : %d"), OverlappingWeapon!=nullptr);
	if (OverlappingWeapon) {
		//设置提示文字显隐
		OverlappingWeapon->ShowPickupWidget(true);
	}
	//上一次的值不为空，说明这一次的值为空，为空则表示离开武器所在范围，隐藏提示.
	//为什么不直接判断OverlappingWeapon为不为空，因为如果OverlappingWeapon为空了无法调用ShowPickupWidget，所以得通过这个LastWeapon来调用）
	//同理SetOverlappingWeapon中的情况 仍需要对于服务器单独处理，那么把处理方式也放在SetOverlappingWeapon中
	if (LastWeapon) {
		LastWeapon->ShowPickupWidget(false);
	}
}
/// <summary>
/// 用于BlasterAnimInstance中访问判断是否装备武器，以播放相应的动画
/// </summary>
/// <returns></returns>
bool ABlasterCharacter::IsWeaponEquipped()
{
	//战斗组件以及战斗组件上装备的武器不为空，则装备了武器
	return (Combat1 && Combat1->EquippedWeapon);
}

bool ABlasterCharacter::IsAiming()
{
	//战斗组件以及战斗组件上的瞄准状态
	return (Combat1 && Combat1->bAiming);
}

AWeapon* ABlasterCharacter::GetEquippedWeapon()
{
	 if (Combat1 == nullptr) return nullptr;
	 return Combat1->EquippedWeapon;
}

FVector ABlasterCharacter::GetHitTarget() const
{
	if (Combat1 == nullptr) return FVector();
	return Combat1->HitTarget;
}
/// <summary>
/// 获取战斗状态
/// </summary>
/// <returns></returns>
ECombatState ABlasterCharacter::GetCombatState() const
{
	if (Combat1 == nullptr) return ECombatState::ECS_MAX;
	return Combat1->CombatState;
}

/// <summary>
/// 本地是否处于装弹状态
/// </summary>
/// <returns></returns>
bool ABlasterCharacter::IsLocallyReloading()
{
	if (Combat1 == nullptr) return false;
	return Combat1->bLocallyReloading;
}