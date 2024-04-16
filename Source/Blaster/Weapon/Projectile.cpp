// Fill out your copyright notice in the Description page of Project Settings.


#include "Projectile.h"
#include "Components/BoxComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Particles/ParticleSystem.h"

// Sets default values
AProjectile::AProjectile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	//该Actor组件被网络复制（让子弹在客户端上生成但仍由服务器控制
	bReplicates = true;
	//初始化碰撞盒
	CollisionBox = CreateDefaultSubobject<UBoxComponent>("CollisionBox");
	//设置根组件
	SetRootComponent(CollisionBox);
	//更改此对象移动时使用的碰撞通道。由于子弹能够在空中飞行，所以设置为ECC_WorldDynamic
	CollisionBox->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	//启用碰撞
	CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	//先忽略所有通道的碰撞，再单独设置要碰撞的通道
	CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	//设置任何可见性和世界静止物体（如墙、地板）为可碰撞通道
	CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);
	//创建组件
	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	//让子弹保持其旋转与速度一致 （每帧更新其旋转 以匹配速度的方向）
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
}

void AProjectile::BeginPlay()
{
	Super::BeginPlay();
	
	/// 子弹生成时，生成特效并存储。
	if (Tracer)
	{
		//创建粒子附加器（附加例子特效）
		TracerComponent = UGameplayStatics::SpawnEmitterAttached(
			Tracer,  //粒子系统
			CollisionBox, //附加对象
			FName(),
			GetActorLocation(), //位置
			GetActorRotation(), //旋转
			EAttachLocation::KeepWorldPosition //附加方式  KeepWorldPosition：将在CollisionBox的位置生成并跟随CollisionBox
		);
	}
}
void AProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

