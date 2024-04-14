// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/WidgetComponent.h"
#include "Net/UnrealNetwork.h"
#include "Blaster/Weapon/Weapon.h"
#include "Blaster/BlasterComponents/CombatComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
ABlasterCharacter::ABlasterCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

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

	Combat = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	///使其可复制，由于组件有些特殊，因此并不需要在getlifetimereplicatedprops中注册。直接设置即可
	Combat->SetIsReplicated(true);

	//将该变量设置为true才能正常蹲下，因为Crouch函数会依据这个
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
	///下面两行防止两个角色相交时，胶囊体碰撞导致相机视角改变
	//解决胶囊与相机碰撞的问题
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	//解决网格与相机的碰撞问题
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	//初始化转向为不转
	TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	//设置网络更新的频率（也就是每秒总共发送多少帧数据）
	NetUpdateFrequency = 66.f; //默认是100；
	//设置最小网络更新频率（用于决定在复制属性很少发生变化时的节流率
	MinNetUpdateFrequency = 33.f;//默认是2
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
}

// Called when the game starts or when spawned
void ABlasterCharacter::BeginPlay()
{
	Super::BeginPlay();
}
// Called every frame
void ABlasterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	AimOffset(DeltaTime);
	////目前先使用该方法来做显隐。一旦服务器上设置了重叠武器，基于复制变量的效果，
	////所有的客户端上也会进行重叠武器的复制，使得所有客户端上的武器均不为空，因此能够显示文字提示
	//if (OverlappingWeapon) {
	//	//设置武器的提示文字的显隐
	//	OverlappingWeapon->ShowPickupWidget(true);
	//}
}

// Called to bind functionality to input
void ABlasterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	//绑定自带的跳跃函数
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);

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
}
/// <summary>
/// 在创建对象并且其所有组件都已注册和初始化时调用
/// </summary>
void ABlasterCharacter::PostInitializeComponents() {
	Super::PostInitializeComponents();
	if (Combat)
	{
		Combat->Character = this;
	}
}
void ABlasterCharacter::MoveForward(float Value)
{
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
/// 装备武器和捡起武器（需要服务器来负责处理
/// </summary>
void ABlasterCharacter::EquipButtonPressed()
{
	if (Combat) {
		if (HasAuthority()) {
			Combat->EquipWeapon(OverlappingWeapon);
		}
		else {
			//没有服务器权限，说明是由客户端调用的武器装备，因此使用RPC来调用服务器执行
			ServerEquipButtonPressed();

		}
	}
}
/// <summary>
/// 由于在装备武器时，使用if (HasAuthority())进行判断有服务器权限才能装备，基于此方法则只能在服务器上装备
/// 因此使用RPC来解决这样的问题
/// RPC 用于客户端调用服务器执行的函数。函数命名会比.h中的函数名多_Implementation
/// </summary>
void ABlasterCharacter::ServerEquipButtonPressed_Implementation()
{
	if (Combat) {//去除 HasAuthority() 因为RPC只会在服务器上执行
		Combat->EquipWeapon(OverlappingWeapon);
	}
}
/// <summary>
/// 蹲起
/// </summary>
void ABlasterCharacter::CrouchButtonPressed()
{
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
/// <summary>
/// 瞄准
/// </summary>
void ABlasterCharacter::AimButtonPressed()
{
	if (Combat) {
		Combat->SetAiming(true);
	}
}
/// <summary>
/// 取消瞄准
/// </summary>
void ABlasterCharacter::AimButtonReleased()
{
	if (Combat) {
		Combat->SetAiming(false);
	}
}
/// <summary>
/// 动画偏移（动画叠加），这里用于获取角色枪口方向的值AO_Yaw和AO_Pitch来复制到BlasterAnimInstance中。 DeltaTime用于插值过渡动画
/// 视频规定：只会在角色静止（没有在跑或者跳跃）且装备武器时影响角色动画。（但个人不合理，后续有时间进行修改）
/// </summary>
/// <param name="DeltaTime"></param>
void ABlasterCharacter::AimOffset(float DeltaTime)
{
	if (Combat && Combat->EquippedWeapon == nullptr) return;
	//获取速度
	FVector Velocity = GetVelocity();
	//不关心z值
	Velocity.Z = 0.f;
	float Speed = Velocity.Size();
	bool bIsInAir = GetCharacterMovement()->IsFalling();

	if (Speed == 0.f && !bIsInAir) {//速度为0，并且没有在跳跃
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
		StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f); //静止前的旋转情况
		AO_Yaw = 0.f; //一旦移动或者跳跃就设置为0
		bUseControllerRotationYaw = true;
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	}
	
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
	return (Combat && Combat->EquippedWeapon);
}

bool ABlasterCharacter::IsAiming()
{
	//战斗组件以及战斗组件上的瞄准状态
	return (Combat && Combat->bAiming);
}

AWeapon* ABlasterCharacter::GetEquippedWeapon()
{
	 if (Combat == nullptr) return nullptr;
	 return Combat->EquippedWeapon;
}



