﻿// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "BlasterGameMode.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API ABlasterGameMode : public AGameMode
{
	GENERATED_BODY()
public:
	/// <summary>
	/// 玩家被淘汰（死亡）时调用的函数
	/// </summary>
	virtual void PlayerEliminated(class ABlasterCharacter* ElimmedCharacter, class ABlasterPlayerController* VictimController, ABlasterPlayerController* AttackerController);
	/// <summary>
	/// 玩家重生
	/// </summary>
	/// <param name="ElimmedCharacter">淘汰的角色</param>
	/// <param name="ElimmedController">淘汰的控制器</param>
	virtual void RequestRespawn(ACharacter* ElimmedCharacter, AController* ElimmedController);
};
