// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileGrenade.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

AProjectileGrenade::AProjectileGrenade()
{
	//创建网格对象
	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Grenade Mesh"));
	//将该组件附加到另一个组件上
	ProjectileMesh->SetupAttachment(RootComponent);
	//设置碰撞
	ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	//创建组件
	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	//让子弹保持其旋转与速度一致 （每帧更新其旋转 以匹配速度的方向）
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	//设置为复制
	ProjectileMovementComponent->SetIsReplicated(true);
	//是否可以弹射（不是接触瞬爆）
	ProjectileMovementComponent->bShouldBounce = true;
}

void AProjectileGrenade::BeginPlay()
{
	AActor::BeginPlay();
	//设置拖尾
	SpawnTrailSystem();
	//销毁倒计时开始
	StartDestroyTimer();
	//注册碰撞弹跳检测事件 
	ProjectileMovementComponent->OnProjectileBounce.AddDynamic(this, &AProjectileGrenade::OnBounce);
}

/// <summary>
/// 子弹碰撞弹跳检测事件回调
/// </summary>
/// <param name="ImpactResult"></param>
/// <param name="ImpactVelocity"></param>
void AProjectileGrenade::OnBounce(const FHitResult& ImpactResult, const FVector& ImpactVelocity)
{
	if (BounceSound)
	{
		UGameplayStatics::PlaySoundAtLocation( //在弹跳位置播放音效
			this,
			BounceSound,
			GetActorLocation()
		);
	}
}

void AProjectileGrenade::Destroyed()
{
	ExplodeDamage();//摧毁时造成爆炸伤害
	Super::Destroyed();
}