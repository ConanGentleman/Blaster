// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Projectile.h"
#include "ProjectileRocket.generated.h"

/**
 * �����
 */
UCLASS()
class BLASTER_API AProjectileRocket : public AProjectile
{
	GENERATED_BODY()
public:
	AProjectileRocket();//���캯��
protected:
	/// <summary>
	/// �ӵ���װ����
	/// </summary>
	/// <param name="HitComp"></param>
	/// <param name="OtherActor"></param>
	/// <param name="OtherComp"></param>
	/// <param name="NormalImpulse"></param>
	/// <param name="Hit"></param>
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) override;
private:
	/// <summary>
	/// �ӵ������壨�ӵ�Ԥ��ģ�ͣ�
	/// </summary>
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* RocketMesh;
};
