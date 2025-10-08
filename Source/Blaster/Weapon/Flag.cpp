// Fill out your copyright notice in the Description page of Project Settings.


#include "Flag.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"

AFlag::AFlag()
{
	//创建旗帜静态网格
	FlagMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FlagMesh"));
	//设置根组件
	SetRootComponent(FlagMesh);
	//将碰撞检测组件附加（绑定）到旗帜网格上
	GetAreaSphere()->SetupAttachment(FlagMesh);
	//将拾取控件组件附加（绑定）到旗帜网格上
	GetPickupWidget()->SetupAttachment(FlagMesh);
	//忽略碰撞
	FlagMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	FlagMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}