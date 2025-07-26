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

	bool bHealing = false;
	/// <summary>
	/// ÿ��ظ�����
	/// </summary>
	float HealingRate = 0;
	float AmountToHeal = 0.f;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;


};