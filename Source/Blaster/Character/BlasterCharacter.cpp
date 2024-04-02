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

	//����Ĭ�϶�������ΪCameraBoom
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	//�����ɱ����ӵ����������ͨ���Ǹ���������������ж��²�������ı佺����Ĵ�С������tan'huan
	CameraBoom->SetupAttachment(GetMesh());
	//��Ƶ��ɱ۳�
	CameraBoom->TargetArmLength = 600.f;
	//�Ƿ���Pawn������ת (������������������
	CameraBoom->bUsePawnControlRotation = true;

	//�������
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	//��������ӵ����ɱ��ϵ���������
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	//����Ҫʹ��Pawn������ת����Ϊ���ӵ���
	FollowCamera->bUsePawnControlRotation = false;

	//���ý�ɫ���ſ�����һ����ת(�������������׼��Ϊ���ӣ�
	bUseControllerRotationYaw = false;
	//�ý�ɫ�����Լ����˶�����
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
	//���Դ�����Ծ����
	PlayerInputComponent->BindAction("Jump",IE_Pressed,this, &ACharacter::Jump);

	//��Ӧ���������ƣ����󣬺�����ַ
	PlayerInputComponent->BindAxis("MoveForward", this, &ABlasterCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ABlasterCharacter::MoveRight);
	PlayerInputComponent->BindAxis("Turn", this, &ABlasterCharacter::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &ABlasterCharacter::LookUp);

}

void ABlasterCharacter::MoveForward(float Value)
{
	if (Controller != nullptr && Value != 0.f) {
		//�ҵ�����������ת���룬�õ���z����ת�ķ���Ҳ���ǿ������ĳ�����
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		//ͨ��FRotationMatrix(YawRotation)������ת����
		//ʵ���Ͼ��ǽ�����Ŀǰ��pitch��roll�ǹ��㣬�Ա�֤����ƽ���ڵ��棬�����������һֱ
		//�Ա��ȡ�õ������ʱ��ǰ����
		//https://blog.csdn.net/weixin_51940803/article/details/130467309
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X));
		//��������
		//����������A��D��ʱ��Value��ֵ�Ͳ���0���ͻᰴ��������ķ�������ҽ����ƶ�
		//����������W��S��ʱ��Ҳ�ǡ�
		AddMovementInput(Direction, Value);
		//����������BlasterAnimInstance��ͨ��GetVelocity���ܹ���ȡ��������ƶ��ٶ�
	}
}

void ABlasterCharacter::MoveRight(float Value)
{
	if (Controller != nullptr && Value != 0.f) {
		//�ҵ�����������ת���룬�õ���z����ת�ķ���Ҳ���ǿ������ĳ�����
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		//ͨ��FRotationMatrix(YawRotation)������ת����
		//ʵ���Ͼ��ǽ�����Ŀǰ��pitch��roll�ǹ��㣬�Ա�֤����ƽ���ڵ��棬�����������һֱ
		//�Ա��ȡ�õ������ʱ��ǰ����
		//https://blog.csdn.net/weixin_51940803/article/details/130467309
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y));
		//��������
		//����������A��D��ʱ��Value��ֵ�Ͳ���0���ͻᰴ��������ķ�������ҽ����ƶ�
		//����������W��S��ʱ��Ҳ�ǡ�
		AddMovementInput(Direction, Value);
		//����������BlasterAnimInstance��ͨ��GetVelocity���ܹ���ȡ��������ƶ��ٶ�
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



