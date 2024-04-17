// Fill out your copyright notice in the Description page of Project Settings.


#include "Casing.h"

// Sets default values
ACasing::ACasing()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;//不需要每帧运行
	//创建组件
	CasingMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CasingMesh"));
	//设置根组件
	SetRootComponent(CasingMesh);

}

// Called when the game starts or when spawned
void ACasing::BeginPlay()
{
	Super::BeginPlay();
	
}
//
//void ACasing::Tick(float DeltaTime)
//{
//	Super::Tick(DeltaTime);
//
//}

