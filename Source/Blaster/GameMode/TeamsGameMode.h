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
	/// 玩家连接游戏进入房间
	/// 在一位新玩家完全登录并准备好进入游戏世界之后。这发生在玩家控制器被创建、初始化并与游戏世界关联之后，比 BeginPlay 更晚。
	/// </summary>
	virtual void PostLogin(APlayerController* NewPlayer) override;
	/// <summary>
	/// 玩家断连
	/// 在一位玩家的控制器（AController）即将被从世界中销毁之前。这发生在玩家主动断开连接、网络中断或服务器将其踢出时。
	/// </summary>
	/// <param name="Exiting"></param>
	virtual void Logout(AController* Exiting) override;
	 
	virtual float CalculateDamage(AController* Attacker, AController* Victim, float BaseDamage) override;
protected:
	/// <summary>
	/// 当比赛（Match）正式开始时。这个调用是由游戏框架内部触发的，通常在你调用 StartMatch() 之后发生。
	/// </summary>
	virtual void HandleMatchHasStarted() override;
};