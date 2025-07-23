// Fill out your copyright notice in the Description page of Project Settings.


#include "Pickup.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Components/SphereComponent.h"


APickup::APickup()
{
	PrimaryActorTick.bCanEverTick = true;
	//bReplicates 变量告知游戏此Actor应复制。Actor默认仅存在于生成它的机器上。
	//当 bReplicates 设为 True，只要Actor的权威副本存在于服务器上，就会尝试将该Actor复制到所有已连接的客户端。
	bReplicates = true;
	//创建根组件（构建一个场景作为根组件，而不是让球体或者其他组件成为根组件
	//这里好像如果将USceneComponent组件设置为父组件，则基于该类的蓝图组件拖拽到场景内的话，会直接附着在地面，而不是像武器类一下，需要调整位置
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	//创建球形碰撞体
	OverlapSphere = CreateDefaultSubobject<USphereComponent>(TEXT("OverlapSphere"));
	//设置父组件为根组件
	OverlapSphere->SetupAttachment(RootComponent);
	//设置球形大小
	OverlapSphere->SetSphereRadius(150.f);
	//设置碰撞状态（开启但只查询
	OverlapSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	//忽略所有碰撞
	OverlapSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	//但是可与玩家进行碰撞检测
	OverlapSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
	//创建网格
	PickupMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PickupMesh"));
	//附着在球型组件上
	PickupMesh->SetupAttachment(OverlapSphere);
	//关闭碰撞
	PickupMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void APickup::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		//添加重叠监听
		OverlapSphere->OnComponentBeginOverlap.AddDynamic(this, &APickup::OnSphereOverlap);
	}
}

/// <summary>
/// 碰撞体重叠函数回调（该函数应当只在服务端调用）。
/// </summary>
void APickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{

}

void APickup::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void APickup::Destroyed()
{
	Super::Destroyed();

	if (PickupSound)
	{
		//播放捡起音效
		UGameplayStatics::PlaySoundAtLocation(
			this,
			PickupSound,
			GetActorLocation()
		);
	}
}