// Fill out your copyright notice in the Description page of Project Settings.


#include "Flag.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Blaster/Character/BlasterCharacter.h"

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

void AFlag::Dropped()
{
	//��������״̬��
	SetWeaponState(EWeaponState::EWS_Dropped);
	//����������EDetachmentRule::KeepWorld��ʾ�����ᱣ��������任
	FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld, true);
	//���������������룬����������
	FlagMesh->DetachFromComponent(DetachRules);
	//����������Ϊ��
	SetOwner(nullptr);
	//������������Ͳ�Ӧ������������
	BlasterOwnerCharacter = nullptr;
	BlasterOwnerController = nullptr;
}

/// <summary>
/// ��������λ�ã������˻ص÷ֺ���Ҫ���ûصط�λ�ã�
/// </summary>
void AFlag::ResetFlag()
{
	ABlasterCharacter* FlagBearer = Cast<ABlasterCharacter>(GetOwner());
	//���������ҵĳ���״̬
	if (FlagBearer)
	{
		FlagBearer->SetHoldingTheFlag(false);
		//������ײ������Ϊnull����Ȼ�������ĺ���Ҿ������û����ײ���κ����������ɿ��԰�E����
		FlagBearer->SetOverlappingWeapon(nullptr);
		FlagBearer->UnCrouch();
	}

	if (!HasAuthority()) return;
	//���������ײ����
	FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld, true);
	FlagMesh->DetachFromComponent(DetachRules);
	SetWeaponState(EWeaponState::EWS_Initial);//������Ϊ��ʼ״̬
	//��������Ϊ��ʼ״̬�󲢲��ᴥ��OnWeaponStateSet�����е�case�����������Ҫ�ֶ�����һ����ײ����
	GetAreaSphere()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetAreaSphere()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
	//����������
	SetOwner(nullptr);
	BlasterOwnerCharacter = nullptr;
	BlasterOwnerController = nullptr;
	//���ûس�ʼλ��
	SetActorTransform(InitialTransform);
}

void AFlag::OnEquipped()
{
	//װ�����ĺ�������������ʾ
	ShowPickupWidget(false);
	//װ�����ĺ󣬽���������ײ��������װ���ϵ�������Ȼ�ܹ�����ص���
	GetAreaSphere()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	//������CombatComponent������Owener����Ϊ����OwenerҲ��ReplicatedUsing����ˣ������ڷ�����������ʱ���ͻ���Ҳͬ����
	//�ر���������ģ��
	FlagMesh->SetSimulatePhysics(false);
	//�ر�����������Ӱ��
	FlagMesh->SetEnableGravity(false);
	//���������޷���ײ
	FlagMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	FlagMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Overlap);
	//���������ɫ��������
	EnableCustomDepth(false);
}

void AFlag::OnDropped()
{
	/*
	����set weapon state�ڷ������˱����ã������ǲ�֪���Ƿ���ڿͻ��˵���set weapons state��
	����������Ҫȷ�����������Ϊ��������������ײ������ֻ�ڷ�������ִ�У��������ǾͿ���ȷ�����������ڷ�������ִ�С�
	*/
	if (HasAuthority())  //�������������ڷ���������������ײ��
	{
		GetAreaSphere()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}
	//������������ģ��
	FlagMesh->SetSimulatePhysics(true);
	//����������Ӱ��
	FlagMesh->SetEnableGravity(true);
	//��������������ײ
	FlagMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	//�ܹ��������ģ����Ҷ���ʱ��������ϣ��������ú������ײͨ������������ͨ������ײ��Ӧ������ֹ
	FlagMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	//������ײ����Pawn
	FlagMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	//������ײ�������
	FlagMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);

	FlagMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_BLUE);
	//���ڱ����Ⱦ״̬Ϊ��״̬������ζ���ڵ�ǰ֡����ʱ�Ὣ�䷢�͵���Ⱦ�߳�
	//ͨ������Ҫ����������Ӿ�����ʱʹ�ã�������Ĳ��ʻ�任
	FlagMesh->MarkRenderStateDirty();
	//��������ʱ����������ʾ
	EnableCustomDepth(true);

}

void AFlag::BeginPlay()
{
	Super::BeginPlay();
	InitialTransform = GetActorTransform();
}