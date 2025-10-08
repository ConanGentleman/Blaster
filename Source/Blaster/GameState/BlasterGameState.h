// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "BlasterGameState.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API ABlasterGameState : public AGameState
{
	GENERATED_BODY()

public:
	/// <summary>
	/// ����ע�Ḵ�Ʊ����ĺ���
	/// </summary>
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	/// <summary>
	/// ������߷�
	/// </summary>
	/// <param name="ScoringPlayer"></param>
	void UpdateTopScore(class ABlasterPlayerState* ScoringPlayer);

	/// <summary>
	/// ��ߵ÷���ҵ�״̬���飨���Ʊ���
	/// </summary>
	UPROPERTY(Replicated)
	TArray<ABlasterPlayerState*> TopScoringPlayers;


	/**
	* Teams
	*/
	TArray<ABlasterPlayerState*> RedTeam;
	TArray<ABlasterPlayerState*> BlueTeam;

	/// <summary>
	/// ��ӷ���
	/// </summary>
	UPROPERTY(ReplicatedUsing = OnRep_RedTeamScore)
	float RedTeamScore = 0.f;

	/// <summary>
	/// ���ӷ���
	/// </summary>
	UPROPERTY(ReplicatedUsing = OnRep_BlueTeamScore)
	float BlueTeamScore = 0.f;

	/// <summary>
	/// �����ӷ��������Ʊ�����ֵ�ص�
	/// </summary>
	UFUNCTION()
	void OnRep_RedTeamScore();

	/// <summary>
	/// �������ӷ��������Ʊ�����ֵ�ص�
	/// </summary>
	UFUNCTION()
	void OnRep_BlueTeamScore();
private:

	float TopScore = 0.f;
};
