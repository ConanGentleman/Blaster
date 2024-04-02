// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "LobbyGameMode.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API ALobbyGameMode : public AGameMode
{
	GENERATED_BODY()
public:
	/// <summary>
	/// 每当有人加入游戏时就会调用该函数
	/// </summary>
	virtual void PostLogin(APlayerController* NewPlayer) override;
};
