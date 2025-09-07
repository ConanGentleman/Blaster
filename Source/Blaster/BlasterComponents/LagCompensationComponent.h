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

	/// <summary>
	/// 对应的角色
	/// </summary>
	UPROPERTY()
	ABlasterCharacter* Character;
};

// 延迟补充算法-倒带击中结果数据包
USTRUCT(BlueprintType)
struct FServerSideRewindResult
{
	GENERATED_BODY()

	/// <summary>
	/// 是否命中
	/// </summary>
	UPROPERTY()
	bool bHitConfirmed;

	/// <summary>
	/// 是否命中的是头部
	/// </summary>
	UPROPERTY()
	bool bHeadShot;
};

// 延迟补充算法-霰弹枪倒带击中结果数据包
USTRUCT(BlueprintType)
struct FShotgunServerSideRewindResult
{
	GENERATED_BODY()

	/// <summary>
	/// 各玩家头部命中次数
	/// </summary>
	UPROPERTY()
	TMap<ABlasterCharacter*, uint32> HeadShots;

	/// <summary>
	/// 各玩家身体命中次数
	/// </summary>
	UPROPERTY()
	TMap<ABlasterCharacter*, uint32> BodyShots;

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
	FServerSideRewindResult ServerSideRewind(
		class ABlasterCharacter* HitCharacter,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize& HitLocation,
		float HitTime);


	/// <summary>
	/// 服务器霰弹枪延迟补偿倒带回放
	/// </summary>
	/// <param name="HitCharacters"></param>
	/// <param name="TraceStart"></param>
	/// <param name="HitLocations"></param>
	/// <param name="HitTime"></param>
	/// <returns></returns>
	FShotgunServerSideRewindResult ShotgunServerSideRewind(
		const TArray<ABlasterCharacter*>& HitCharacters,
		const FVector_NetQuantize& TraceStart,
		const TArray<FVector_NetQuantize>& HitLocations,
		float HitTime);

	/// <summary>
	/// 服务器伤害/请求RPC（通过倒带算法判定击中玩家后，进行伤害判定和得分结算
	/// </summary>
	UFUNCTION(Server, Reliable)
		void ServerScoreRequest(
			ABlasterCharacter* HitCharacter, //击中的玩家
			const FVector_NetQuantize& TraceStart,//检测判定开始位置
			const FVector_NetQuantize& HitLocation,//击中位置
			float HitTime, //击中的时间用于倒带算法倒回到HitTime进行计算
			class AWeapon* DamageCauser //造成击中的武器
		);

	/// <summary>
	/// 服务器霰弹枪伤害/请求RPC（通过倒带算法判定击中玩家后，进行伤害判定和得分结算
	/// </summary>
	UFUNCTION(Server, Reliable)
		void ShotgunServerScoreRequest(
			const TArray<ABlasterCharacter*>& HitCharacters,
			const FVector_NetQuantize& TraceStart,
			const TArray<FVector_NetQuantize>& HitLocations,
			float HitTime
		);
protected:
	virtual void BeginPlay() override;
	/// <summary>
	/// 赋值单个延迟补偿数据包
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

	/// <summary>
	/// 击中判定
	/// </summary>
	/// <param name="Package"></param>
	/// <param name="HitCharacter"></param>
	/// <param name="TraceStart"></param>
	/// <param name="HitLocation"></param>
	/// <returns></returns>
	FServerSideRewindResult ConfirmHit(
		const FFramePackage& Package,
		ABlasterCharacter* HitCharacter,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize& HitLocation);
	/// <summary>
	/// 将HitCharacter中的命中框信息赋值给OutFramePackage
	/// </summary>
	/// <param name="HitCharacter"></param>
	/// <param name="OutFramePackage"></param>
	void CacheBoxPositions(ABlasterCharacter* HitCharacter, FFramePackage& OutFramePackage);
	/// <summary>
	/// 移动所有命中框，即将Package中的命中框位置旋转等信息赋值给与HitCharacter的命中框
	/// 与CacheBoxPositions函数作用相反
	/// </summary>
	/// <param name="HitCharacter"></param>
	/// <param name="Package"></param>
	void MoveBoxes(ABlasterCharacter* HitCharacter, const FFramePackage& Package);
	/// <summary>
	/// 重置所有命中框
	/// </summary>
	/// <param name="HitCharacter"></param>
	/// <param name="Package"></param>
	void ResetHitBoxes(ABlasterCharacter* HitCharacter, const FFramePackage& Package);
	/// <summary>
	/// 开启或关闭角色网格的碰撞检测(在倒带进行击中判定时需要关闭网格碰撞来避免影响到命中框命中的判定）
	/// </summary>
	/// <param name="HitCharacter"></param>
	/// <param name="CollisionEnabled"></param>
	void EnableCharacterMeshCollision(ABlasterCharacter* HitCharacter, ECollisionEnabled::Type CollisionEnabled);
	/// <summary>
	/// 存储延迟补偿（倒带算法）历史数据
	/// </summary>
	void SaveFramePackage();

	/// <summary>
	/// 获取延迟补偿应该检测哪一个帧数据
	/// </summary>
	/// <param name="HitCharacter"></param>
	/// <param name="HitTime"></param>
	/// <returns></returns>
	FFramePackage GetFrameToCheck(ABlasterCharacter* HitCharacter, float HitTime);

	/**
	* Shotgun霰弹枪延迟补偿
	*/

	/// <summary>
	/// 霰弹枪延迟补偿伤害判定
	/// </summary>
	/// <param name="FramePackages"></param>
	/// <param name="TraceStart"></param>
	/// <param name="HitLocations"></param>
	/// <returns></returns>
	FShotgunServerSideRewindResult ShotgunConfirmHit(
		const TArray<FFramePackage>& FramePackages,
		const FVector_NetQuantize& TraceStart,
		const TArray<FVector_NetQuantize>& HitLocations
	);
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