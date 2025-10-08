// Fill out your copyright notice in the Description page of Project Settings.


#include "Flag.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"

AFlag::AFlag()
{
	//�������ľ�̬����
	FlagMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FlagMesh"));
	//���ø����
	SetRootComponent(FlagMesh);
	//����ײ���������ӣ��󶨣�������������
	GetAreaSphere()->SetupAttachment(FlagMesh);
	//��ʰȡ�ؼ�������ӣ��󶨣�������������
	GetPickupWidget()->SetupAttachment(FlagMesh);
	//������ײ
	FlagMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	FlagMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}