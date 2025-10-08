// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "Blaster/BlasterTypes/Team.h"
#include "BlasterPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API ABlasterPlayerState : public APlayerState
{
	GENERATED_BODY()
public:

	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const override;

	/// <summary>
	/// 更新分数（用于客户端更新）
	/// 当分数复制时调用。
	/// </summary>
	virtual void OnRep_Score() override;

	/// <summary>
	/// 更新死亡数（用于客户端更新）
	/// 当死亡数复制时调用。
	/// 由于玩家状态没有想分数一样提供死亡数，因此这里得自行定义，而不是重写。
	/// </summary>
	UFUNCTION()
	virtual void OnRep_Defeats();

	/// <summary>
	/// 更新分数（用于服务端更新）
	/// 由于复制仅发生在服务器到客户端，因此永远不会在服务器上调用OnRep_Score，因此还要为服务器编写一个分数修改时调用的函数
	/// </summary>
	/// <param name="ScoreAmount"></param>
	void AddToScore(float ScoreAmount);

	/// <summary>
	/// 更新死亡数（用于服务端更新）
	/// 由于复制仅发生在服务器到客户端，因此永远不会在服务器上调用OnRep_Defeats，因此还要为服务器编写一个死亡数修改时调用的函数
	/// </summary>
	/// <param name="ScoreAmount"></param>
	void AddToDefeats(int32 DefeatsAmount);
private:
	//玩家状态类实际上没有直接访问角色和控制器的功能。因此在这里定义两个在OnRep_Score中初始化
	//加上UPROPERTY()的原因是让Character初始化为nullptr，即与class ABlasterCharacter* Character=nullptr相同
	UPROPERTY()
	class ABlasterCharacter* Character;
	UPROPERTY()
	class ABlasterPlayerController* Controller;

	/// <summary>
	/// 死亡数
	/// </summary>
	UPROPERTY(ReplicatedUsing = OnRep_Defeats)
	int32 Defeats;

	/// <summary>
	/// 所属队伍
	/// </summary>
	UPROPERTY(Replicated)
	ETeam Team = ETeam::ET_NoTeam;

public:
	FORCEINLINE ETeam GetTeam() const { return Team; }
	FORCEINLINE void SetTeam(ETeam TeamToSet) { Team = TeamToSet; }
};
