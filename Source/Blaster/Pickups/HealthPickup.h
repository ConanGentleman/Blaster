// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pickup.h"
#include "HealthPickup.generated.h"

/**
 * Ѫ��ʰȡ��
 */
UCLASS()
class BLASTER_API AHealthPickup : public APickup
{
	GENERATED_BODY()
public:
	AHealthPickup();
	virtual void Destroyed() override;
protected:
	/// <summary>
	/// �ص��¼��ص�
	/// </summary>
	/// <param name="OverlappedComponent"></param>
	/// <param name="OtherActor"></param>
	/// <param name="OtherComp"></param>
	/// <param name="OtherBodyIndex"></param>
	/// <param name="bFromSweep"></param>
	/// <param name="SweepResult"></param>
	virtual void OnSphereOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);
private:
	/// <summary>
	/// Ѫ��������Ѫ��
	/// </summary>
	UPROPERTY(EditAnywhere)
	float HealAmount = 100.f;

	/// <summary>
	/// Ѫ���ڶ���ڻָ���HealAmount
	/// </summary>
	UPROPERTY(EditAnywhere)
	float HealingTime = 5.f;

	/// <summary>
	/// ��ЧNiagara���
	/// </summary>
	UPROPERTY(VisibleAnywhere)
	class UNiagaraComponent* PickupEffectComponent;

	/// <summary>
	/// Ѫ����ʰȡʱ�Ķ�Ч
	/// </summary>
	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* PickupEffect;
};