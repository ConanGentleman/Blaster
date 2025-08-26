// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LagCompensationComponent.generated.h"


/// 碰撞框信息
USTRUCT(BlueprintType)
struct FBoxInformation
{
	GENERATED_BODY()

	UPROPERTY()
	FVector Location;

	UPROPERTY()
	FRotator Rotation;

	UPROPERTY()
	FVector BoxExtent;
};

/// 延迟补偿数据包
USTRUCT(BlueprintType)
struct FFramePackage
{
	GENERATED_BODY()

	///某帧时间
	UPROPERTY()
	float Time;

	/// <summary>
	/// 帧时间角色所有碰撞框信息
	/// </summary>
	UPROPERTY()
	TMap<FName, FBoxInformation> HitBoxInfo;
};


UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class BLASTER_API ULagCompensationComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	ULagCompensationComponent();
	friend class ABlasterCharacter;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	/// <summary>
	/// 打印数据包中包含的命中狂（显示用，方便查看而已）
	/// </summary>
	/// <param name="Package"></param>
	/// <param name="Color"></param>
	void ShowFramePackage(const FFramePackage& Package, const FColor& Color);

protected:
	virtual void BeginPlay() override;
	/// <summary>
	/// 存储数据包
	/// </summary>
	/// <param name="Package"></param>
	/// <param name="Color"></param>
	void SaveFramePackage(FFramePackage& Package);
private:

	UPROPERTY()
	ABlasterCharacter* Character;

	UPROPERTY()
	class ABlasterPlayerController* Controller;

public:


};