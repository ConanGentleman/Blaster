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

	/// <summary>
	/// 框体大小
	/// </summary>
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

	/// <summary>
	/// 服务器延迟补偿倒带回放
	/// </summary>
	/// <param name="HitCharacter">击中的角色</param>
	/// <param name="TraceStart">击中检测起始位置</param>
	/// <param name="HitLocation">击中位置</param>
	/// <param name="HitTime">击中时间</param>
	void ServerSideRewind(
		class ABlasterCharacter* HitCharacter,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize& HitLocation,
		float HitTime);
protected:
	virtual void BeginPlay() override;
	/// <summary>
	/// 存储数据包
	/// </summary>
	/// <param name="Package"></param>
	/// <param name="Color"></param>
	void SaveFramePackage(FFramePackage& Package);

	/// <summary>
	/// 依据击中时间进行插值以计算命中框的位置、旋转等信息
	/// </summary>
	/// <param name="OlderFrame"></param>
	/// <param name="YoungerFrame"></param>
	/// <param name="HitTime"></param>
	/// <returns></returns>
	FFramePackage InterpBetweenFrames(const FFramePackage& OlderFrame, const FFramePackage& YoungerFrame, float HitTime);

private:

	UPROPERTY()
	ABlasterCharacter* Character;

	UPROPERTY()
	class ABlasterPlayerController* Controller;

	/// <summary>
	/// 历史记录的延迟补偿数据包
	/// 双向链表
	/// </summary>
	TDoubleLinkedList<FFramePackage> FrameHistory;

	/// <summary>
	/// 历史记录内包含的帧数据最大记录时间差
	/// </summary>
	UPROPERTY(EditAnywhere)
	float MaxRecordTime = 4.f;

public:


};