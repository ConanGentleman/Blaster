// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BuffComponent.generated.h"

/*
ClassGroup定义这个类在 编辑器中的分类。
Custom 表示它会被归类到 "Custom" 分组（在组件面板或类选择器中）。
meta 是元数据（metadata），用于提供额外的编辑器或运行时行为控制。
BlueprintSpawnableComponent 表示这个类 可以在蓝图中被创建（Spawn），通常用于 UActorComponent 派生类。
玩家buff类（回血、增加条约、移速、护盾等
*/
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class BLASTER_API UBuffComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UBuffComponent();
	/// <summary>
	/// 定义为friend class 这样可以直接访问ABlasterCharacter的私有成员
	/// </summary>
	friend class ABlasterCharacter;
	void Heal(float HealAmount, float HealingTime);
	/// <summary>
	/// 开始增速buff
	/// </summary>
	/// <param name="BuffBaseSpeed"></param>
	/// <param name="BuffCrouchSpeed"></param>
	/// <param name="BuffTime"></param>
	void BuffSpeed(float BuffBaseSpeed, float BuffCrouchSpeed, float BuffTime);
	/// <summary>
	/// 初始保存正常的速度
	/// </summary>
	/// <param name="BaseSpeed"></param>
	/// <param name="CrouchSpeed"></param>
	void SetInitialSpeeds(float BaseSpeed, float CrouchSpeed);
protected:
	virtual void BeginPlay() override;
	/// <summary>
	/// 回血buff
	/// </summary>
	/// <param name="DeltaTime"></param>
	void HealRampUp(float DeltaTime);
private:
	UPROPERTY()
	class ABlasterCharacter* Character;

	/**
	* 血量回复 buff
	*/

	bool bHealing = false;
	/// <summary>
	/// 每秒回复多少
	/// </summary>
	float HealingRate = 0;
	/// <summary>
	/// 计时器
	/// </summary>
	float AmountToHeal = 0.f;

	/**
	* 移速 buff
	*/

	/// <summary>
	/// 计时器
	/// </summary>
	FTimerHandle SpeedBuffTimer;
	/// <summary>
	/// buff结束后重置为正常速度
	/// </summary>
	void ResetSpeeds();
	/// <summary>
	/// 正常的步行速度
	/// </summary>
	float InitialBaseSpeed;
	/// <summary>
	/// 正常的蹲下行走速度
	/// </summary>
	float InitialCrouchSpeed;

	/// <summary>
	/// 多播设置速度
	/// 用于角色速度射中的 多播RPC。客户端调用，服务器执行的函数。如果在服务器上执行多播RPC，那么将在服务器以及所有客户端上调用。在定义时需在函数名后补充_Implementation
	/// reliable可靠的，发送失败会尝试重发
	/// </summary>
	UFUNCTION(NetMulticast, Reliable)
	void MulticastSpeedBuff(float BaseSpeed, float CrouchSpeed);

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;


};