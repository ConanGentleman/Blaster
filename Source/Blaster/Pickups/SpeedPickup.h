// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pickup.h"
#include "SpeedPickup.generated.h"

/**
 *
 */
UCLASS()
class BLASTER_API ASpeedPickup : public APickup
{
	GENERATED_BODY()
protected:
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
	/// buff�����ڼ�Ĳ����ٶ�
	/// </summary>
	UPROPERTY(EditAnywhere)
	float BaseSpeedBuff = 1600.f;

	/// <summary>
	/// buff�����ڼ�Ķ��������ٶ�
	/// </summary>
	UPROPERTY(EditAnywhere)
	float CrouchSpeedBuff = 850.f;

	/// <summary>
	/// buff����ʱ��
	/// </summary>
	UPROPERTY(EditAnywhere)
	float SpeedBuffTime = 30.f;
};