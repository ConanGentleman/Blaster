// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BuffComponent.generated.h"

/*
ClassGroup����������� �༭���еķ��ࡣ
Custom ��ʾ���ᱻ���ൽ "Custom" ���飨�����������ѡ�����У���
meta ��Ԫ���ݣ�metadata���������ṩ����ı༭��������ʱ��Ϊ���ơ�
BlueprintSpawnableComponent ��ʾ����� ��������ͼ�б�������Spawn����ͨ������ UActorComponent �����ࡣ
���buff�ࣨ��Ѫ��������Լ�����١����ܵ�
*/
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class BLASTER_API UBuffComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UBuffComponent();
	/// <summary>
	/// ����Ϊfriend class ��������ֱ�ӷ���ABlasterCharacter��˽�г�Ա
	/// </summary>
	friend class ABlasterCharacter;
	void Heal(float HealAmount, float HealingTime);
	/// <summary>
	/// ��ʼ����buff
	/// </summary>
	/// <param name="BuffBaseSpeed"></param>
	/// <param name="BuffCrouchSpeed"></param>
	/// <param name="BuffTime"></param>
	void BuffSpeed(float BuffBaseSpeed, float BuffCrouchSpeed, float BuffTime);
	/// <summary>
	/// ��ʼ�����������ٶ�
	/// </summary>
	/// <param name="BaseSpeed"></param>
	/// <param name="CrouchSpeed"></param>
	void SetInitialSpeeds(float BaseSpeed, float CrouchSpeed);
protected:
	virtual void BeginPlay() override;
	/// <summary>
	/// ��Ѫbuff
	/// </summary>
	/// <param name="DeltaTime"></param>
	void HealRampUp(float DeltaTime);
private:
	UPROPERTY()
	class ABlasterCharacter* Character;

	/**
	* Ѫ���ظ� buff
	*/

	bool bHealing = false;
	/// <summary>
	/// ÿ��ظ�����
	/// </summary>
	float HealingRate = 0;
	/// <summary>
	/// ��ʱ��
	/// </summary>
	float AmountToHeal = 0.f;

	/**
	* ���� buff
	*/

	/// <summary>
	/// ��ʱ��
	/// </summary>
	FTimerHandle SpeedBuffTimer;
	/// <summary>
	/// buff����������Ϊ�����ٶ�
	/// </summary>
	void ResetSpeeds();
	/// <summary>
	/// �����Ĳ����ٶ�
	/// </summary>
	float InitialBaseSpeed;
	/// <summary>
	/// �����Ķ��������ٶ�
	/// </summary>
	float InitialCrouchSpeed;

	/// <summary>
	/// �ಥ�����ٶ�
	/// ���ڽ�ɫ�ٶ����е� �ಥRPC���ͻ��˵��ã�������ִ�еĺ���������ڷ�������ִ�жಥRPC����ô���ڷ������Լ����пͻ����ϵ��á��ڶ���ʱ���ں������󲹳�_Implementation
	/// reliable�ɿ��ģ�����ʧ�᳢ܻ���ط�
	/// </summary>
	UFUNCTION(NetMulticast, Reliable)
	void MulticastSpeedBuff(float BaseSpeed, float CrouchSpeed);

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;


};