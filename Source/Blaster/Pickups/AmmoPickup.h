// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pickup.h"
#include "Blaster/Weapon/WeaponTypes.h"
#include "AmmoPickup.generated.h"

/**
 *
 */
UCLASS()
class BLASTER_API AAmmoPickup : public APickup
{
	GENERATED_BODY()
protected:
	/// <summary>
	/// �����OnSphereOverlap �Ѿ�������UFUNCTION�����಻��Ҫ������ 
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
	/// ʰȡ���õ��ӵ�����
	/// </summary>
	UPROPERTY(EditAnywhere)
	int32 AmmoAmount = 30;

	/// <summary>
	/// ���������������ӵ�
	/// </summary>
	UPROPERTY(EditAnywhere)
	EWeaponType WeaponType;
};