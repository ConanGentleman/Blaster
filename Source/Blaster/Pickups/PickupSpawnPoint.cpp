// Fill out your copyright notice in the Description page of Project Settings.


#include "PickupSpawnPoint.h"
#include "Pickup.h"

APickupSpawnPoint::APickupSpawnPoint()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
}

void APickupSpawnPoint::BeginPlay()
{
	Super::BeginPlay();
	//这种传强转空指针的方式不好，但必须要有一个开始生成的调用
	StartSpawnPickupTimer((AActor*)nullptr);
}

/// <summary>
/// 生成拾取物体
/// </summary>
void APickupSpawnPoint::SpawnPickup()
{
	int32 NumPickupClasses = PickupClasses.Num();
	if (NumPickupClasses > 0)
	{
		//随机选取一个拾取类
		int32 Selection = FMath::RandRange(0, NumPickupClasses - 1);
		SpawnedPickup = GetWorld()->SpawnActor<APickup>(PickupClasses[Selection], GetActorTransform());

		if (HasAuthority() && SpawnedPickup)
		{
			//注册拾取物体销毁事件，当物体销毁后触发StartSpawnPickupTimer（即一个拾取道具被拾取后开始计时，计时结束后重新随随机生成新的道具
			SpawnedPickup->OnDestroyed.AddDynamic(this, &APickupSpawnPoint::StartSpawnPickupTimer);
		}
	}
}

/// <summary>
/// 拾取物体被拾取后计时完成触发的函数
/// </summary>
void APickupSpawnPoint::SpawnPickupTimerFinished()
{
	if (HasAuthority())
	{
		SpawnPickup();
	}
}

/// <summary>
/// 一个拾取道具被拾取后开始计时，计时结束后重新随机生成新的道具（重新调用调用SpawnPickup）
/// </summary>
/// <param name="DestroyedActor"></param>
void APickupSpawnPoint::StartSpawnPickupTimer(AActor* DestroyedActor)
{
	//随机时间后生成新的道具
	const float SpawnTime = FMath::FRandRange(SpawnPickupTimeMin, SpawnPickupTimeMax);
	GetWorldTimerManager().SetTimer(
		SpawnPickupTimer,
		this,
		&APickupSpawnPoint::SpawnPickupTimerFinished,
		SpawnTime
	);
}

void APickupSpawnPoint::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}