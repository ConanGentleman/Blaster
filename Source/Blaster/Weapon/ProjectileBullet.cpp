// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileBullet.h"
#include "Kismet/GameplayStatics.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Blaster/BlasterComponents/LagCompensationComponent.h"
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

///����ֵ�ڱ༭���б����ĺ�ִ�и��߼�������д��Ŀ������Ϊ�����ڱ༭���ж�InitialSpeed�޸ĸ�ֵ��ProjectileMovementComponent�����InitialSpeed��MaxSpeed��û��ͬ���޸�
///�������������ڽ���һ��ͬ���޸��߼�
#if WITH_EDITOR
void AProjectileBullet::PostEditChangeProperty(FPropertyChangedEvent& Event)
{
	Super::PostEditChangeProperty(Event);

	FName PropertyName = Event.Property != nullptr ? Event.Property->GetFName() : NAME_None;
	if (PropertyName == GET_MEMBER_NAME_CHECKED(AProjectileBullet, InitialSpeed))//���������ΪInitialSpeed��ֵ�仯������и�ֵ
	{
		if (ProjectileMovementComponent)
		{
			ProjectileMovementComponent->InitialSpeed = InitialSpeed;
			ProjectileMovementComponent->MaxSpeed = InitialSpeed;
		}
	}
}
#endif


/// <summary>
/// ��д�ӵ������е��ӵ���ײ�����������е��ӵ���ײ�����Ѿ�������ӵ����ٵȹ��ܡ�
/// ������Ҫ�����˺�������
/// </summary>
void AProjectileBullet::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	//��ȡ�ӵ��������ߣ� ��combatcomponent.cpp�У���װ������������������Ϊ�˽�ɫ��
	ABlasterCharacter* OwnerCharacter = Cast<ABlasterCharacter>(GetOwner());
	if (OwnerCharacter)
	{
		//��ȡ������
		ABlasterPlayerController* OwnerController = Cast<ABlasterPlayerController>(OwnerCharacter->Controller);
		if (OwnerController)
		{
			UGameplayStatics::ApplyDamage(OtherActor, Damage, OwnerController, this, UDamageType::StaticClass());
			//����Ƿ���������δ���������㷨��������ʱʩ���˺�
			if (OwnerCharacter->HasAuthority() && !bUseServerSideRewind)
			{
				//����ʱʩ���˺���ʹ��ue�Դ����˺����������������˺���actor���˺�ֵ����ɴ��˺��Ŀ����������������������ң���ʵ������˺���actor������������Ϻ�����
				const float DamageToCause = Hit.BoneName.ToString() == FString("head") ? HeadShotDamage : Damage;

				UGameplayStatics::ApplyDamage(OtherActor, DamageToCause, OwnerController, this, UDamageType::StaticClass());
				Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);
				return;
			}
			ABlasterCharacter* HitCharacter = Cast<ABlasterCharacter>(OtherActor);
			//����������ӳٲ����㷨�������ӵ��������Ǳ��ؿ��Ƶ�
			if (bUseServerSideRewind && OwnerCharacter->GetLagCompensation() && OwnerCharacter->IsLocallyControlled() && HitCharacter)
			{
				//������������ͷ�������
				OwnerCharacter->GetLagCompensation()->ProjectileServerScoreRequest(
					HitCharacter,
					TraceStart,
					InitialVelocity,
					OwnerController->GetServerTime() - OwnerController->SingleTripTime //��ȥһ���ʱ��
				);
			}
		}
	}
	//���ڸ������Ƕ��ӵ��������٣���˽�������ӵ���ײ�������÷�����󣬱�֤�������С�
	Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);
}

void AProjectileBullet::BeginPlay()
{
	Super::BeginPlay();


	//����Ԥ���ӵ��켣����
	/*FPredictProjectilePathParams PathParams;
	PathParams.bTraceWithChannel = true;//�Ƿ�ʹ���ض�����ײͨ��
	PathParams.bTraceWithCollision = true;//�Ƿ�����ײ���
	PathParams.DrawDebugTime = 5.f;//���Ƶ���·������ʱ��
	PathParams.DrawDebugType = EDrawDebugTrace::ForDuration;//���Ƶ�������
	PathParams.LaunchVelocity = GetActorForwardVector() * InitialSpeed;//�����ʼ�ٶ��ٶ�
	PathParams.MaxSimTime = 4.f;//���з���ʱ��
	PathParams.ProjectileRadius = 5.f;//�ӵ��뾶
	PathParams.SimFrequency = 30.f;//ģ�⾫�ȣ�SimFrequency Խ�ߣ���λʱ����ģ��ĵ�����Խ�ࡣ��ζ���ܸ�׼ȷ�ز�׽�������︴�ӷ��й켣��ϸ�ڣ��ܸ���ȷ�ؼ�⵽���С��ϱ��������ײ
	PathParams.StartLocation = GetActorLocation();//��ʼλ��
	PathParams.TraceChannel = ECollisionChannel::ECC_Visibility;//��ײ���ͨ��
	PathParams.ActorsToIgnore.Add(this);//������ײ��ɫ

	FPredictProjectilePathResult PathResult;
	//Ԥ���ӵ��켣�����������磬·��������·�����
	UGameplayStatics::PredictProjectilePath(this, PathParams, PathResult);*/
}