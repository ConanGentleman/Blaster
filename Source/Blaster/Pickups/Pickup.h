// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Pickup.generated.h"

UCLASS()
class BLASTER_API APickup : public AActor
{
	GENERATED_BODY()

public:
	APickup();
	virtual void Tick(float DeltaTime) override;
	virtual void Destroyed() override;
protected:
	virtual void BeginPlay() override;

	/// <summary>
	/// 碰撞体重叠函数回调（该函数应当只在服务端调用）。
	/// </summary>
	UFUNCTION()
	virtual void OnSphereOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	/// <summary>
	/// 每秒旋转率
	/// </summary>
	UPROPERTY(EditAnywhere)
	float BaseTurnRate = 45.f;

private:
	/// <summary>
	/// 球形碰撞体。重叠体积（碰撞体），用于角色靠近时，能够识别该能被捡起的物体
	/// </summary>
	UPROPERTY(EditAnywhere)
	class USphereComponent* OverlapSphere;
	/// <summary>
	/// 捡起音效
	/// </summary>
	UPROPERTY(EditAnywhere)
	class USoundCue* PickupSound;
	/// <summary>
	/// 网格
	/// </summary>
	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* PickupMesh;

	/// <summary>
	/// 特效Niagara组件
	/// </summary>
	UPROPERTY(VisibleAnywhere)
	class UNiagaraComponent* PickupEffectComponent;

	/// <summary>
	/// 血包被拾取时的动效
	/// </summary>
	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* PickupEffect;

	/// <summary>
	/// 重叠监听触发延时，避免刚一生成就直接被触发了
	/// </summary>
	FTimerHandle BindOverlapTimer;
	/// <summary>
	/// 延时时间
	/// </summary>
	float BindOverlapTime = 0.25f;
	void BindOverlapTimerFinished();

public:

};