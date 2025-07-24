// Fill out your copyright notice in the Description page of Project Settings.


#include "Pickup.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Components/SphereComponent.h"
#include "Blaster/Weapon/WeaponTypes.h"

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
	//ƫ��һ��λ�ã���Ϊ���ڵ��ǳ����������ֱ�Ӹ����ڵ��棬�ӵ���λ�þ���Ҫ����һ�£�
	OverlapSphere->AddLocalOffset(FVector(0.f, 0.f, 85.f));
	//��������
	PickupMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PickupMesh"));
	//���������������
	PickupMesh->SetupAttachment(OverlapSphere);
	//�ر���ײ
	PickupMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	//����һ������Ԥ���壩��С
	PickupMesh->SetRelativeScale3D(FVector(5.f, 5.f, 5.f));
	//�����Զ�����ȣ����ú�������ɼ�����������ȵ��������ʵ��������ô����BlasterMap��ͼ�����е�PostProcessVolume�����˺�����ʣ�ʹ�ó����еĿɼ�����������������
	PickupMesh->SetRenderCustomDepth(true);
	//������������Ϊ��ɫ 
	PickupMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_PURPLE);
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

	if (PickupMesh)
	{
		//ÿ֡��ת
		PickupMesh->AddWorldRotation(FRotator(0.f, BaseTurnRate * DeltaTime, 0.f));
	}
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