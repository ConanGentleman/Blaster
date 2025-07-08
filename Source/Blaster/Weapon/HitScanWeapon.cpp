// Fill out your copyright notice in the Description page of Project Settings.


#include "HitScanWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "Kismet/KismetMathLibrary.h"
#include "WeaponTypes.h"

#include "DrawDebugHelpers.h"

void AHitScanWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn == nullptr) return;
	AController* InstigatorController = OwnerPawn->GetController();

	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash"); //武器子弹发射口
	if (MuzzleFlashSocket)
	{
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		FVector Start = SocketTransform.GetLocation();

		FHitResult FireHit;
		WeaponTraceHit(Start, HitTarget, FireHit);

		ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(FireHit.GetActor());
		if (BlasterCharacter && HasAuthority() && InstigatorController)
		{
			UGameplayStatics::ApplyDamage(// 命中直接造成伤害
				BlasterCharacter,
				Damage,
				InstigatorController,
				this,
				UDamageType::StaticClass()
			);
		}
		if (ImpactParticles)  //播放击中动效
		{
			UGameplayStatics::SpawnEmitterAtLocation(
				GetWorld(),
				ImpactParticles,
				FireHit.ImpactPoint,
				FireHit.ImpactNormal.Rotation()
			);
		}
		if (HitSound) //击中音效
		{
			UGameplayStatics::PlaySoundAtLocation(
				this,
				HitSound,
				FireHit.ImpactPoint
			);
		}
		if (MuzzleFlash) //开火动效(有些武器没有自带动画所以这里通过以特效的形式来增加动效感)
		{
			UGameplayStatics::SpawnEmitterAtLocation(
				GetWorld(),
				MuzzleFlash,
				SocketTransform
			);
		}
		if (FireSound) //开火音效
		{
			UGameplayStatics::PlaySoundAtLocation(
				this,
				FireSound,
				GetActorLocation()
			);
		}
	}
}

/// <summary>
/// 武器击中检测
/// </summary>
/// <param name="TraceStart"></param>
/// <param name="HitTarget"></param>
/// <param name="OutHit"></param>
void AHitScanWeapon::WeaponTraceHit(const FVector& TraceStart, const FVector& HitTarget, FHitResult& OutHit)
{
	UWorld* World = GetWorld();
	if (World)
	{
		//是否是霰弹枪 是的话通过霰弹形式求检测结束点（随机生成），否则直接以直线计算结束点（乘以1.25是担心子弹正好在网格表面上而判断没有受到伤害）
		FVector End = bUseScatter ? TraceEndWithScatter(TraceStart, HitTarget) : TraceStart + (HitTarget - TraceStart) * 1.25f;

		World->LineTraceSingleByChannel(  //沿给定的线执行碰撞追踪，并返回首个阻挡命中，只返回对特定追踪 通道 响应的对象
			OutHit, //结果
			TraceStart,//射线起始点
			End, // 射线终点
			ECollisionChannel::ECC_Visibility //检测通道
		);
		FVector BeamEnd = End;
		if (OutHit.bBlockingHit) //如果命中了
		{
			BeamEnd = OutHit.ImpactPoint; //光束终点（检测终点）
		}
		if (BeamParticles)
		{
			UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation( //播放子弹路径特效
				World,
				BeamParticles,
				TraceStart, 
				FRotator::ZeroRotator,
				true //自动销毁
			);
			if (Beam)
			{
				Beam->SetVectorParameter(FName("Target"), BeamEnd); //设置特效终点
			}
		}
	}
}

/// <summary>
/// 带分散点的追踪终点（霰弹枪的多个子弹终点计算，随机返回霰弹枪的某颗字段的终点）
/// </summary>
/// <param name="TraceStart">开始位置</param>
/// <param name="HitTarget">命中目标位置</param>
/// <returns></returns>
FVector AHitScanWeapon::TraceEndWithScatter(const FVector& TraceStart, const FVector& HitTarget)
{
	FVector ToTargetNormalized = (HitTarget - TraceStart).GetSafeNormal(); //归一化方向向量
	FVector SphereCenter = TraceStart + ToTargetNormalized * DistanceToSphere; //求出球心位置
	FVector RandVec = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, SphereRadius); //随机出一个半径为SphereRadius的球中的点
	FVector EndLoc = SphereCenter + RandVec; // 子弹散射的终点位置（注意：并不是子弹的最终检测击中位置）
	FVector ToEndLoc = EndLoc - TraceStart; // 子弹散射的方向

	/*
	DrawDebugSphere(GetWorld(), SphereCenter, SphereRadius, 12, FColor::Red, true); //调试绘制球体（world,球心，半径，网格数，颜色，绘制持久线一直保留）
	DrawDebugSphere(GetWorld(), EndLoc, 4.f, 12, FColor::Orange, true); //调试绘制球体
	DrawDebugLine(
		GetWorld(),
		TraceStart,
		FVector(TraceStart + ToEndLoc * TRACE_LENGTH / ToEndLoc.Size()),
		FColor::Cyan,
		true); //调试绘制线*/

	//子弹检测击中的最远位置 = 子弹发出的位置 + 子弹散射的方向 * 检测距离 / 子弹散射的长度   
	//。。注意除以长度主要就是为了归一化，可以理解为 方向/长度 * 检测距离 （个人理解）
	//但是官方给出的原因是 ToEndLoc * TRACE_LENGTH可能会超出double 所以除以了ToEndLoc.Size()
	return FVector(TraceStart + ToEndLoc * TRACE_LENGTH / ToEndLoc.Size()); 
}