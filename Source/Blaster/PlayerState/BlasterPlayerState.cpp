// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterPlayerState.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Net/UnrealNetwork.h"

void ABlasterPlayerState::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	//注册复制变量 死亡数
	DOREPLIFETIME(ABlasterPlayerState, Defeats);
}


/// <summary>
/// 更新分数（用于服务端更新）
/// 由于复制仅发生在服务器到客户端，因此永远不会在服务器上调用OnRep_Score，因此还要为服务器编写一个分数修改时调用的函数
/// </summary>
/// <param name="ScoreAmount"></param>
void ABlasterPlayerState::AddToScore(float ScoreAmount)
{
	SetScore(GetScore() + ScoreAmount);//复制变量修改会触发OnRep_Score调用
	//获取当前控制的玩家角色
	Character = Character == nullptr ? Cast<ABlasterCharacter>(GetPawn()) : Character;
	//if (Character && Character->Controller)将原有的判断条件修改为了下一行，保证Controller的正常获取，以修复上一次出现的defeat不正确的问题
	if (Character)
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
	//if (Character && Character->Controller)将原有的判断条件修改为了下一行，保证Controller的正常获取，以修复上一次出现的defeat不正确的问题
	if (Character)
	{
		//获取当前控制器
		Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			// 设置HUD分数信息
			Controller->SetHUDScore(GetScore());
		}
	}
}

/// <summary>
/// 更新死亡数（用于服务端更新）
/// 由于复制仅发生在服务器到客户端，因此永远不会在服务器上调用OnRep_Defeats，因此还要为服务器编写一个死亡数修改时调用的函数
/// </summary>
/// <param name="ScoreAmount"></param>
void ABlasterPlayerState::AddToDefeats(int32 DefeatsAmount)
{
	Defeats += DefeatsAmount;//复制变量修改会触发OnRep_Defeats调用
	Character = Character == nullptr ? Cast<ABlasterCharacter>(GetPawn()) : Character;
	//if (Character && Character->Controller)将原有的判断条件修改为了下一行，保证Controller的正常获取，以修复上一次出现的defeat不正确的问题
	if (Character)
	{
		Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			Controller->SetHUDDefeats(Defeats);
		}
	}
}

/// <summary>
/// 更新死亡数（用于客户端更新）
/// 当死亡数复制时调用。
/// 由于玩家状态没有想分数一样提供死亡数，因此这里得自行定义，而不是重写。
/// </summary>
void ABlasterPlayerState::OnRep_Defeats()
{
	Character = Character == nullptr ? Cast<ABlasterCharacter>(GetPawn()) : Character;
	//if (Character && Character->Controller)将原有的判断条件修改为了下一行，保证Controller的正常获取，以修复上一次出现的defeat不正确的问题
	if (Character)
	{
		Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			Controller->SetHUDDefeats(Defeats);
		}
	}
}