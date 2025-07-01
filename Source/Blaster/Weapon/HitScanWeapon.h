// Fill out your copyright notice in the Description page of Project Settings.Add commentMore actions

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "HitScanWeapon.generated.h"

/**
 * 直接造成伤害的武器（如手枪）
 */
UCLASS()
class BLASTER_API AHitScanWeapon : public AWeapon
{
	GENERATED_BODY()
public:
	virtual void Fire(const FVector& HitTarget) override;
private:

	UPROPERTY(EditAnywhere)
	float Damage = 20.f;

	/// <summary>
	/// 击中特效
	/// </summary>
	UPROPERTY(EditAnywhere)
	class UParticleSystem* ImpactParticles;

	/// <summary>
	/// 子弹路径特效（光束特效）
	/// </summary>
	UPROPERTY(EditAnywhere)
	UParticleSystem* BeamParticles;

	/// <summary>
	/// 枪口开火特效
	/// </summary>
	UPROPERTY(EditAnywhere)
	UParticleSystem* MuzzleFlash;

	/// <summary>
	/// 开火音效
	/// </summary>
	UPROPERTY(EditAnywhere)
	USoundCue* FireSound;

	/// <summary>
	/// 击中音效
	/// </summary>
	UPROPERTY(EditAnywhere)
	USoundCue* HitSound;
};