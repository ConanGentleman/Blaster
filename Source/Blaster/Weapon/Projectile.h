// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Projectile.generated.h"


/// <summary>
/// 子弹类
/// </summary>
UCLASS()
class BLASTER_API AProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	AProjectile();
	virtual void Tick(float DeltaTime) override;
protected:
	virtual void BeginPlay() override;

private:
	//子弹碰撞体
	UPROPERTY(EditAnywhere)
	class UBoxComponent* CollisionBox;
	/// <summary>
	/// 子弹运动组件(用来做抛物线或者子弹的组件)
	/// </summary>
	UPROPERTY(VisibleAnywhere)
	class UProjectileMovementComponent* ProjectileMovementComponent;

	/// <summary>
	/// 子弹轨迹粒子特效
	/// </summary>
	UPROPERTY(EditAnywhere)
	class UParticleSystem* Tracer;

	/// <summary>
	/// 用以存储基于Tracer生成后的粒子附加器
	/// </summary>
	class UParticleSystemComponent* TracerComponent;
public:	
	
};
