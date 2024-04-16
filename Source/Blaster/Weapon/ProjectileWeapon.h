// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "ProjectileWeapon.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API AProjectileWeapon : public AWeapon
{
	GENERATED_BODY()
	
public:
	virtual void Fire(const FVector& HitTarget) override;

private:
	//TSubclassOf ���ṩ UClass ���Ͱ�ȫ�Ե�ģ���ࡣ�������ڴ���һ��Ͷ�����࣬���������ָ���˺����͡�
	//����ֻ����һ�� UClass ���͵� UPROPERTY���������ָ�������� UDamageType ���ࣻ��������ʹ�� TSubclassOf ģ��ǿ��Ҫ���ѡ��
	//����Խ� TSubclassOf ���ڴ洢��ĳ���ࣨ�������ࣩ�����á�
	UPROPERTY(EditAnywhere)
	TSubclassOf<class AProjectile> ProjectileClass;
};
