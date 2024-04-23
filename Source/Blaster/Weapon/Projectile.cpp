// Fill out your copyright notice in the Description page of Project Settings.


#include "Projectile.h"
#include "Components/BoxComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Particles/ParticleSystem.h"
#include "Sound/SoundCue.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/Blaster.h"

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
	//自定义object通道提出的主要原因: 角色进行碰撞的是胶囊体组件（规则胶囊体），而非网格组件（人物三维模型）。
	//希望子弹能够选择性的选择它现在能击中的物体，因此自定义了一个新的object通道（在项目设置中设置）
	//对于墙壁和可见性的东西确实需要进行碰撞的，所以上面并没有进行删除。
	//使用自定义的object通道，并设置能进行碰撞。
	CollisionBox->SetCollisionResponseToChannel(ECC_SkeletalMesh, ECollisionResponse::ECR_Block);
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
	//仅允许在服务器上进行子弹的碰撞检测
	if (HasAuthority())
	{
		//注册碰撞检测事件
		CollisionBox->OnComponentHit.AddDynamic(this, &AProjectile::OnHit);
	}
}
/// <summary>
/// 子弹碰撞函数
/// </summary>
/// https://dev.epicgames.com/documentation/zh-cn/unreal-engine/events-in-unreal-engine
/// <param name="HitComp">进行碰撞的组件（这里就是CollisionBox）</param>
/// <param name="OtherActor">参与碰撞的其他 Actor</param>
/// <param name="OtherComp">被击中的另一个组件</param>
/// <param name="NormalImpulse">Actor 碰撞的力/param>
/// <param name="Hit">击中结果</param>
void AProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	////子弹碰撞后，如果otheractor类型转换为ABlasterCharacter成功，即BlasterCharacter不为空，则子弹碰撞到的是角色。
	//ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor);
	//if (BlasterCharacter)
	//{
	//	//让角色播放受伤动画。调用多播RPC
	//	BlasterCharacter->MulticastHit();
	//}
	//撞击后销毁子弹
	Destroy();
}
void AProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}
/// <summary>
/// 子弹是一个复制的actor，在服务器上销毁一个复制的actor的行为会传播到所有客户端
/// 销毁Actor（子弹销毁) ，也会在网络上通知本 Actor被摧毁，通知服务器在服务端和各客户端之间删除当前 Actor。即能够同步调用所有的客户端上的摧毁
/// </summary>
void AProjectile::Destroyed()
{
	Super::Destroyed();

	if (ImpactParticles)
	{
		//播放特效
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, GetActorTransform());
	}
	if (ImpactSound)
	{
		//播放音效
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
	}
}