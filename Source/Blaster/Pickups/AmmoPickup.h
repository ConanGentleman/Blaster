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
	/// 父类的OnSphereOverlap 已经声明了UFUNCTION，子类不需要再声明 
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
	/// 拾取后获得的子弹数量
	/// </summary>
	UPROPERTY(EditAnywhere)
	int32 AmmoAmount = 30;

	/// <summary>
	/// 属于哪种武器的子弹
	/// </summary>
	UPROPERTY(EditAnywhere)
	EWeaponType WeaponType;
};