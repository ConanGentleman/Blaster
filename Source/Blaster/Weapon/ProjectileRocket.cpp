// Fill out your copyright notice in the Description page of Project Settings.
/// 火箭弹

#include "ProjectileRocket.h"
#include "Kismet/GameplayStatics.h"

AProjectileRocket::AProjectileRocket()
{
	//创建对象
	RocketMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Rocket Mesh"));
	//将该组件附加到另一个组件上
	RocketMesh->SetupAttachment(RootComponent);
	//设置碰撞
	RocketMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

/// <summary>
/// 子弹击中函数
/// </summary>
/// <param name="HitComp"></param>
/// <param name="OtherActor"></param>
/// <param name="OtherComp"></param>
/// <param name="NormalImpulse"></param>
/// <param name="Hit"></param>
void AProjectileRocket::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// 获取火箭筒的释放者（该释放者是在OrijectileWeapon的Fire函数中被设置的）
	APawn* FiringPawn = GetInstigator(); 
	if (FiringPawn)
	{
		AController* FiringController = FiringPawn->GetController();
		if (FiringController)
		{
			UGameplayStatics::ApplyRadialDamageWithFalloff( //造成圆形爆炸伤害
				this, // World context object
				Damage, // BaseDamage 基础伤害（父类的一个成员变量
				10.f, // MinimumDamage 最小伤害
				GetActorLocation(), // Origin 爆炸原点
				200.f, // DamageInnerRadius 伤害内径
				500.f, // DamageOuterRadius 伤害外径（在外径内收到最小伤害
				1.f, // DamageFalloff 伤害衰减（线性
				UDamageType::StaticClass(), // DamageTypeClass 上海类型
				TArray<AActor*>(), // IgnoreActors 忽略伤害的玩家
				this, // DamageCauser 造成伤害的原因
				FiringController // InstigatorController 释放者控制器
			);
		}
	}
	Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);
}