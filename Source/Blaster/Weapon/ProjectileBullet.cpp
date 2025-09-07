// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileBullet.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "GameFramework/ProjectileMovementComponent.h"

AProjectileBullet::AProjectileBullet()
{
	//�������
	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	//���ӵ���������ת���ٶ�һ�� ��ÿ֡��������ת ��ƥ���ٶȵķ���
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	//����Ϊ����
	ProjectileMovementComponent->SetIsReplicated(true);
	ProjectileMovementComponent->InitialSpeed = InitialSpeed;
	ProjectileMovementComponent->MaxSpeed = InitialSpeed;
}


/// <summary>
/// ��д�ӵ������е��ӵ���ײ�����������е��ӵ���ײ�����Ѿ�������ӵ����ٵȹ��ܡ�
/// ������Ҫ�����˺�������
/// </summary>
void AProjectileBullet::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	//��ȡ�ӵ��������ߣ� ��combatcomponent.cpp�У���װ������������������Ϊ�˽�ɫ��
	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (OwnerCharacter)
	{
		//��ȡ������
		AController* OwnerController = OwnerCharacter->Controller;
		if (OwnerController)
		{
			//����ʱʩ���˺���ʹ��ue�Դ����˺����������������˺���actor���˺�ֵ����ɴ��˺��Ŀ����������������������ң���ʵ������˺���actor������������Ϻ�����
			UGameplayStatics::ApplyDamage(OtherActor, Damage, OwnerController, this, UDamageType::StaticClass());
		}
	}
	//���ڸ������Ƕ��ӵ��������٣���˽�������ӵ���ײ�������÷�����󣬱�֤�������С�
	Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);
}

void AProjectileBullet::BeginPlay()
{
	Super::BeginPlay();

	FPredictProjectilePathParams PathParams;
	PathParams.bTraceWithChannel = true;
	PathParams.bTraceWithCollision = true;
	PathParams.DrawDebugTime = 5.f;
	PathParams.DrawDebugType = EDrawDebugTrace::ForDuration;
	PathParams.LaunchVelocity = GetActorForwardVector() * InitialSpeed;
	PathParams.MaxSimTime = 4.f;
	PathParams.ProjectileRadius = 5.f;
	PathParams.SimFrequency = 30.f;
	PathParams.StartLocation = GetActorLocation();
	PathParams.TraceChannel = ECollisionChannel::ECC_Visibility;
	PathParams.ActorsToIgnore.Add(this);

	FPredictProjectilePathResult PathResult;

	UGameplayStatics::PredictProjectilePath(this, PathParams, PathResult);
}