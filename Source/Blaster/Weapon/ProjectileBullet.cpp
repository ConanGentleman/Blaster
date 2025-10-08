// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileBullet.h"
#include "Kismet/GameplayStatics.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Blaster/BlasterComponents/LagCompensationComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"

AProjectileBullet::AProjectileBullet()
{
	//创建组件
	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	//让子弹保持其旋转与速度一致 （每帧更新其旋转 以匹配速度的方向）
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	//设置为复制
	ProjectileMovementComponent->SetIsReplicated(true);
	ProjectileMovementComponent->InitialSpeed = InitialSpeed;
	ProjectileMovementComponent->MaxSpeed = InitialSpeed;
}

///属性值在编辑器中被更改后执行该逻辑（这里写的目的是因为发现在编辑器中对InitialSpeed修改赋值后，ProjectileMovementComponent组件的InitialSpeed和MaxSpeed并没有同步修改
///所以这里类似于进行一个同步修改逻辑
#if WITH_EDITOR
void AProjectileBullet::PostEditChangeProperty(FPropertyChangedEvent& Event)
{
	Super::PostEditChangeProperty(Event);

	FName PropertyName = Event.Property != nullptr ? Event.Property->GetFName() : NAME_None;
	if (PropertyName == GET_MEMBER_NAME_CHECKED(AProjectileBullet, InitialSpeed))//如果属性名为InitialSpeed的值变化，则进行赋值
	{
		if (ProjectileMovementComponent)
		{
			ProjectileMovementComponent->InitialSpeed = InitialSpeed;
			ProjectileMovementComponent->MaxSpeed = InitialSpeed;
		}
	}
}
#endif


/// <summary>
/// 重写子弹父类中的子弹碰撞函数。父类中的子弹碰撞函数已经完成了子弹销毁等功能。
/// 这里主要调用伤害函数。
/// </summary>
void AProjectileBullet::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	//获取子弹的所有者（ 在combatcomponent.cpp中，将装备武器的所有者设置为了角色）
	ABlasterCharacter* OwnerCharacter = Cast<ABlasterCharacter>(GetOwner());
	if (OwnerCharacter)
	{
		//获取控制器
		ABlasterPlayerController* OwnerController = Cast<ABlasterPlayerController>(OwnerCharacter->Controller);
		if (OwnerController)
		{
			UGameplayStatics::ApplyDamage(OtherActor, Damage, OwnerController, this, UDamageType::StaticClass());
			//如果是服务器，且未开启倒带算法，则命中时施加伤害
			if (OwnerCharacter->HasAuthority() && !bUseServerSideRewind)
			{
				//命中时施加伤害，使用ue自带的伤害函数。参数：受伤害的actor，伤害值，造成此伤害的控制器（例如射击武器的玩家），实际造成伤害的actor，描述所造成上海的类
				const float DamageToCause = Hit.BoneName.ToString() == FString("head") ? HeadShotDamage : Damage;

				UGameplayStatics::ApplyDamage(OtherActor, DamageToCause, OwnerController, this, UDamageType::StaticClass());
				Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);
				return;
			}
			ABlasterCharacter* HitCharacter = Cast<ABlasterCharacter>(OtherActor);
			//如果开启了延迟补偿算法，并且子弹所有者是本地控制的
			if (bUseServerSideRewind && OwnerCharacter->GetLagCompensation() && OwnerCharacter->IsLocallyControlled() && HitCharacter)
			{
				//则向服务器发送分数请求
				OwnerCharacter->GetLagCompensation()->ProjectileServerScoreRequest(
					HitCharacter,
					TraceStart,
					InitialVelocity,
					OwnerController->GetServerTime() - OwnerController->SingleTripTime //减去一半的时间
				);
			}
		}
	}
	//由于父类中是对子弹进行销毁，因此将父类的子弹碰撞函数调用放在最后，保证正常运行。
	Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);
}

void AProjectileBullet::BeginPlay()
{
	Super::BeginPlay();


	//构建预测子弹轨迹参数
	/*FPredictProjectilePathParams PathParams;
	PathParams.bTraceWithChannel = true;//是否使用特定的碰撞通道
	PathParams.bTraceWithCollision = true;//是否开启碰撞检测
	PathParams.DrawDebugTime = 5.f;//绘制调试路径保留时长
	PathParams.DrawDebugType = EDrawDebugTrace::ForDuration;//绘制调试类型
	PathParams.LaunchVelocity = GetActorForwardVector() * InitialSpeed;//发射初始速度速度
	PathParams.MaxSimTime = 4.f;//空中飞行时间
	PathParams.ProjectileRadius = 5.f;//子弹半径
	PathParams.SimFrequency = 30.f;//模拟精度：SimFrequency 越高，单位时间内模拟的点数就越多。意味着能更准确地捕捉到抛射物复杂飞行轨迹的细节，能更精确地检测到与较小或较薄物体的碰撞
	PathParams.StartLocation = GetActorLocation();//起始位置
	PathParams.TraceChannel = ECollisionChannel::ECC_Visibility;//碰撞检测通道
	PathParams.ActorsToIgnore.Add(this);//忽略碰撞角色

	FPredictProjectilePathResult PathResult;
	//预测子弹轨迹（参数：世界，路径参数，路劲结果
	UGameplayStatics::PredictProjectilePath(this, PathParams, PathResult);*/
}