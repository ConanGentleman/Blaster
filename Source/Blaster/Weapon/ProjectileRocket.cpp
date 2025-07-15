// Fill out your copyright notice in the Description page of Project Settings.
/// 火箭弹

#include "ProjectileRocket.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraComponent.h"
#include "Sound/SoundCue.h"
#include "Components/BoxComponent.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "RocketMovementComponent.h"

AProjectileRocket::AProjectileRocket()
{
	//创建网格对象
    ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Rocket Mesh"));
	//将该组件附加到另一个组件上
    ProjectileMesh->SetupAttachment(RootComponent);
	//设置碰撞
    ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    //创建组件
    RocketMovementComponent = CreateDefaultSubobject<URocketMovementComponent>(TEXT("RocketMovementComponent"));
    //让子弹保持其旋转与速度一致 （每帧更新其旋转 以匹配速度的方向）
    RocketMovementComponent->bRotationFollowsVelocity = true;
    //设置为复制
    RocketMovementComponent->SetIsReplicated(true);
}

void AProjectileRocket::BeginPlay()
{
    Super::BeginPlay();
    //在服务器上进行子弹的碰撞检测会造成伤害（Projectile中的Super::BeginPlay()的CollisionBox->OnComponentHit.AddDynamic也进行了OnHit的绑定，此类OnHit中对伤害造成时判断了是否为服务器）
    //客户端上也通过AddDynamic绑定OnHit处理子弹的表现子弹的隐藏 和子弹的拖尾特效销毁（消失）
    if (!HasAuthority())
    {
        CollisionBox->OnComponentHit.AddDynamic(this, &AProjectileRocket::OnHit);
    }

    //if (TrailSystem)
    //{
    //    //给物体上附加Niagara组件
    //    TrailSystemComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
    //        TrailSystem, //Niagara系统（特效源）
    //        GetRootComponent(),//附加到哪个组件
    //        FName(),//附加到的骨骼或者插槽名（这里传空名称，因为不需要附加到指定的位置）
    //        GetActorLocation(), //位置
    //        GetActorRotation(), //旋转
    //        EAttachLocation::KeepWorldPosition, //位置模式，以便附加的组件保持相同的世界变换。
    //        false//是否自动销毁（这里我们不通过接口自动销毁特效，而是通过倒计时控制
    //    );
    //}
    SpawnTrailSystem();

    if (ProjectileLoop && LoopingSoundAttenuation)
    {
        //子弹飞行时的音效
        ProjectileLoopComponent = UGameplayStatics::SpawnSoundAttached(
            ProjectileLoop, //音效源
            GetRootComponent(),//附加到哪个组件
            FName(),//附加到的骨骼或者插槽名（这里传空名称，因为不需要附加到指定的位置）
            GetActorLocation(),//位置
            EAttachLocation::KeepWorldPosition, //位置模式，以便附加的组件保持相同的世界变换。
            false,//是否销毁时自动停止播放（这里我们不通过接口自动销毁特效，而是通过倒计时控制
            1.f,//音量大小系数
            1.f,//音高大小系数
            0.f, //开始时间
            LoopingSoundAttenuation, //音效衰减
            (USoundConcurrency*)nullptr,//音效并发
            false //是否自动销毁
        );
    }
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
    //如果玩家发射出去的子弹击中的是玩家自己，则忽略，销毁在
    if (OtherActor == GetOwner())
    {
        return;
    }
    //碰撞时造成爆炸伤害
    ExplodeDamage();

	///*Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit); //去掉父类调用，不然会导致Niagara自动销毁。但这样会导致不会触发父类AProjectile中的OnHit并触发Destroy函数，因此需要补充下面的代码 */ 

    StartDestroyTimer();

    if (ImpactParticles)
    {
        UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, GetActorTransform());
    }
    if (ImpactSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
    }
    if (ProjectileMesh)
    {
        ProjectileMesh->SetVisibility(false); //碰撞后隐藏子弹网格体
    }
    if (CollisionBox)
    {
        CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);//碰撞后设置子弹碰撞体不检测碰撞
    }
    if (TrailSystemComponent && TrailSystemComponent->GetSystemInstance())
    {
        TrailSystemComponent->GetSystemInstance()->Deactivate();//设置拖尾特效不再产生粒子
    }
    if (ProjectileLoopComponent && ProjectileLoopComponent->IsPlaying())
    {
        ProjectileLoopComponent->Stop();//子弹飞行声音停止播放
    }
}

void AProjectileRocket::Destroyed()
{

}