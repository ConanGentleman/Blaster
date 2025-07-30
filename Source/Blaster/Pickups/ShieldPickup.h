// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pickup.h"
#include "ShieldPickup.generated.h"

/**
 *
 */
UCLASS()
class BLASTER_API AShieldPickup : public APickup
{
	GENERATED_BODY()
protected:
	/// <summary>
	/// �ص������ص�
	/// </summary>
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
	/// ���ܻظ�����
	/// </summary>
	UPROPERTY(EditAnywhere)
	float ShieldReplenishAmount = 100.f;

	/// <summary>
	/// ���ܻظ�ʱ��
	/// </summary>
	UPROPERTY(EditAnywhere)
	float ShieldReplenishTime = 5.f;
};