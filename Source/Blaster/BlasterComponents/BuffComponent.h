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

	bool bHealing = false;
	/// <summary>
	/// 每秒回复多少
	/// </summary>
	float HealingRate = 0;
	float AmountToHeal = 0.f;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;


};