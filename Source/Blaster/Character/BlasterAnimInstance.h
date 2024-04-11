// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "BlasterAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API UBlasterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
public:
	//类似于beginplay
	virtual void NativeInitializeAnimation() override;
	//类似于Tick
	virtual void NativeUpdateAnimation(float DeltaTime) override;
private:
	//由于不能在私有变量上使用BlueprintReadOnly,因此需要通过meta来指定允许私有访问）
	UPROPERTY(BlueprintReadOnly,Category=Character,meta=(AllowPrivateAccess="true"))
	class ABlasterCharacter* BlasterCharacter;
	/// <summary>
	/// 人物速度
	/// </summary>
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float Speed;
	/// <summary>
	/// 是否在空中
	/// </summary>
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bIsInAir;
	/// <summary>
	/// 是否正在加速（加速度）
	/// </summary>
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bIsAccelerating;

	/// <summary>
	/// 是否装备有武器(便于在蓝图中判断进而切换到装备时的动画
	/// </summary>
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bWeaponEquipped;

	/// <summary>
	/// 是否是蹲下状态
	/// </summary>
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bIsCrouched;

	/// <summary>
	/// 是否正在瞄准
	/// </summary>
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bAiming;

	/// <summary>
	/// 控制人物拿着装备的移动的混合动画值（混合动画中的水平坐标
	/// </summary>
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float YawOffset;
	/// <summary>
	/// 控制人物拿着装备的倾斜的混合动画值（混合动画中的垂直坐标
	/// </summary>
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float Lean;

	/// <summary>
	/// 角色上一帧的旋转值
	/// </summary>
	FRotator CharacterRotationLastFrame;
	/// <summary>
	/// 角色旋转值。利用CharacterRotationLastFrame和CharacterRotation来控制角色的动画的倾斜程度
	/// </summary>
	FRotator CharacterRotation;
	/// <summary>
	/// 用于混合动画EquippedRun更加平滑的过度
	/// </summary>
	FRotator DeltaRotation;

	/// <summary>
	/// 用于目标偏移动画（叠加动画），也就是控制角色枪口方向的参数。 并且只会在角色静止（没有在跑或者跳跃）且装备武器时才会产生动画效果（视频规定的，避免移动时又倾斜身子，又转动身子，但我觉得并不合理）
	/// </summary>
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float AO_Yaw;

	/// <summary>
	/// 用于目标偏移动画（叠加动画），也就是控制角色枪口方向的参数。并且只会在角色静止（没有在跑或者跳跃）且装备武器时才会产生动画效果（视频规定的，避免移动时又倾斜身子，又转动身子，但我觉得并不合理）
	/// </summary>
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float AO_Pitch;
};
