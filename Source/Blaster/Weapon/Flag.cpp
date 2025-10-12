// Fill out your copyright notice in the Description page of Project Settings.


#include "Flag.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Blaster/Character/BlasterCharacter.h"

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

void AFlag::Dropped()
{
	//设置武器状态。
	SetWeaponState(EWeaponState::EWS_Dropped);
	//组件分离规则。EDetachmentRule::KeepWorld表示武器会保持其世界变换
	FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld, true);
	//旗帜网格从组件分离，传入分离规则
	FlagMesh->DetachFromComponent(DetachRules);
	//设置所有者为空
	SetOwner(nullptr);
	//武器被丢弃后就不应该有所有者了
	BlasterOwnerCharacter = nullptr;
	BlasterOwnerController = nullptr;
}

/// <summary>
/// 重置旗帜位置（旗帜运回得分后，需要重置回地方位置）
/// </summary>
void AFlag::ResetFlag()
{
	ABlasterCharacter* FlagBearer = Cast<ABlasterCharacter>(GetOwner());
	//清掉所属玩家的持旗状态
	if (FlagBearer)
	{
		FlagBearer->SetHoldingTheFlag(false);
		//设置碰撞的武器为null，不然重置旗帜后，玩家尽管玩家没有碰撞到任何武器，依旧可以按E捡起
		FlagBearer->SetOverlappingWeapon(nullptr);
		FlagBearer->UnCrouch();
	}

	if (!HasAuthority()) return;
	//设置相关碰撞属性
	FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld, true);
	FlagMesh->DetachFromComponent(DetachRules);
	SetWeaponState(EWeaponState::EWS_Initial);//旗帜置为初始状态
	//旗帜设置为初始状态后并不会触发OnWeaponStateSet函数中的case，因此这里需要手动设置一下碰撞参数
	GetAreaSphere()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetAreaSphere()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
	//清空所属玩家
	SetOwner(nullptr);
	BlasterOwnerCharacter = nullptr;
	BlasterOwnerController = nullptr;
	//重置回初始位置
	SetActorTransform(InitialTransform);
}

void AFlag::OnEquipped()
{
	//装备旗帜后，隐藏其文字提示
	ShowPickupWidget(false);
	//装备旗帜后，禁用球体碰撞器（避免装备上的武器仍然能够检测重叠）
	GetAreaSphere()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	//不用像CombatComponent再设置Owener。因为本身Owener也用ReplicatedUsing标记了，所以在服务器上设置时，客户端也同步了
	//关闭武器物理模拟
	FlagMesh->SetSimulatePhysics(false);
	//关闭武器受重力影响
	FlagMesh->SetEnableGravity(false);
	//设置网格无法碰撞
	FlagMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	FlagMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Overlap);
	//禁用深度颜色（轮廓）
	EnableCustomDepth(false);
}

void AFlag::OnDropped()
{
	/*
	现在set weapon state在服务器端被调用，但我们不知道是否会在客户端调用set weapons state。
	所以我们需要确保，如果我们为区域球体启用碰撞，我们只在服务器上执行，这样我们就可以确保我们首先在服务器上执行。
	*/
	if (HasAuthority())  //武器被丢掉后，在服务器上启动球碰撞器
	{
		GetAreaSphere()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}
	//开启武器物理模拟
	FlagMesh->SetSimulatePhysics(true);
	//开启受重力影响
	FlagMesh->SetEnableGravity(true);
	//开启旗帜网格碰撞
	FlagMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	//能够丢弃旗帜，并且丢弃时会掉到地上，所以设置合理的碰撞通道。即对所有通道的碰撞相应进行阻止
	FlagMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	//但是碰撞忽略Pawn
	FlagMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	//但是碰撞忽略相机
	FlagMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);

	FlagMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_BLUE);
	//用于标记渲染状态为脏状态，这意味着在当前帧结束时会将其发送到渲染线程
	//通常在需要更新组件的视觉表现时使用，例如更改材质或变换
	FlagMesh->MarkRenderStateDirty();
	//丢弃旗帜时开启轮廓显示
	EnableCustomDepth(true);

}

void AFlag::BeginPlay()
{
	Super::BeginPlay();
	InitialTransform = GetActorTransform();
}