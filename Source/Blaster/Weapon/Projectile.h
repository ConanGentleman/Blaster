// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Projectile.generated.h"


/// <summary>
/// �ӵ���
/// </summary>
UCLASS()
class BLASTER_API AProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	AProjectile();
	virtual void Tick(float DeltaTime) override;
protected:
	virtual void BeginPlay() override;

private:
	//�ӵ���ײ��
	UPROPERTY(EditAnywhere)
	class UBoxComponent* CollisionBox;
	/// <summary>
	/// �ӵ��˶����(�����������߻����ӵ������)
	/// </summary>
	UPROPERTY(VisibleAnywhere)
	class UProjectileMovementComponent* ProjectileMovementComponent;
public:	
	
};
