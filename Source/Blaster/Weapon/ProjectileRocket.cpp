// Fill out your copyright notice in the Description page of Project Settings.
/// �����

#include "ProjectileRocket.h"
#include "Kismet/GameplayStatics.h"

AProjectileRocket::AProjectileRocket()
{
	//��������
	RocketMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Rocket Mesh"));
	//����������ӵ���һ�������
	RocketMesh->SetupAttachment(RootComponent);
	//������ײ
	RocketMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

/// <summary>
/// �ӵ����к���
/// </summary>
/// <param name="HitComp"></param>
/// <param name="OtherActor"></param>
/// <param name="OtherComp"></param>
/// <param name="NormalImpulse"></param>
/// <param name="Hit"></param>
void AProjectileRocket::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// ��ȡ���Ͳ���ͷ��ߣ����ͷ�������OrijectileWeapon��Fire�����б����õģ�
	APawn* FiringPawn = GetInstigator(); 
	if (FiringPawn)
	{
		AController* FiringController = FiringPawn->GetController();
		if (FiringController)
		{
			UGameplayStatics::ApplyRadialDamageWithFalloff( //���Բ�α�ը�˺�
				this, // World context object
				Damage, // BaseDamage �����˺��������һ����Ա����
				10.f, // MinimumDamage ��С�˺�
				GetActorLocation(), // Origin ��ըԭ��
				200.f, // DamageInnerRadius �˺��ھ�
				500.f, // DamageOuterRadius �˺��⾶�����⾶���յ���С�˺�
				1.f, // DamageFalloff �˺�˥��������
				UDamageType::StaticClass(), // DamageTypeClass �Ϻ�����
				TArray<AActor*>(), // IgnoreActors �����˺������
				this, // DamageCauser ����˺���ԭ��
				FiringController // InstigatorController �ͷ��߿�����
			);
		}
	}
	Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);
}