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

protected:
	/// <summary>
	/// 带分散点的检测追踪终点（霰弹枪的多个子弹终点计算，随机返回霰弹枪的某颗字段的终点）
	/// </summary>
	/// <param name="TraceStart"></param>
	/// <param name="HitTarget"></param>
	/// <returns></returns>
	FVector TraceEndWithScatter(const FVector& TraceStart, const FVector& HitTarget);
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


	/**
	*  带分散点的检测追踪终点（霰弹枪用）
	*/

	/// <summary>
	/// 霰弹枪生成点位的球心距离（用以调整散射的程度）
	/// 实际上代表的就是霰弹枪的射程
	/// </summary>
	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	float DistanceToSphere = 800.f;

	/// <summary>
	///球的半径（击中范围）
	/// </summary>
	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	float SphereRadius = 75.f;

	/// <summary>
	/// 是否使用散射（如霰弹枪
	/// </summary>
	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	bool bUseScatter = false;


};