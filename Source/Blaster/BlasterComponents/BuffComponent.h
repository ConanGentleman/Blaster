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
	void ReplenishShield(float ShieldAmount, float ReplenishTime);
	/// <summary>
	/// 开始增速buff
	/// </summary>
	/// <param name="BuffBaseSpeed"></param>
	/// <param name="BuffCrouchSpeed"></param>
	/// <param name="BuffTime"></param>
	void BuffSpeed(float BuffBaseSpeed, float BuffCrouchSpeed, float BuffTime);
	/// <summary>
	/// 开始跳跃buff
	/// </summary>
	/// <param name="BuffJumpVelocity"></param>
	/// <param name="BuffTime"></param>
	void BuffJump(float BuffJumpVelocity, float BuffTime);

	/// <summary>
	/// 初始保存正常的速度
	/// </summary>
	/// <param name="BaseSpeed"></param>
	/// <param name="CrouchSpeed"></param>
	void SetInitialSpeeds(float BaseSpeed, float CrouchSpeed);
	/// <summary>
	/// 初始保存正常的跳跃速度（高度）
	/// </summary>
	/// <param name="Velocity"></param>
	void SetInitialJumpVelocity(float Velocity);
protected:
	virtual void BeginPlay() override;
	/// <summary>
	/// 回血buff
	/// </summary>
	/// <param name="DeltaTime"></param>
	void HealRampUp(float DeltaTime);
	/// <summary>
	/// 回护盾buff
	/// </summary>
	/// <param name="DeltaTime"></param>
	void ShieldRampUp(float DeltaTime);
private:
	UPROPERTY()
	class ABlasterCharacter* Character;

	/**
	* 血量回复 buff
	*/

	/// <summary>
	/// 是否正在回血
	/// </summary>
	bool bHealing = false;
	/// <summary>
	/// 每秒回复多少
	/// </summary>
	float HealingRate = 0.f;
	/// <summary>
	/// 计时器（也就是记录回复多少血量了
	/// </summary>
	float AmountToHeal = 0.f;

	/**
	* 护盾回复 buff
	*/

	/// <summary>
	/// 是否正在回护盾
	/// </summary>
	bool bReplenishingShield = false;
	/// <summary>
	/// 每秒回复多少
	/// </summary>
	float ShieldReplenishRate = 0.f;
	/// <summary>
	/// 计时器（也就是记录回复多少护盾了
	/// </summary>
	float ShieldReplenishAmount = 0.f;

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
	/// 用于角色速度设置的 多播RPC。客户端调用，服务器执行的函数。如果在服务器上执行多播RPC，那么将在服务器以及所有客户端上调用。在定义时需在函数名后补充_Implementation
	/// reliable可靠的，发送失败会尝试重发
	/// </summary>
	UFUNCTION(NetMulticast, Reliable)
	void MulticastSpeedBuff(float BaseSpeed, float CrouchSpeed);

	/**
	* 跳跃 buff
	*/
	FTimerHandle JumpBuffTimer;
	void ResetJump();
	float InitialJumpVelocity;

	/// <summary>
	/// 多播设置跳跃速度（高度
	/// 用于角色跳跃设置的 多播RPC。客户端调用，服务器执行的函数。如果在服务器上执行多播RPC，那么将在服务器以及所有客户端上调用。在定义时需在函数名后补充_Implementation
	/// reliable可靠的，发送失败会尝试重发
	/// </summary>
	UFUNCTION(NetMulticast, Reliable)
	void MulticastJumpBuff(float JumpVelocity);


public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;


};