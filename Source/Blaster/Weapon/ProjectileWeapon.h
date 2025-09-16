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
	
	/// <summary>
	/// 子弹
	/// </summary>
	UPROPERTY(EditAnywhere)
	TSubclassOf<class AProjectile> ProjectileClass;

	/// <summary>
	/// 可以进行延迟补偿的子弹(不会设置为复制变量
	/// </summary>
	UPROPERTY(EditAnywhere)
	TSubclassOf<AProjectile> ServerSideRewindProjectileClass;
};
//对于武器、武器子弹其服务器倒带、子弹是否复制会出现以下几种情况
/*
一、火箭筒类武器使用SSR（服务器倒带算法）
1. 对于服务器上本地控制的角色来说，其生成的子弹不会使用延迟补偿算法（倒带）//这样就不会在服务器上发送服务器得分请求//，但是复制变量。直接造成伤害，并将复制到客户端
2. 对于服务器上不是本地控制的橘色来说，其生成的子弹使用延迟补偿算法（倒带）//这样就不会在服务器上发送服务器得分请求//，且不是复制变量。
3. 对于客户端上本地控制的角色来说，其生成的子弹将使用延迟补偿算法（倒带），且不是复制变量。当弹丸在机器上本地击中其他玩家时，使用倒带，请求服务器分数，服务器对其进行验证
4. 对于客户端上非本地控制的角色来说，其生成的子弹不会使用延迟补偿算法（倒带），且不是复制变量。因为开火是一种多播RPC，武器会在别的电脑上直接开火
二、火箭筒类武器不使用SSR算法
1. 在服务器上无论是否是本地控制，都将仅产生可复制的弹丸，且不使用倒带算法。因为客户端开火，子弹仍将在服务器上产生并复制到客户端
2. 在客户端上不会有子弹产生，但是由于服务器会产生复制的弹丸，因此在客户端上也会产生
*/

