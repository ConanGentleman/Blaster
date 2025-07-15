// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Projectile.h"
#include "ProjectileGrenade.generated.h"

/**
 *
 */
UCLASS()
class BLASTER_API AProjectileGrenade : public AProjectile
{
	GENERATED_BODY()
public:
	/// <summary>
	/// 构造函数
	/// </summary>
	AProjectileGrenade();
	virtual void Destroyed() override;
protected:
	virtual void BeginPlay() override;

	/// <summary>
	/// 弹跳回调函数 （UFUNCTION()表示对蓝图可视化脚本图表公开，以便开发者从蓝图资源调用或扩展UFunction，而无需更改C++代码
	/// </summary>
	UFUNCTION()
	void OnBounce(const FHitResult& ImpactResult, const FVector& ImpactVelocity);
private:
	/// <summary>
	/// 弹跳声音
	/// </summary>
	UPROPERTY(EditAnywhere)
	USoundCue* BounceSound;
};