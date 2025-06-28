// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "ProjectileWeapon.generated.h"

/**
 * 火箭筒
 */
UCLASS()
class BLASTER_API AProjectileWeapon : public AWeapon
{
	GENERATED_BODY()
	
public:
	virtual void Fire(const FVector& HitTarget) override;

private:
	//TSubclassOf 是提供 UClass 类型安全性的模板类。例如您在创建一个投射物类，允许设计者指定伤害类型。
	//您可只创建一个 UClass 类型的 UPROPERTY，让设计者指定派生自 UDamageType 的类；或者您可使用 TSubclassOf 模板强制要求此选择
	//你可以将 TSubclassOf 用于存储对某个类（或其子类）的引用。
	UPROPERTY(EditAnywhere)
	TSubclassOf<class AProjectile> ProjectileClass;
};
