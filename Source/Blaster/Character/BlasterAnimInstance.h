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
	//������beginplay
	virtual void NativeInitializeAnimation() override;
	//������Tick
	virtual void NativeUpdateAnimation(float DeltaTime) override;
private:
	//���ڲ�����˽�б�����ʹ��BlueprintReadOnly,�����Ҫͨ��meta��ָ������˽�з��ʣ�
	UPROPERTY(BlueprintReadOnly,Category=Character,meta=(AllowPrivateAccess="true"))
	class ABlasterCharacter* BlasterCharacter;
	/// <summary>
	/// �����ٶ�
	/// </summary>
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float Speed;
	/// <summary>
	/// �Ƿ��ڿ���
	/// </summary>
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bIsInAir;
	/// <summary>
	/// �Ƿ����ڼ��٣����ٶȣ�
	/// </summary>
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bIsAccelerating;
};
