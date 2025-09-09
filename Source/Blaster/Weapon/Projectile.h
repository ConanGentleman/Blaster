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
	/// <summary>
	/// 子弹是一个复制的actor，在服务器上销毁一个复制的actor的行为会传播到所有客户端
	/// 销毁Actor（这里即子弹销毁）。也会在网络上通知本 Actor被摧毁，通知服务器在服务端和各客户端之间删除当前 Actor。即能够同步调用所有的客户端上的摧毁
	/// </summary>
	virtual void Destroyed() override;

	/**
	* Used with server-side rewind
	* 使用延迟补偿算法
	*/

	//是否开启延迟补偿
	bool bUseServerSideRewind = false;
	//初始发射位置
	FVector_NetQuantize TraceStart;
	//初始速度
	FVector_NetQuantize100 InitialVelocity;

	/// <summary>
	/// 初始速度
	/// </summary>
	UPROPERTY(EditAnywhere)
	float InitialSpeed = 15000;

protected:
	virtual void BeginPlay() override;
	void StartDestroyTimer();
	/// <summary>
	/// 销毁倒计时回调
	/// </summary>
	void DestroyTimerFinished();

	void SpawnTrailSystem();
	void ExplodeDamage();

	/// <summary>
	/// 子弹碰撞函数
	/// </summary>
	/// https://dev.epicgames.com/documentation/zh-cn/unreal-engine/events-in-unreal-engine
	/// <param name="HitComp">进行碰撞的组件（这里就是CollisionBox）</param>
	/// <param name="OtherActor">参与碰撞的其他 Actor</param>
	/// <param name="OtherComp">被击中的另一个组件</param>
	/// <param name="NormalImpulse">Actor 碰撞的力/param>
	/// <param name="Hit">击中结果</param>
	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	/// <summary>
	/// 子弹伤害
	/// </summary>
	UPROPERTY(EditAnywhere)
	float Damage = 20.f;


	/// <summary>
	/// 撞击特效
	/// </summary>
	UPROPERTY(EditAnywhere)
	class UParticleSystem* ImpactParticles;

	/// <summary>
	/// 撞击音效
	/// </summary>
	UPROPERTY(EditAnywhere)
	class USoundCue* ImpactSound;

	//子弹碰撞体
	UPROPERTY(EditAnywhere)
	class UBoxComponent* CollisionBox;

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
	/// 子弹运动组件(用来做抛物线或者子弹的组件)
	/// </summary>
	UPROPERTY(VisibleAnywhere)
	class UProjectileMovementComponent* ProjectileMovementComponent;

	/// <summary>
	/// 子弹网格体（子弹预制模型）
	/// </summary>
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* ProjectileMesh;

	/// <summary>
	/// 爆炸伤害内径
	/// </summary>
	UPROPERTY(EditAnywhere)
	float DamageInnerRadius = 200.f;

	/// <summary>
	/// 爆炸伤害外径
	/// </summary>
	UPROPERTY(EditAnywhere)
	float DamageOuterRadius = 500.f;

private:
	/// <summary>
	/// 子弹轨迹粒子特效
	/// </summary>
	UPROPERTY(EditAnywhere)
	UParticleSystem* Tracer;

	/// <summary>
	/// 用以存储基于Tracer生成后的粒子附加器
	/// 加上UPROPERTY()使其初始化为nullptr
	/// </summary>
	UPROPERTY()
	class UParticleSystemComponent* TracerComponent;

	/// <summary>
	/// 子弹销毁计时器
	/// </summary>
	FTimerHandle DestroyTimer;

	/// <summary>
	/// 子弹销毁倒计时时间
	/// </summary>
	UPROPERTY(EditAnywhere)
	float DestroyTime = 3.f;

public:	
	
};
