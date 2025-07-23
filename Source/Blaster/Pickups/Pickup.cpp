// Fill out your copyright notice in the Description page of Project Settings.


#include "Pickup.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Components/SphereComponent.h"


APickup::APickup()
{
	PrimaryActorTick.bCanEverTick = true;
	//bReplicates ������֪��Ϸ��ActorӦ���ơ�ActorĬ�Ͻ��������������Ļ����ϡ�
	//�� bReplicates ��Ϊ True��ֻҪActor��Ȩ�����������ڷ������ϣ��ͻ᳢�Խ���Actor���Ƶ����������ӵĿͻ��ˡ�
	bReplicates = true;
	//���������������һ��������Ϊ�������������������������������Ϊ�����
	//������������USceneComponent�������Ϊ�����������ڸ������ͼ�����ק�������ڵĻ�����ֱ�Ӹ����ڵ��棬��������������һ�£���Ҫ����λ��
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	//����������ײ��
	OverlapSphere = CreateDefaultSubobject<USphereComponent>(TEXT("OverlapSphere"));
	//���ø����Ϊ�����
	OverlapSphere->SetupAttachment(RootComponent);
	//�������δ�С
	OverlapSphere->SetSphereRadius(150.f);
	//������ײ״̬��������ֻ��ѯ
	OverlapSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	//����������ײ
	OverlapSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	//���ǿ�����ҽ�����ײ���
	OverlapSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
	//��������
	PickupMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PickupMesh"));
	//���������������
	PickupMesh->SetupAttachment(OverlapSphere);
	//�ر���ײ
	PickupMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void APickup::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		//����ص�����
		OverlapSphere->OnComponentBeginOverlap.AddDynamic(this, &APickup::OnSphereOverlap);
	}
}

/// <summary>
/// ��ײ���ص������ص����ú���Ӧ��ֻ�ڷ���˵��ã���
/// </summary>
void APickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{

}

void APickup::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void APickup::Destroyed()
{
	Super::Destroyed();

	if (PickupSound)
	{
		//���ż�����Ч
		UGameplayStatics::PlaySoundAtLocation(
			this,
			PickupSound,
			GetActorLocation()
		);
	}
}