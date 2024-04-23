// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterPlayerState.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"

/// <summary>
/// 更新分数（用于服务端更新）
/// 由于复制仅发生在服务器到客户端，因此永远不会在服务器上调用OnRep_Score，因此还要为服务器编写一个分数修改时调用的函数
/// </summary>
/// <param name="ScoreAmount"></param>
void ABlasterPlayerState::AddToScore(float ScoreAmount)
{
	Score += ScoreAmount;
	//获取当前控制的玩家角色
	Character = Character == nullptr ? Cast<ABlasterCharacter>(GetPawn()) : Character;
	if (Character && Character->Controller)
	{
		//获取当前控制器
		Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			//设置HUD分数信息
			Controller->SetHUDScore(Score);
		}
	}
}

/// <summary>
/// 更新分数（用于客户端更新）
/// 当分数复制时调用。
/// </summary>
void ABlasterPlayerState::OnRep_Score()
{
	Super::OnRep_Score();

	//获取当前控制的玩家角色
	Character = Character == nullptr ? Cast<ABlasterCharacter>(GetPawn()) : Character;
	if (Character && Character->Controller)
	{
		//获取当前控制器
		Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			// 设置HUD分数信息
			Controller->SetHUDScore(Score);
		}
	}
}