// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileBullet.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"

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