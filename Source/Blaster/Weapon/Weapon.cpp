﻿// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Blaster/Character/BlasterCharacter.h"

// Sets default values
AWeapon::AWeapon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	//bReplicates 变量告知游戏此Actor应复制。Actor默认仅存在于生成它的机器上。
	//当 bReplicates 设为 True，只要Actor的权威副本存在于服务器上，就会尝试将该Actor复制到所有已连接的客户端。
	bReplicates = true;

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	//设置自己为根组件
	SetRootComponent(WeaponMesh);

	//能够丢弃武器，并且丢弃时会掉到地上，所以设置合理的碰撞通道。即对所有通道的碰撞相应进行阻止
	WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	//但是碰撞忽略Pawn
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	//武器该开始生成时禁用碰撞，知道玩家捡起并捡起丢下，才使用碰撞。所有该开始肯定是禁用状态
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	//创建球形检测重叠
	AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AreaSphere"));
	//绑定到根组件
	AreaSphere->SetupAttachment(RootComponent);

	//对于检测重叠事件最好在服务器上完成，（避免作弊）
	//区域球体对所有通道都不碰撞
	AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	//通过在构造函数中，所有机器上都禁用区域球体的碰撞，而在BeginPlay中，通过检测权限判断是否为服务器，再在服务器上启用碰撞
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	//武器捡起的提示说明
	PickupWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickupWidget"));
	//绑定到根组件
	PickupWidget->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void AWeapon::BeginPlay()
{
	Super::BeginPlay();
	
	//判断是否是服务器。可以使用GetLocalRole() == ENetRole::ROLE_Authority 也可以直接用HasAuthority()
	//因此服务器将负责所有武器对象,所以武器只能在服务器上拥有权限，因此需要在构造函数中设置bReplicates = true;
	////当 bReplicates 设为 True，只要Actor的权威副本存在于服务器上，就会尝试将该Actor复制到所有已连接的客户端。
	if (HasAuthority()) {
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		//设置对Pawn可以检测重叠事件（碰撞触发事件）
		AreaSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
		//添加开始重叠事件委托。参数：用户对象，回调函数
		AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &AWeapon::OnSphereOverlap);
		//添加结束重叠事件委托。参数：用户对象，回调函数
		AreaSphere->OnComponentEndOverlap.AddDynamic(this, &AWeapon::OnSphereEndOverlap);
	}
	if (PickupWidget)
	{
		PickupWidget->SetVisibility(false);
	}
}
// Called every frame
void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

/// <summary>
/// 碰撞体重叠函数回调（该函数应当只在服务端调用）。可见是在BeginPlay中的if(HasAuthority)后添加的委托，所以是保证了在服务器端调用的。
/// 但是目前只能保证在服务器上看到对PickupWidget的可见性修改，客户端上还无法看到，所以要使用到复制（replicaton）来进行
/// </summary>
/// <param name="OverlappedComp">重叠组件</param>
/// <param name="OtherActor">触发事件的主体</param>
/// <param name="OtherComp"></param>
/// <param name="OtherBodyIndexType"></param>
/// <param name="bFromSweep"></param>
/// <param name="SweepResult"></param>
void AWeapon::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor);
	//如果重叠的组件是BlasterCharacter，则修改可见性
	if (BlasterCharacter)
	{
		//PickupWidget->SetVisibility(true);
		BlasterCharacter->SetOverlappingWeapon(this);//设置OverlappingWeapon，因为刚开始BlasterCharacter中的OverlappingWeapon为空
	}
}
/// <summary>
/// 碰撞体重叠结束函数回调（该函数应当只在服务端调用）
/// </summary>
/// <param name="OverlappedComp"></param>
/// <param name="OtherActor"></param>
/// <param name="OtherComp"></param>
/// <param name="OtherBodyIndex"></param>
void AWeapon::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor);
	//如果重叠的组件是BlasterCharacter，则修改可见性
	if (BlasterCharacter)
	{
		//PickupWidget->SetVisibility(true);
		BlasterCharacter->SetOverlappingWeapon(nullptr);//设置OverlappingWeapon，因为离开了 所以为空
	}
}

void AWeapon::ShowPickupWidget(bool bShowWidget)
{
	if (PickupWidget) {
		PickupWidget->SetVisibility(bShowWidget);
	}
}
