﻿// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Projectile.h"
#include "ProjectileRocket.generated.h"

/**
 * 火箭弹
 */
UCLASS()
class BLASTER_API AProjectileRocket : public AProjectile
{
	GENERATED_BODY()
public:
	AProjectileRocket();//构造函数
	virtual void Destroyed() override;//销毁时调用
protected:
	/// <summary>
	/// 子弹碰装函数
	/// </summary>
	/// <param name="HitComp"></param>
	/// <param name="OtherActor"></param>
	/// <param name="OtherComp"></param>
	/// <param name="NormalImpulse"></param>
	/// <param name="Hit"></param>
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) override;
	virtual void BeginPlay() override;

	/// <summary>
	/// 子弹飞行时的声音源
	/// </summary>
	UPROPERTY(EditAnywhere)
	USoundCue* ProjectileLoop;

	/// <summary>
	/// 子弹飞行时的声音组件
	/// </summary>
	UPROPERTY()
	UAudioComponent* ProjectileLoopComponent;

	/// <summary>
	/// 子弹飞行时的声音衰减
	/// </summary>
	UPROPERTY(EditAnywhere)
	USoundAttenuation* LoopingSoundAttenuation;

	/// <summary>
	/// 子弹运动组件(用来做抛物线或者子弹的组件)，不使用父类的ProjectileMovementComponent组件
	/// </summary>
	UPROPERTY(VisibleAnywhere)
	class URocketMovementComponent* RocketMovementComponent;

private:
};
