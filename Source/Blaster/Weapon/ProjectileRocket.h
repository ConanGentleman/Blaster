// Fill out your copyright notice in the Description page of Project Settings.

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
	/// 销毁倒计时回调
	/// </summary>
	void DestroyTimerFinished();

	/// <summary>
	/// 用于火箭弹拖尾的特效
	/// </summary>
	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* TrailSystem;

	/// <summary>
	/// 用于火箭弹拖尾特效的组件
	/// </summary>
	UPROPERTY()
	class UNiagaraComponent* TrailSystemComponent;

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
private:
	/// <summary>
	/// 子弹网格体（子弹预制模型）
	/// </summary>
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* RocketMesh;

	/// <summary>
	/// 子弹销毁计时器
	/// </summary>
	FTimerHandle DestroyTimer;

	/// <summary>
	/// 子弹销毁倒计时时间
	/// </summary>
	UPROPERTY(EditAnywhere)
	float DestroyTime = 3.f;
};
