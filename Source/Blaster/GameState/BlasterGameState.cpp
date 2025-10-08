// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterGameState.h"
#include "Net/UnrealNetwork.h"
#include "Blaster/PlayerState/BlasterPlayerState.h"
void ABlasterGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	//ע�Ḵ�Ʊ���
	DOREPLIFETIME(ABlasterGameState, TopScoringPlayers);
	DOREPLIFETIME(ABlasterGameState, RedTeamScore);
	DOREPLIFETIME(ABlasterGameState, BlueTeamScore);
}
/// <summary>
/// ������߷�
/// </summary>
/// <param name="ScoringPlayer"></param>
void ABlasterGameState::UpdateTopScore(class ABlasterPlayerState* ScoringPlayer)
{
	if (TopScoringPlayers.Num() == 0)//����Ϊ�գ�����ӵ��˾�����ߵ÷�
	{
		TopScoringPlayers.Add(ScoringPlayer);
		TopScore = ScoringPlayer->GetScore();
	}
	else if (ScoringPlayer->GetScore() == TopScore)//�����ӵ����뵱ǰ��ߵ÷���ͬ����������飬�����е��˶��ǲ��е�
	{
		TopScoringPlayers.AddUnique(ScoringPlayer);//AddUnique ȥ��
	}
	else if (ScoringPlayer->GetScore() > TopScore)//�����ӵ��˵ķ������ڵ�ǰ����������������������
	{
		TopScoringPlayers.Empty();
		TopScoringPlayers.AddUnique(ScoringPlayer);
		TopScore = ScoringPlayer->GetScore();
	}
}

void ABlasterGameState::OnRep_RedTeamScore()
{
}

void ABlasterGameState::OnRep_BlueTeamScore()
{
}