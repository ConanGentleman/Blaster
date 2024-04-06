// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/WidgetComponent.h"
#include "Net/UnrealNetwork.h"
#include "Blaster/Weapon/Weapon.h"

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
/// 用于在武器类中设置复制变量OverlappingWeapon
/// </summary>
/// <param name="Weapon"></param>
void ABlasterCharacter::SetOverlappingWeapon(AWeapon* Weapon) {
	//如果OverlappingWeapon为空(其实就是代表上一次武器不为空，这次就为空了)，则隐藏提示
	if (OverlappingWeapon) {
		OverlappingWeapon->ShowPickupWidget(false);
	}
	OverlappingWeapon = Weapon;
	//判断是否是本地控制。主要是由于变量赋值的条件为COND_OwnerOnly，因此检测重叠只有在客户端上才会显示pickup，而服务器上无法显示，即OnRep_OverleappingWeapon没有被调用
	if (IsLocallyControlled()) {//由于SetOverlappingWeapon是在服务器上才会被调用，因此只有当控制者是服务器时IsLocallyControlled会返回true
		if (OverlappingWeapon) {
			//设置提示文字显隐
			OverlappingWeapon->ShowPickupWidget(true);
		}
	}
}
/// <summary>
/// 当绑定重叠武器复制前调用的函数可以无参也可以有一个参数（该参数为复制变量）
/// </summary>
/// <param name="LastWeapon">为变量被复制之前的最后一个值</param>
void ABlasterCharacter::OnRep_OverleappingWeapon(AWeapon* LastWeapon)
{
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




