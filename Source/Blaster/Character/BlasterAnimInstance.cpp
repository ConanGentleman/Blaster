// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterAnimInstance.h"
#include "BlasterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

void UBlasterAnimInstance::NativeInitializeAnimation() {
	Super::NativeInitializeAnimation();
	//���ý�ɫ ����Pawnת��ΪBlasterCharacter
	BlasterCharacter = Cast<ABlasterCharacter>(TryGetPawnOwner());
}
void UBlasterAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	if (BlasterCharacter == nullptr) {
		BlasterCharacter = Cast<ABlasterCharacter>(TryGetPawnOwner());
	}
	if (BlasterCharacter == nullptr) return;
	//��ȡ�ٶ�
	FVector Velocity = BlasterCharacter->GetVelocity();
	//������zֵ
	Velocity.Z = 0.f;
	Speed = Velocity.Size();
	//ֱ��ͨ��API��ȡ
	bIsInAir = BlasterCharacter->GetCharacterMovement()->IsFalling();
	//���ٶȴ���0���Ǽ���
	bIsAccelerating = BlasterCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f ? true : false;
}
