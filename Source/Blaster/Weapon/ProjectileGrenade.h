// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Projectile.h"
#include "ProjectileGrenade.generated.h"

/**
 *
 */
UCLASS()
class BLASTER_API AProjectileGrenade : public AProjectile
{
	GENERATED_BODY()
public:
	/// <summary>
	/// ���캯��
	/// </summary>
	AProjectileGrenade();
	virtual void Destroyed() override;
protected:
	virtual void BeginPlay() override;

	/// <summary>
	/// �����ص����� ��UFUNCTION()��ʾ����ͼ���ӻ��ű�ͼ�������Ա㿪���ߴ���ͼ��Դ���û���չUFunction�����������C++����
	/// </summary>
	UFUNCTION()
	void OnBounce(const FHitResult& ImpactResult, const FVector& ImpactVelocity);
private:
	/// <summary>
	/// ��������
	/// </summary>
	UPROPERTY(EditAnywhere)
	USoundCue* BounceSound;
};