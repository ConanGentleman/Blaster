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
	/// �ص��¼��ص�
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
	//buff�ڼ����Ծ�ٶ�
	UPROPERTY(EditAnywhere)
	float JumpZVelocityBuff = 4000.f;
	//buff����ʱ��
	UPROPERTY(EditAnywhere)
	float JumpBuffTime = 30.f;
};