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
/// ��ӻظ�����buff
/// </summary>
/// <param name="HealAmount">��Ѫ����</param>
/// <param name="HealingTime">��Ѫʱ��</param>
void UBuffComponent::ReplenishShield(float ShieldAmount, float ReplenishTime)
{
	bReplenishingShield = true;
	ShieldReplenishRate = ShieldAmount / ReplenishTime;
	ShieldReplenishAmount += ShieldAmount;
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

/// <summary>
/// ���ܻظ�buff
/// </summary>
/// <param name="DeltaTime"></param>
void UBuffComponent::ShieldRampUp(float DeltaTime)
{
	if (!bReplenishingShield || Character == nullptr || Character->IsElimmed()) return;

	const float ReplenishThisFrame = ShieldReplenishRate * DeltaTime;
	Character->SetShield(FMath::Clamp(Character->GetShield() + ReplenishThisFrame, 0.f, Character->GetMaxShield()));
	Character->UpdateHUDShield();
	ShieldReplenishAmount -= ReplenishThisFrame;

	if (ShieldReplenishAmount <= 0.f || Character->GetShield() >= Character->GetMaxShield())
	{
		bReplenishingShield = false;
		ShieldReplenishAmount = 0.f;
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
/// ��ʼ������������Ծ�ٶȣ��߶ȣ�
/// </summary>
/// <param name="Velocity"></param>
void UBuffComponent::SetInitialJumpVelocity(float Velocity)
{
	InitialJumpVelocity = Velocity;
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
/// �ಥ�����ٶȣ��������ܱ�֤�ͻ���Ȩ�������buffЧ��
/// ���ڽ�ɫ�ٶ����õ� �ಥRPC���ͻ��˵��ã�������ִ�еĺ���������ڷ�������ִ�жಥRPC����ô���ڷ������Լ����пͻ����ϵ��á��ڶ���ʱ���ں������󲹳�_Implementation
/// reliable�ɿ��ģ�����ʧ�᳢ܻ���ط�
/// </summary>
void UBuffComponent::MulticastSpeedBuff_Implementation(float BaseSpeed, float CrouchSpeed)
{
	if (Character && Character->GetCharacterMovement())
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = BaseSpeed;
		Character->GetCharacterMovement()->MaxWalkSpeedCrouched = CrouchSpeed;
	}
}

/// <summary>
/// ��ʼ��Ծbuff
/// </summary>
/// <param name="BuffJumpVelocity"></param>
/// <param name="BuffTime"></param>
void UBuffComponent::BuffJump(float BuffJumpVelocity, float BuffTime)
{
	if (Character == nullptr) return;

	Character->GetWorldTimerManager().SetTimer(
		JumpBuffTimer,
		this,
		&UBuffComponent::ResetJump,
		BuffTime
	);

	if (Character->GetCharacterMovement())
	{
		Character->GetCharacterMovement()->JumpZVelocity = BuffJumpVelocity;
	}
	MulticastJumpBuff(BuffJumpVelocity);
}

/// <summary>
/// �ಥ������Ծ���������ܱ�֤�ͻ���Ȩ�������buffЧ��
/// ���ڽ�ɫ��Ծ�ٶȣ��߶ȣ����õ� �ಥRPC���ͻ��˵��ã�������ִ�еĺ���������ڷ�������ִ�жಥRPC����ô���ڷ������Լ����пͻ����ϵ��á��ڶ���ʱ���ں������󲹳�_Implementation
/// reliable�ɿ��ģ�����ʧ�᳢ܻ���ط�
/// </summary>
void UBuffComponent::MulticastJumpBuff_Implementation(float JumpVelocity)
{
	if (Character && Character->GetCharacterMovement())
	{
		Character->GetCharacterMovement()->JumpZVelocity = JumpVelocity;
	}
}

void UBuffComponent::ResetJump()
{
	if (Character->GetCharacterMovement())
	{
		Character->GetCharacterMovement()->JumpZVelocity = InitialJumpVelocity;
	}
	MulticastJumpBuff(InitialJumpVelocity);
}


void UBuffComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	HealRampUp(DeltaTime);
	ShieldRampUp(DeltaTime);
}