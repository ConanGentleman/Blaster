// Fill out your copyright notice in the Description page of Project Settings.


#include "Projectile.h"
#include "Components/BoxComponent.h"

// Sets default values
AProjectile::AProjectile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
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
}

void AProjectile::BeginPlay()
{
	Super::BeginPlay();
	
}
void AProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

