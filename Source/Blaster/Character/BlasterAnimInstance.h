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
};
