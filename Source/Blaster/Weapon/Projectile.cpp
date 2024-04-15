// Fill out your copyright notice in the Description page of Project Settings.


#include "Projectile.h"
#include "Components/BoxComponent.h"

// Sets default values
AProjectile::AProjectile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	//��ʼ����ײ��
	CollisionBox = CreateDefaultSubobject<UBoxComponent>("CollisionBox");
	//���ø����
	SetRootComponent(CollisionBox);
	//���Ĵ˶����ƶ�ʱʹ�õ���ײͨ���������ӵ��ܹ��ڿ��з��У���������ΪECC_WorldDynamic
	CollisionBox->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	//������ײ
	CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	//�Ⱥ�������ͨ������ײ���ٵ�������Ҫ��ײ��ͨ��
	CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	//�����κοɼ��Ժ����羲ֹ���壨��ǽ���ذ壩Ϊ����ײͨ��
	CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);
}

void AProjectile::BeginPlay()
{
	Super::BeginPlay();
	
}
void AProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

