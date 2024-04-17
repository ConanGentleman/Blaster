// Fill out your copyright notice in the Description page of Project Settings.


#include "Casing.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

// Sets default values
ACasing::ACasing()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;//不需要每帧运行
	//创建组件
	CasingMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CasingMesh"));
	//设置根组件
	SetRootComponent(CasingMesh);
	//设置弹壳不与相机发生碰撞
	CasingMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	//启用弹壳受物理影响
	CasingMesh->SetSimulatePhysics(true);
	//启用弹壳受重力影响
	CasingMesh->SetEnableGravity(true);
	//确保模拟生成碰撞(hit)事件是被启用的-对应到蓝图里就是Simulation Generates Hit Event。不然碰撞不会触发，OnHit不会被调用
	//因为是启用了物理模拟SetSimulatePhysics，所以需要开启这个才会触发碰撞检测。如果不启用物理模拟应该就不用管这个（我猜）
	CasingMesh->SetNotifyRigidBodyCollision(true);
	ShellEjectionImpulse = 10.f;

}

// Called when the game starts or when spawned
void ACasing::BeginPlay()
{
	Super::BeginPlay();
	
	//注册弹壳碰撞函数
	CasingMesh->OnComponentHit.AddDynamic(this, &ACasing::OnHit);
	//给予某个方向上力.这里是actor正前方的力
	CasingMesh->AddImpulse(GetActorForwardVector() * ShellEjectionImpulse);
}
/// <summary>
/// 弹壳碰撞函数回调（主要是弹壳落地的碰撞
/// </summary>
/// https://dev.epicgames.com/documentation/zh-cn/unreal-engine/events-in-unreal-engine
/// <param name="HitComp">进行碰撞的组件（这里就是CollisionBox）</param>
/// <param name="OtherActor">参与碰撞的其他 Actor</param>
/// <param name="OtherComp">被击中的另一个组件</param>
/// <param name="NormalImpulse">Actor 碰撞的力/param>
/// <param name="Hit">击中结果</param>
void ACasing::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (ShellSound)
	{
		//播放落地音效
		UGameplayStatics::PlaySoundAtLocation(this, ShellSound, GetActorLocation());
	}
	//落地立即摧毁，可选方案：落地后几秒再销毁。
	//销毁actor会同步到服务器及所有的客户端上
	Destroy();
}
//
//void ACasing::Tick(float DeltaTime)
//{
//	Super::Tick(DeltaTime);
//
//}

