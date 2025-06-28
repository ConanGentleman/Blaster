// Fill out your copyright notice in the Description page of Project Settings.


#include "HitScanWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Kismet/GameplayStatics.h"

void AHitScanWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn == nullptr) return;
	AController* InstigatorController = OwnerPawn->GetController();

	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash"); //武器子弹发射口
	if (MuzzleFlashSocket && InstigatorController)
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
			if (FireHit.bBlockingHit) //如果命中了
			{
				ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(FireHit.GetActor());
				if (BlasterCharacter)
				{
					if (HasAuthority()) // 命中直接造成伤害
					{
						UGameplayStatics::ApplyDamage(
							BlasterCharacter,
							Damage,
							InstigatorController,
							this,
							UDamageType::StaticClass()
						);
					}
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
			}
		}
	}
}