// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "BlasterGameMode.generated.h"

//向AGameMode类中的命名空间为MatchState的游戏状态添加一个冷却游戏状态
//这里的BLASTER_API与AGameMode类中的MatchState的游戏状态的_API作用相同，但是只能使用BLASTER_API，ENGINE_AP是引擎使用的。其作用的使变量能够通过dll动态导入（实际上不加好像也可以？
//加上extern 方便在.cpp中访问使用
namespace MatchState
{
	extern BLASTER_API const FName Cooldown; //定义冷却游戏模式状态。比赛结束，显示获胜者并开始执行冷却计时器的状态
}


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

	void PlayerLeftGame(class ABlasterPlayerState* PlayerLeaving);

	/// <summary>
	/// 预热时间（游戏开始前的等待时间）
	/// </summary>
	UPROPERTY(EditDefaultsOnly)
	float WarmupTime = 10.f;

	/// <summary>
	/// 比赛时间
	/// </summary>
	UPROPERTY(EditDefaultsOnly)
	float MatchTime = 120.f;

	/// <summary>
	/// 比赛结束冷却时间
	/// </summary>
	UPROPERTY(EditDefaultsOnly)
	float CooldownTime = 10.f;


	/// <summary>
	/// 进入关卡的时间（但游戏还未开始
	/// </summary>
	float LevelStartingTime = 0.f;

protected:
	virtual void BeginPlay() override;
	/// <summary>
	/// 游戏状态设置的回调
	/// </summary>
	virtual void OnMatchStateSet() override;
private:
	/// <summary>
	/// 预热时间倒计时
	/// </summary>
	float CountdownTime = 0.f;
public:
	FORCEINLINE float GetCountdownTime() const { return CountdownTime; }
};
