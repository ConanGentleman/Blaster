// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "RocketMovementComponent.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API URocketMovementComponent : public UProjectileMovementComponent
{
	GENERATED_BODY()
protected:
    /// <summary>
    /// 处理模拟更新期间的阻止命中。检查模拟在碰撞后是否仍然有效。
    /// 如果进行模拟，则调用 HandleImpact（），并默认返回 EHandleHitWallResult： : D eflect，以通过 HandleDeflection（） 启用多次弹跳和滑动支持。
    /// 如果不再模拟，则返回 EHandleHitWallResult：：Abort，这将中止进一步模拟的尝试。
    /// 
    /// 处理移动过程中遇到的阻挡碰撞，主要是用来解决玩家自己发出的子弹碰撞到玩家导致停止运动
    /// 其中返回值存在一种返回结果枚举AdvanceNextSubstep
    /// 该枚举结果表示 前进到下一个模拟更新。通常在可以忽略其他 slide/multi-bounce logic 时使用，例如，当阻挡弹丸的物体被摧毁并且应该继续移动而不是停止时。
    /// 
    /// </summary>
    virtual EHandleBlockingHitResult HandleBlockingHit(const FHitResult& Hit, float TimeTick, const FVector& MoveDelta, float& SubTickTimeRemaining) override;

    virtual void HandleImpact(const FHitResult& Hit, float TimeSlice = 0.f, const FVector& MoveDelta = FVector::ZeroVector) override;
};
