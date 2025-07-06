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
		FVector End = Start + (HitTarget - Start) * 1.25f; //乘以1.25是担心子弹正好在网格表面上而判断没有受到伤害

		FHitResult FireHit;
		UWorld* World = GetWorld();
		if (World)
		{
			World->LineTraceSingleByChannel( //沿给定的线执行碰撞追踪，并返回首个阻挡命中，只返回对特定追踪 通道 响应的对象
				FireHit, //结果
				Start, //射线起始点
				End,  // 涉嫌终点
				ECollisionChannel::ECC_Visibility //检测通道
			);
			FVector BeamEnd = End;
			if (FireHit.bBlockingHit) //如果命中了
			{
				BeamEnd = FireHit.ImpactPoint;
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
				if (ImpactParticles)  //播放动效
				{
					UGameplayStatics::SpawnEmitterAtLocation(
						World,
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
			}
			if (BeamParticles)
			{
				UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(
					World,
					BeamParticles,
					SocketTransform
				);
				if (Beam)
				{
					Beam->SetVectorParameter(FName("Target"), BeamEnd);
				}
			}
		}
		if (MuzzleFlash) //开火动效(有些武器没有自带动画所以这里通过以特效的形式来增加动效感)
		{
			UGameplayStatics::SpawnEmitterAtLocation(
				World,
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

	DrawDebugSphere(GetWorld(), SphereCenter, SphereRadius, 12, FColor::Red, true); //调试绘制球体（world,球心，半径，网格数，颜色，绘制持久线一直保留）
	DrawDebugSphere(GetWorld(), EndLoc, 4.f, 12, FColor::Orange, true); //调试绘制球体
	DrawDebugLine(
		GetWorld(),
		TraceStart,
		FVector(TraceStart + ToEndLoc * TRACE_LENGTH / ToEndLoc.Size()),
		FColor::Cyan,
		true); //调试绘制线

	//子弹检测击中的最远位置 = 子弹发出的位置 + 子弹散射的方向 * 检测距离 / 子弹散射的长度   
	//。。注意除以长度主要就是为了归一化，可以理解为 方向/长度 * 检测距离 （个人理解）
	//但是官方给出的原因是 ToEndLoc * TRACE_LENGTH可能会超出double 所以除以了ToEndLoc.Size()
	return FVector(TraceStart + ToEndLoc * TRACE_LENGTH / ToEndLoc.Size()); 
}