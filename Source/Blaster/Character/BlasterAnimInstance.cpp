// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterAnimInstance.h"
#include "BlasterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

void UBlasterAnimInstance::NativeInitializeAnimation() {
	Super::NativeInitializeAnimation();
	//设置角色 ，将Pawn转换为BlasterCharacter
	BlasterCharacter = Cast<ABlasterCharacter>(TryGetPawnOwner());
}
void UBlasterAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	if (BlasterCharacter == nullptr) {
		BlasterCharacter = Cast<ABlasterCharacter>(TryGetPawnOwner());
	}
	if (BlasterCharacter == nullptr) return;
	//获取速度
	FVector Velocity = BlasterCharacter->GetVelocity();
	//不关心z值
	Velocity.Z = 0.f;
	Speed = Velocity.Size();
	//直接通过API获取
	bIsInAir = BlasterCharacter->GetCharacterMovement()->IsFalling();
	//加速度大于0则是加速
	bIsAccelerating = BlasterCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f ? true : false;
	//是否装备了武器
	bWeaponEquipped = BlasterCharacter->IsWeaponEquipped();
	//是否蹲下
	bIsCrouched = BlasterCharacter->bIsCrouched;
	bAiming = BlasterCharacter->IsAiming();
}
