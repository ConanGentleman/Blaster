// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pickup.h"
#include "JumpPickup.generated.h"

/**
 *
 */
UCLASS()
class BLASTER_API AJumpPickup : public APickup
{
	GENERATED_BODY()
protected:
	/// <summary>
	/// 重叠事件回调
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
	//buff期间的跳跃速度
	UPROPERTY(EditAnywhere)
	float JumpZVelocityBuff = 4000.f;
	//buff持续时间
	UPROPERTY(EditAnywhere)
	float JumpBuffTime = 30.f;
};