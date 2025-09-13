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

	//if (!HasAuthority()) return;//保证仅允许由服务器来生成子弹

	//可以理解为使用子弹的所有者的所有者
	APawn* InstigatorPawn = Cast<APawn>(GetOwner());
	//获取武器网格上名为MuzzleFlash的插槽（也就是枪口处）
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));
	//if (MuzzleFlashSocket)
	UWorld* World = GetWorld();
	if (MuzzleFlashSocket && World)
	{
		//获取枪口插槽的变换信息。参数：所属的网格
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		// From muzzle flash socket to hit location from TraceUnderCrosshairs
		//从枪口插槽到准星射线检测的命中位置
		FVector ToTarget = HitTarget - SocketTransform.GetLocation();
		//子弹的方向
		FRotator TargetRotation = ToTarget.Rotation();
		//SpawnActor传参，Owner和Instigator含义。 举个例子：玩家在坦克中开炮，命中了敌人。这里炮弹的Owner是坦克，Instigator是玩家。
		//生成Actor的参数
		FActorSpawnParameters SpawnParams;
		//设置生成的子弹Actor的所有者 也就是武器
		SpawnParams.Owner = GetOwner();
		//可以理解为使用子弹的所有者的所有者
		SpawnParams.Instigator = InstigatorPawn;
		
		AProjectile* SpawnedProjectile = nullptr;
		if (bUseServerSideRewind) //是否使用服务器倒带算法
		{
			if (InstigatorPawn->HasAuthority()) // server 如果是服务器
			{
				if (InstigatorPawn->IsLocallyControlled()) // server, host - use replicated projectile 如果是服务器且为本地控制，则生成可以进行复制的子弹，子弹不使用倒带算法，这样就不会在服务器上发送服务器得分请求
				{
					SpawnedProjectile = World->SpawnActor<AProjectile>(ProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParams);
					SpawnedProjectile->bUseServerSideRewind = false;
					//产生复制子弹变量后直接将武器的伤害设置为子弹伤害
					SpawnedProjectile->Damage = Damage;
				}
				else // server, not locally controlled - spawn non-replicated projectile, no SSR  如果是服务器且不为本地控制，生成不可进行复制的子弹，且不适用倒带算法，这样就不会在服务器上发送服务器得分请求。（因为这是客户端请求调用的，客户端会在下面的判断分支，在自己本地生成一个子弹，而不是服务器通过复制通知客户端生成
				{
					SpawnedProjectile = World->SpawnActor<AProjectile>(ServerSideRewindProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParams);
					SpawnedProjectile->bUseServerSideRewind = false;
				}
			}
			else // client, using SSR
			{
				if (InstigatorPawn->IsLocallyControlled()) // client, locally controlled - spawn non-replicated projectile, use SSR 如果是客户端且本地控制，生成不可进行复制的子弹，但开启倒带算法
				{
					SpawnedProjectile = World->SpawnActor<AProjectile>(ServerSideRewindProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParams);
					SpawnedProjectile->bUseServerSideRewind = true;
					// 倒带算法需要的如下因此信息需要存储在子弹上面
					SpawnedProjectile->TraceStart = SocketTransform.GetLocation();
					SpawnedProjectile->InitialVelocity = SpawnedProjectile->GetActorForwardVector() * SpawnedProjectile->InitialSpeed;
					SpawnedProjectile->Damage = Damage;
				}
				else // client, not locally controlled - spawn non-replicated projectile, no SSR 如果是客户端且本地控制，生成不可进行复制的子弹，且不开启倒带算法
				{
					SpawnedProjectile = World->SpawnActor<AProjectile>(ServerSideRewindProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParams);
					SpawnedProjectile->bUseServerSideRewind = false;
				}
			}
		}
		else  // weapon not using SSR 不使用延迟补偿算法
		{
			if (InstigatorPawn->HasAuthority())
			{
				//基于子弹类动态创建Actor
				SpawnedProjectile = World->SpawnActor<AProjectile>(ProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParams);
				SpawnedProjectile->bUseServerSideRewind = false;
				SpawnedProjectile->Damage = Damage;
			}
		}
	}
}