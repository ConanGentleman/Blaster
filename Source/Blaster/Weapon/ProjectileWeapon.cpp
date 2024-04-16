// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Projectile.h"

/// <summary>
/// ��������
/// </summary>
/// <param name="HitTarget">��ͨ�����߼�⣨�������������õ�������Ŀ���λ��</param>
void AProjectileWeapon::Fire(const FVector& HitTarget) {
	Super::Fire(HitTarget);
	APawn* InstigatorPawn = Cast<APawn>(GetOwner());
	//��ȡ������������ΪMuzzleFlash�Ĳ�ۣ�Ҳ����ǹ�ڴ���
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));
	if (MuzzleFlashSocket)
	{
		//��ȡǹ�ڲ�۵ı任��Ϣ������������������
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		// From muzzle flash socket to hit location from TraceUnderCrosshairs
		//��ǹ�ڲ�۵�׼�����߼�������λ��
		FVector ToTarget = HitTarget - SocketTransform.GetLocation();
		//�ӵ��ķ���
		FRotator TargetRotation = ToTarget.Rotation();
		if (ProjectileClass && InstigatorPawn)
		{
			//SpawnActor���Σ�Owner��Instigator���塣 �ٸ����ӣ������̹���п��ڣ������˵��ˡ������ڵ���Owner��̹�ˣ�Instigator����ҡ�
			//����Actor�Ĳ���
			FActorSpawnParameters SpawnParams;
			//�������ɵ��ӵ�Actor�������� Ҳ��������
			SpawnParams.Owner = GetOwner();
			//�������Ϊʹ���ӵ��������ߵ�������
			SpawnParams.Instigator = InstigatorPawn;
			UWorld* World = GetWorld();
			if (World)
			{
				//�����ӵ��ද̬����Actor
				World->SpawnActor<AProjectile>(
					ProjectileClass,
					SocketTransform.GetLocation(), //�ӵ����ɵ�λ��
					TargetRotation, //�ӵ����ɵķ���
					SpawnParams
					);
			}
		}
	}
}