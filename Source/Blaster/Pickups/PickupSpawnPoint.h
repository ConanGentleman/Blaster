// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PickupSpawnPoint.generated.h"


/*
在地图上生成各种拾取道具
*/
UCLASS()
class BLASTER_API APickupSpawnPoint : public AActor
{
	GENERATED_BODY()

public:
	APickupSpawnPoint();
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;

	/// <summary>
	/// 生成的拾取类
	/// </summary>
	UPROPERTY(EditAnywhere)
	TArray<TSubclassOf<class APickup>> PickupClasses;

	UPROPERTY()
	APickup* SpawnedPickup;

	/// <summary>
	/// 生成拾取物体
	/// </summary>
	void SpawnPickup();

	/// <summary>
	/// 拾取物体被拾取后计时完成触发的函数
	/// </summary>
	void SpawnPickupTimerFinished();

	/// <summary>
	/// 一个拾取道具被拾取后开始计时，计时结束后重新随随机生成新的道具（重新调用调用SpawnPickup）
	/// 回调委托必须加上UFUNCTION()
	/// </summary>
	/// <param name="DestroyedActor"></param>
	UFUNCTION()
	void StartSpawnPickupTimer(AActor* DestroyedActor);
private:
	FTimerHandle SpawnPickupTimer;

	/// <summary>
	/// 被拾取后生成的时间范围最小值
	/// </summary>
	UPROPERTY(EditAnywhere)
	float SpawnPickupTimeMin;
	/// <summary>
	/// 被拾取后生成的时间范围最大值
	/// </summary>
	UPROPERTY(EditAnywhere)
	float SpawnPickupTimeMax;
public:


};