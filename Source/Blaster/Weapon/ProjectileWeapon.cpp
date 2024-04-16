// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Projectile.h"

/// <summary>
/// 武器开火生成子弹
/// </summary>
/// <param name="HitTarget">已通过射线检测（或其他方法）得到的命中目标的位置</param>
void AProjectileWeapon::Fire(const FVector& HitTarget) {
	Super::Fire(HitTarget);

	if (!HasAuthority()) return;//保证仅允许由服务器来生成子弹

	//可以理解为使用子弹的所有者的所有者
	APawn* InstigatorPawn = Cast<APawn>(GetOwner());
	//获取武器网格上名为MuzzleFlash的插槽（也就是枪口处）
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));
	if (MuzzleFlashSocket)
	{
		//获取枪口插槽的变换信息。参数：所属的网格
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		// From muzzle flash socket to hit location from TraceUnderCrosshairs
		//从枪口插槽到准星射线检测的命中位置
		FVector ToTarget = HitTarget - SocketTransform.GetLocation();
		//子弹的方向
		FRotator TargetRotation = ToTarget.Rotation();
		if (ProjectileClass && InstigatorPawn)
		{
			//SpawnActor传参，Owner和Instigator含义。 举个例子：玩家在坦克中开炮，命中了敌人。这里炮弹的Owner是坦克，Instigator是玩家。
			//生成Actor的参数
			FActorSpawnParameters SpawnParams;
			//设置生成的子弹Actor的所有者 也就是武器
			SpawnParams.Owner = GetOwner();
			//可以理解为使用子弹的所有者的所有者
			SpawnParams.Instigator = InstigatorPawn;
			UWorld* World = GetWorld();
			if (World)
			{
				//基于子弹类动态创建Actor
				World->SpawnActor<AProjectile>(
					ProjectileClass,
					SocketTransform.GetLocation(), //子弹生成的位置
					TargetRotation, //子弹生成的方向
					SpawnParams
					);
			}
		}
	}
}