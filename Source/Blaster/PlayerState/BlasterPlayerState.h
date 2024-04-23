// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "BlasterPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API ABlasterPlayerState : public APlayerState
{
	GENERATED_BODY()
public:
	/// <summary>
	/// 更新分数（用于客户端更新）
	/// 当分数复制时调用。
	/// </summary>
	virtual void OnRep_Score() override;

	/// <summary>
	/// 更新分数（用于服务端更新）
	/// 由于复制仅发生在服务器到客户端，因此永远不会在服务器上调用OnRep_Score，因此还要为服务器编写一个分数修改时调用的函数
	/// </summary>
	/// <param name="ScoreAmount"></param>
	void AddToScore(float ScoreAmount);
private:
	//玩家状态类实际上没有直接访问角色和控制器的功能。因此在这里定义两个在OnRep_Score中初始化
	class ABlasterCharacter* Character;
	class ABlasterPlayerController* Controller;
};
