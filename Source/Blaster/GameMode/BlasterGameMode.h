// Fill out your copyright notice in the Description page of Project Settings.

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
	ABlasterGameMode();
	/// <summary>
	/// 每帧更新
	/// </summary>
	/// <param name="DeltaTime"></param>
	virtual void Tick(float DeltaTime) override;
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

	/// <summary>
	/// 预热时间（游戏开始前的等待时间）
	/// </summary>
	UPROPERTY(EditDefaultsOnly)
	float WarmupTime = 10.f;

	/// <summary>
	/// 进入关卡的时间（但游戏还未开始
	/// </summary>
	float LevelStartingTime = 0.f;

protected:
	virtual void BeginPlay() override;

private:
	/// <summary>
	/// 预热时间倒计时
	/// </summary>
	float CountdownTime = 0.f;
};
