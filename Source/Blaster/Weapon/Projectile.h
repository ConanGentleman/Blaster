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
protected:
	virtual void BeginPlay() override;

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
	/// 加上UPROPERTY()使其初始化为nullptr
	/// </summary>
	UPROPERTY()
	class UParticleSystemComponent* TracerComponent;

	/// <summary>
	/// 撞击特效
	/// </summary>
	UPROPERTY(EditAnywhere)
	UParticleSystem* ImpactParticles;

	/// <summary>
	/// 撞击音效
	/// </summary>
	UPROPERTY(EditAnywhere)
	class USoundCue* ImpactSound;
public:	
	
};
