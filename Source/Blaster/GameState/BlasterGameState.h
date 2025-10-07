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
	/// 用于注册复制变量的函数
	/// </summary>
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	/// <summary>
	/// 更新最高分
	/// </summary>
	/// <param name="ScoringPlayer"></param>
	void UpdateTopScore(class ABlasterPlayerState* ScoringPlayer);

	/// <summary>
	/// 最高得分玩家的状态数组（复制变量
	/// </summary>
	UPROPERTY(Replicated)
	TArray<ABlasterPlayerState*> TopScoringPlayers;
private:

	float TopScore = 0.f;
};
