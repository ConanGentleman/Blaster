// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterGameMode.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "Blaster/PlayerState/BlasterPlayerState.h"
#include "Blaster/GameState/BlasterGameState.h"

//GameMode仅存于服务器上！！！！

//向AGameMode类中的命名空间为MatchState的游戏状态添加一个冷却游戏状态（.h中也要添加）
namespace MatchState
{
	const FName Cooldown = FName("Cooldown");	// 定义冷却游戏模式状态。
}

//构造函数
ABlasterGameMode::ABlasterGameMode()
{
	bDelayedStart = true;//是否延迟开始游戏（即让游戏状态停留在等待开始状态）
}

void ABlasterGameMode::BeginPlay()
{
	Super::BeginPlay();
	//设置进入游戏关卡时的时刻
	LevelStartingTime = GetWorld()->GetTimeSeconds();
}

void ABlasterGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (MatchState == MatchState::WaitingToStart)
	{
		//实际上就是 WarmupTime - (GetWorld()->GetTimeSeconds() - LevelStartingTime)
		//看预热时间是否为0
		CountdownTime = WarmupTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountdownTime <= 0.f)
		{
			StartMatch();//开始游戏，将游戏状态变为InProgress 进行中
		}
	}
	else if (MatchState == MatchState::InProgress)
	{
		CountdownTime = WarmupTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountdownTime <= 0.f)
		{
			//如果游戏结束，则转换为游戏冷却状态
			SetMatchState(MatchState::Cooldown);
		}
	}
	else if (MatchState == MatchState::Cooldown)
	{
		CountdownTime = CooldownTime + WarmupTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountdownTime <= 0.f)
		{
			//游戏冷却状态结束，重开游戏
			RestartGame();
		}
	}
}

/// <summary>
/// 游戏状态设置的回调
/// </summary>
void ABlasterGameMode::OnMatchStateSet()
{
	Super::OnMatchStateSet();
	//获取所有的玩家控制器，并设置他们的游戏模式状态变量
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		ABlasterPlayerController* BlasterPlayer = Cast<ABlasterPlayerController>(*It);
		if (BlasterPlayer)
		{
			//每次状态改变都告诉玩家控制器，一旦转换到 进行中 状态玩家控制器就显示响应的HUD
			BlasterPlayer->OnMatchStateSet(MatchState);
		}
	}
}


/// <summary>
/// 玩家被淘汰（死亡）时调用的函数 （比如处理增加玩家得分之类的事情
/// </summary>
/// <param name="ElimmedCharacter">被淘汰的角色</param>
/// <param name="VictimController">受害者控制器</param>
/// <param name="AttackerController">攻击者控制器</param>
void ABlasterGameMode::PlayerEliminated(class ABlasterCharacter* ElimmedCharacter, class ABlasterPlayerController* VictimController, ABlasterPlayerController* AttackerController)
{
	if (AttackerController == nullptr || AttackerController->PlayerState == nullptr) return;
	if (VictimController == nullptr || VictimController->PlayerState == nullptr) return;
	//获取攻击者和淘汰者的玩家状态
	ABlasterPlayerState* AttackerPlayerState = AttackerController ? Cast<ABlasterPlayerState>(AttackerController->PlayerState) : nullptr;
	ABlasterPlayerState* VictimPlayerState = VictimController ? Cast<ABlasterPlayerState>(VictimController->PlayerState) : nullptr;

	//获取游戏状态（包含最高得分等信息）
	ABlasterGameState* BlasterGameState = GetGameState<ABlasterGameState>();
	if (AttackerPlayerState && AttackerPlayerState != VictimPlayerState && BlasterGameState)
	{
		//玩家死亡，则给予攻击者1分
		AttackerPlayerState->AddToScore(1.f);
		//得分则更新一下最高得分
		BlasterGameState->UpdateTopScore(AttackerPlayerState);
	}
	//死亡的玩家死亡次数+1
	if (VictimPlayerState)
	{
		VictimPlayerState->AddToDefeats(1);
	}

	if (ElimmedCharacter)
	{
		//传false表示不是主动退出游戏导致的死亡
		ElimmedCharacter->Elim(false);
	}
}

/// <summary>
/// 玩家重生
/// </summary>
/// <param name="ElimmedCharacter">淘汰的角色</param>
/// <param name="ElimmedController">淘汰的控制器</param>
void ABlasterGameMode::RequestRespawn(ACharacter* ElimmedCharacter, AController* ElimmedController)
{
	if (ElimmedCharacter)
	{
		//重置角色。它调用 从挂起（待定）的销毁控制器中分离。
		//就是将角色与控制器分离。以便控制器可以重新拥有另一个角色（估计就是以便控制器能够在当前角色销毁后，去控制新生成的角色）
		//另一方面，由于角色被销毁后所有信息都将消失，但是玩家控制器的信息将保留
		ElimmedCharacter->Reset();

		ElimmedCharacter->Destroy();
	}
	if (ElimmedController)
	{
		UE_LOG(LogTemp, Warning, TEXT("ElimmedController valid"))
		TArray<AActor*> PlayerStarts;
		//获取场景中所有玩家重生点
		UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), PlayerStarts);
		//随机选择重生点点
		int32 Selection = FMath::RandRange(0, PlayerStarts.Num() - 1);
		//游戏模式自带的 重生角色 的函数。
		RestartPlayerAtPlayerStart(ElimmedController, PlayerStarts[Selection]);
	}
}

/// <summary>
/// 处理某个玩家离开游戏
/// </summary>
/// <param name="PlayerLeaving"></param>
void ABlasterGameMode::PlayerLeftGame(ABlasterPlayerState* PlayerLeaving)
{
	if (PlayerLeaving == nullptr) return;
	ABlasterGameState* BlasterGameState = GetGameState<ABlasterGameState>();
	//将玩家从最高积分内中删除
	if (BlasterGameState && BlasterGameState->TopScoringPlayers.Contains(PlayerLeaving))
	{
		BlasterGameState->TopScoringPlayers.Remove(PlayerLeaving);
	}
	//设置玩家为淘汰/死亡状态
	ABlasterCharacter* CharacterLeaving = Cast<ABlasterCharacter>(PlayerLeaving->GetPawn());
	if (CharacterLeaving)
	{
		CharacterLeaving->Elim(true);
	}
}