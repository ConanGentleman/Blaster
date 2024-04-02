// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

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
}

// Called when the game starts or when spawned
void ABlasterCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}
// Called to bind functionality to input
void ABlasterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	//绑定自带的跳跃函数
	PlayerInputComponent->BindAction("Jump",IE_Pressed,this, &ACharacter::Jump);

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

// Called every frame
void ABlasterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}



