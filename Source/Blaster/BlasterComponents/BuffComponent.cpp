// Fill out your copyright notice in the Description page of Project Settings.


#include "BuffComponent.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

UBuffComponent::UBuffComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

}

/// <summary>
/// ��ӻ�Ѫbuff
/// </summary>
/// <param name="HealAmount">��Ѫ����</param>
/// <param name="HealingTime">��Ѫʱ��</param>
void UBuffComponent::Heal(float HealAmount, float HealingTime)
{
	bHealing = true;
	HealingRate = HealAmount / HealingTime;
	AmountToHeal += HealAmount;
}

/// <summary>
/// ��Ѫbuff
/// </summary>
/// <param name="DeltaTime"></param>
void UBuffComponent::HealRampUp(float DeltaTime)
{
	if (!bHealing || Character == nullptr || Character->IsElimmed()) return;

	const float HealThisFrame = HealingRate * DeltaTime;
	Character->SetHealth(FMath::Clamp(Character->GetHealth() + HealThisFrame, 0.f, Character->GetMaxHealth()));
	///Ѫ���仯OnRep_Healthֻ���ڷ������ϲŻ�֪ͨ�ص�ִ��UpdateHUDHealth�����ͻ��˱���ҲӦ�������´���
	Character->UpdateHUDHealth();
	AmountToHeal -= HealThisFrame;

	if (AmountToHeal <= 0.f || Character->GetHealth() >= Character->GetMaxHealth())
	{
		bHealing = false;
		AmountToHeal = 0.f;
	}
}


void UBuffComponent::BeginPlay()
{
	Super::BeginPlay();

}

/// <summary>
/// ��ʼ�����������ٶ�
/// </summary>
/// <param name="BaseSpeed"></param>
/// <param name="CrouchSpeed"></param>
void UBuffComponent::SetInitialSpeeds(float BaseSpeed, float CrouchSpeed)
{
	InitialBaseSpeed = BaseSpeed;
	InitialCrouchSpeed = CrouchSpeed;
}

/// <summary>
/// ��ʼ����buff
/// </summary>
/// <param name="BuffBaseSpeed"></param>
/// <param name="BuffCrouchSpeed"></param>
/// <param name="BuffTime"></param>
void UBuffComponent::BuffSpeed(float BuffBaseSpeed, float BuffCrouchSpeed, float BuffTime)
{
	if (Character == nullptr) return;

	Character->GetWorldTimerManager().SetTimer(
		SpeedBuffTimer,
		this,
		&UBuffComponent::ResetSpeeds,
		BuffTime
	);

	if (Character->GetCharacterMovement())
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = BuffBaseSpeed;
		Character->GetCharacterMovement()->MaxWalkSpeedCrouched = BuffCrouchSpeed;
	}
	MulticastSpeedBuff(BuffBaseSpeed, BuffCrouchSpeed);
}

/// <summary>
/// buff����������Ϊ�����ٶ�
/// </summary>
void UBuffComponent::ResetSpeeds()
{
	if (Character == nullptr || Character->GetCharacterMovement() == nullptr) return;

	Character->GetCharacterMovement()->MaxWalkSpeed = InitialBaseSpeed;
	Character->GetCharacterMovement()->MaxWalkSpeedCrouched = InitialCrouchSpeed;
	MulticastSpeedBuff(InitialBaseSpeed, InitialCrouchSpeed);
}

/// <summary>
/// �ಥ�����ٶ�
/// ���ڽ�ɫ�ٶ����е� �ಥRPC���ͻ��˵��ã�������ִ�еĺ���������ڷ�������ִ�жಥRPC����ô���ڷ������Լ����пͻ����ϵ��á��ڶ���ʱ���ں������󲹳�_Implementation
/// reliable�ɿ��ģ�����ʧ�᳢ܻ���ط�
/// </summary>
void UBuffComponent::MulticastSpeedBuff_Implementation(float BaseSpeed, float CrouchSpeed)
{
	Character->GetCharacterMovement()->MaxWalkSpeed = BaseSpeed;
	Character->GetCharacterMovement()->MaxWalkSpeedCrouched = CrouchSpeed;
}


void UBuffComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	HealRampUp(DeltaTime);
}