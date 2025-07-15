// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileGrenade.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

AProjectileGrenade::AProjectileGrenade()
{
	//�����������
	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Grenade Mesh"));
	//����������ӵ���һ�������
	ProjectileMesh->SetupAttachment(RootComponent);
	//������ײ
	ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	//�������
	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	//���ӵ���������ת���ٶ�һ�� ��ÿ֡��������ת ��ƥ���ٶȵķ���
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	//����Ϊ����
	ProjectileMovementComponent->SetIsReplicated(true);
	//�Ƿ���Ե��䣨���ǽӴ�˲����
	ProjectileMovementComponent->bShouldBounce = true;
}

void AProjectileGrenade::BeginPlay()
{
	AActor::BeginPlay();
	//������β
	SpawnTrailSystem();
	//���ٵ���ʱ��ʼ
	StartDestroyTimer();
	//ע����ײ��������¼� 
	ProjectileMovementComponent->OnProjectileBounce.AddDynamic(this, &AProjectileGrenade::OnBounce);
}

/// <summary>
/// �ӵ���ײ��������¼��ص�
/// </summary>
/// <param name="ImpactResult"></param>
/// <param name="ImpactVelocity"></param>
void AProjectileGrenade::OnBounce(const FHitResult& ImpactResult, const FVector& ImpactVelocity)
{
	if (BounceSound)
	{
		UGameplayStatics::PlaySoundAtLocation( //�ڵ���λ�ò�����Ч
			this,
			BounceSound,
			GetActorLocation()
		);
	}
}

void AProjectileGrenade::Destroyed()
{
	ExplodeDamage();//�ݻ�ʱ��ɱ�ը�˺�
	Super::Destroyed();
}