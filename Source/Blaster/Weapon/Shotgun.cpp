// Fill out your copyright notice in the Description page of Project Settings.


#include "Shotgun.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "Kismet/KismetMathLibrary.h"

void AShotgun::Fire(const FVector& HitTarget)
{
	/// 调用爷类的开火方法，而不是父类的开火Super:Fire(HitTarget)
	AWeapon::Fire(HitTarget);
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn == nullptr) return;
	AController* InstigatorController = OwnerPawn->GetController();

	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if (MuzzleFlashSocket)
	{
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		FVector Start = SocketTransform.GetLocation();

		TMap<ABlasterCharacter*, uint32> HitMap;
		for (uint32 i = 0; i < NumberOfPellets; i++) //遍历每个霰弹
		{
			FHitResult FireHit;
			WeaponTraceHit(Start, HitTarget, FireHit); //检测击中结果

			ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(FireHit.GetActor());
			if (BlasterCharacter && HasAuthority() && InstigatorController)
			{
				if (HitMap.Contains(BlasterCharacter))
				{
					HitMap[BlasterCharacter]++;//增加玩家被子弹击中的数量+1
				}
				else
				{
					HitMap.Emplace(BlasterCharacter, 1); //原地构造键值，避免拷贝,记BlasterCharacter被一个子弹击中
				}
			}

			if (ImpactParticles) //播放击中动效
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
					FireHit.ImpactPoint,
					.5f, //音量
					FMath::FRandRange(-.5f, .5f) //音高随机增幅
				);
			}
		}

		for (auto HitPair : HitMap)
		{
			if (HitPair.Key && HasAuthority() && InstigatorController)//遍历子弹命中的玩家
			{
				UGameplayStatics::ApplyDamage( // 命中直接造成伤害
					HitPair.Key, //对应的BlasterCharacter
					Damage * HitPair.Value,
					InstigatorController,
					this,
					UDamageType::StaticClass()
				);
			}
		}
	}
}

/// <summary>
/// 生成子弹的随机发射方向和位置
/// </summary>
/// <param name="HitTarget"></param>
/// <param name="HitTargets"></param>
void AShotgun::ShotgunTraceEndWithScatter(const FVector& HitTarget, TArray<FVector>& HitTargets)
{
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if (MuzzleFlashSocket == nullptr) return;

	const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
	const FVector TraceStart = SocketTransform.GetLocation();

	const FVector ToTargetNormalized = (HitTarget - TraceStart).GetSafeNormal(); //归一化方向向量
	const FVector SphereCenter = TraceStart + ToTargetNormalized * DistanceToSphere;//求出球心位置

	for (uint32 i = 0; i < NumberOfPellets; i++)//实际上也可以调NumberOfPellets次父类的TraceEndWithScatter，但效率感觉不高
	{
		const FVector RandVec = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, SphereRadius); //随机出一个半径为SphereRadius的球中的点
		const FVector EndLoc = SphereCenter + RandVec;// 子弹散射的终点位置（注意：并不是子弹的最终检测击中位置）
		FVector ToEndLoc = EndLoc - TraceStart;// 子弹散射的方向

		//子弹检测击中的最远位置 = 子弹发出的位置 + 子弹散射的方向 * 检测距离 / 子弹散射的长度   
		//。。注意除以长度主要就是为了归一化，可以理解为 方向/长度 * 检测距离 （个人理解）
		//但是官方给出的原因是 ToEndLoc * TRACE_LENGTH可能会超出double 所以除以了ToEndLoc.Size()
		ToEndLoc = TraceStart + ToEndLoc * TRACE_LENGTH / ToEndLoc.Size();

		HitTargets.Add(ToEndLoc);
	}
}