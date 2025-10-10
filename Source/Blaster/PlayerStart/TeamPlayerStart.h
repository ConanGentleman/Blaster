// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerStart.h"
#include "Blaster/BlasterTypes/Team.h"
#include "TeamPlayerStart.generated.h"

/**
 * 团队模式 出生点
 */
UCLASS()
class BLASTER_API ATeamPlayerStart : public APlayerStart
{
	GENERATED_BODY()
public:
	/// <summary>
	/// 出生点所对应的队伍
	/// </summary>
	UPROPERTY(EditAnywhere)
	ETeam Team;
};