// Fill out your copyright notice in the Description page of Project Settings.


#include "FlagZone.h"
#include "Components/SphereComponent.h"
#include "Blaster/Weapon/Flag.h"
#include "Blaster/GameMode/CaptureTheFlagGameMode.h"
#include "Blaster/Character/BlasterCharacter.h"

AFlagZone::AFlagZone()
{
	PrimaryActorTick.bCanEverTick = false;
	//������ײ��
	ZoneSphere = CreateDefaultSubobject<USphereComponent>(TEXT("ZoneSphere"));
	//���ø����
	SetRootComponent(ZoneSphere);
}

void AFlagZone::BeginPlay()
{
	Super::BeginPlay();
	//ע�����
	ZoneSphere->OnComponentBeginOverlap.AddDynamic(this, &AFlagZone::OnSphereOverlap);
}

void AFlagZone::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AFlag* OverlappingFlag = Cast<AFlag>(OtherActor);
	//�����ײ���������ģ������Ĳ��ǵ�ǰ��������,��ӷ�
	if (OverlappingFlag && OverlappingFlag->GetTeam() != Team)
	{
		ACaptureTheFlagGameMode* GameMode = GetWorld()->GetAuthGameMode<ACaptureTheFlagGameMode>();
		if (GameMode)
		{
			//�ӷ�
			GameMode->FlagCaptured(OverlappingFlag, this);
		}
		//������ײ��������
		OverlappingFlag->ResetFlag();
	}
}