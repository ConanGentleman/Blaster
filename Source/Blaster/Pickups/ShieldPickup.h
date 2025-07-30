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
	/// 重叠触发回调
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
	/// 护盾回复总量
	/// </summary>
	UPROPERTY(EditAnywhere)
	float ShieldReplenishAmount = 100.f;

	/// <summary>
	/// 护盾回复时间
	/// </summary>
	UPROPERTY(EditAnywhere)
	float ShieldReplenishTime = 5.f;
};