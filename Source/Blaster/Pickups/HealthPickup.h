// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pickup.h"
#include "HealthPickup.generated.h"

/**
 * 血包拾取物
 */
UCLASS()
class BLASTER_API AHealthPickup : public APickup
{
	GENERATED_BODY()
public:
	AHealthPickup();
	virtual void Destroyed() override;
protected:
	/// <summary>
	/// 重叠事件回调
	/// </summary>
	/// <param name="OverlappedComponent"></param>
	/// <param name="OtherActor"></param>
	/// <param name="OtherComp"></param>
	/// <param name="OtherBodyIndex"></param>
	/// <param name="bFromSweep"></param>
	/// <param name="SweepResult"></param>
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
	/// 血包治愈的血量
	/// </summary>
	UPROPERTY(EditAnywhere)
	float HealAmount = 100.f;

	/// <summary>
	/// 血量在多久内恢复到HealAmount
	/// </summary>
	UPROPERTY(EditAnywhere)
	float HealingTime = 5.f;

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
};