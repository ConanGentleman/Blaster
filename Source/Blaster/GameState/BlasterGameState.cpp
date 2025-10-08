// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterGameState.h"
#include "Net/UnrealNetwork.h"
#include "Blaster/PlayerState/BlasterPlayerState.h"
void ABlasterGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	//注册复制变量
	DOREPLIFETIME(ABlasterGameState, TopScoringPlayers);
	DOREPLIFETIME(ABlasterGameState, RedTeamScore);
	DOREPLIFETIME(ABlasterGameState, BlueTeamScore);
}
/// <summary>
/// 更新最高分
/// </summary>
/// <param name="ScoringPlayer"></param>
void ABlasterGameState::UpdateTopScore(class ABlasterPlayerState* ScoringPlayer)
{
	if (TopScoringPlayers.Num() == 0)//数组为空，则添加的人就是最高得分
	{
		TopScoringPlayers.Add(ScoringPlayer);
		TopScore = ScoringPlayer->GetScore();
	}
	else if (ScoringPlayer->GetScore() == TopScore)//如果添加的人与当前最高得分相同，则加入数组，数组中的人都是并列的
	{
		TopScoringPlayers.AddUnique(ScoringPlayer);//AddUnique 去重
	}
	else if (ScoringPlayer->GetScore() > TopScore)//如果添加的人的分数大于当前分数，则清空数组重新添加
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