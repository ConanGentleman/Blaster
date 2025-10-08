// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BlasterGameMode.h"
#include "TeamsGameMode.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API ATeamsGameMode : public ABlasterGameMode
{
	GENERATED_BODY()
public:
	ATeamsGameMode();
	/// <summary>
	/// ���������Ϸ���뷿��
	/// ��һλ�������ȫ��¼��׼���ý�����Ϸ����֮���ⷢ������ҿ���������������ʼ��������Ϸ�������֮�󣬱� BeginPlay ����
	/// </summary>
	virtual void PostLogin(APlayerController* NewPlayer) override;
	/// <summary>
	/// ��Ҷ���
	/// ��һλ��ҵĿ�������AController��������������������֮ǰ���ⷢ������������Ͽ����ӡ������жϻ�����������߳�ʱ��
	/// </summary>
	/// <param name="Exiting"></param>
	virtual void Logout(AController* Exiting) override;
	 
	virtual float CalculateDamage(AController* Attacker, AController* Victim, float BaseDamage) override;
protected:
	/// <summary>
	/// ��������Match����ʽ��ʼʱ���������������Ϸ����ڲ������ģ�ͨ��������� StartMatch() ֮������
	/// </summary>
	virtual void HandleMatchHasStarted() override;
};