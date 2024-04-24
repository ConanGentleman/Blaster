// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterGameMode.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "Blaster/PlayerState/BlasterPlayerState.h"

//GameMode仅存于服务器上！！！！

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

	if (AttackerPlayerState && AttackerPlayerState != VictimPlayerState)
	{
		//玩家死亡，则给予攻击者1分
		AttackerPlayerState->AddToScore(1.f);
	}
	//死亡的玩家死亡次数+1
	if (VictimPlayerState)
	{
		VictimPlayerState->AddToDefeats(1);
	}

	if (ElimmedCharacter)
	{
		ElimmedCharacter->Elim();
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
		TArray<AActor*> PlayerStarts;
		//获取场景中所有玩家重生点
		UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), PlayerStarts);
		//随机选择重生点点
		int32 Selection = FMath::RandRange(0, PlayerStarts.Num() - 1);
		//游戏模式自带的 重生角色 的函数。
		RestartPlayerAtPlayerStart(ElimmedController, PlayerStarts[Selection]);
	}
}
