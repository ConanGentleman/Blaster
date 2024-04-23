// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterGameMode.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"

//GameMode只会发生在服务器上

/// <summary>
/// 玩家被淘汰（死亡）时调用的函数 （比如处理增加玩家得分之类的事情
/// </summary>
/// <param name="ElimmedCharacter">被淘汰的角色</param>
/// <param name="VictimController">受害者控制器</param>
/// <param name="AttackerController">攻击者控制器</param>
void ABlasterGameMode::PlayerEliminated(class ABlasterCharacter* ElimmedCharacter, class ABlasterPlayerController* VictimController, ABlasterPlayerController* AttackerController)
{
	if (ElimmedCharacter)
	{
		ElimmedCharacter->Elim();
	}
}
