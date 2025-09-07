// Fill out your copyright notice in the Description page of Project Settings.


#include "HitScanWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "WeaponTypes.h"
#include "Blaster/BlasterComponents/LagCompensationComponent.h"

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
		if (BlasterCharacter && InstigatorController)
		{
			if (HasAuthority() && !bUseServerSideRewind) { //如果是客户端，则不需要使用延迟补偿算法
				UGameplayStatics::ApplyDamage(// 命中直接造成伤害
					BlasterCharacter,
					Damage,
					InstigatorController,
					this,
					UDamageType::StaticClass()
				);
			}
			if (!HasAuthority() && bUseServerSideRewind) //如果客户端，则使用延迟补偿算法
			{
				BlasterOwnerCharacter = BlasterOwnerCharacter == nullptr ? Cast<ABlasterCharacter>(OwnerPawn) : BlasterOwnerCharacter;
				BlasterOwnerController = BlasterOwnerController == nullptr ? Cast<ABlasterPlayerController>(InstigatorController) : BlasterOwnerController;
				if (BlasterOwnerController && BlasterOwnerCharacter && BlasterOwnerCharacter->GetLagCompensation() && BlasterOwnerCharacter->IsLocallyControlled())
				{
					//调用 服务器伤害/得分请求RPC（通过倒带算法判定击中玩家后，进行伤害判定和得分结算
					BlasterOwnerCharacter->GetLagCompensation()->ServerScoreRequest(
						BlasterCharacter,//击中的角色
						Start,
						HitTarget,
						BlasterOwnerController->GetServerTime() - BlasterOwnerController->SingleTripTime,//击中时间等于服务器时间-从客户端（服务端）传到服务器（客户端）的时间
						this
					);
				}
			}
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
		
		//直接以直线计算结束点（乘以1.25是担心子弹正好在网格表面上而判断没有受到伤害）
		FVector End = TraceStart + (HitTarget - TraceStart) * 1.25f;

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

		DrawDebugSphere(GetWorld(), BeamEnd, 16.f, 12, FColor::Orange, true);

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
