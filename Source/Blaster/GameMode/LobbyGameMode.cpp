// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyGameMode.h"
#include "GameFramework/GameStateBase.h"

/// <summary>
/// 每当有人加入游戏时就会调用该函数
/// </summary>
/// <param name="NewPlayer"></param>
void ALobbyGameMode::PostLogin(APlayerController* NewPlayer) {
	Super::PostLogin(NewPlayer);
	//当前函数是第一个可以安全访问刚刚加入的玩家控制器的地方
	//通过GameState获取玩家数组,得到玩家数量
	//这里的流程是，第一个人服务器建立后，会跳转到大厅界面；
	//并等待加入大厅的玩家达到一定数量后，才通知所有人跳转到游戏场景。
	//但在跳转到游戏场景时，需要优先加载场景，因此会先跳转到过度场景，再跳转到游戏场景
	//过度场景（任意Map都行）用于等待游戏场景的加载，待加载完成后，所有玩家再跳转到加载场景（这个过程由UE提供，可在项目设置中的transition设置跳转地图即可）
	int32 NumberOfPlayers = GameState.Get()->PlayerArray.Num();
	if (NumberOfPlayers == 2) {
		UWorld* World = GetWorld();
		if (World) {
			//进行无缝旅游（无缝代表在旅游时不会先断开，再连接相同的服务器）
			bUseSeamlessTravel = true;
			//调用服务器旅行来使所有已连接的客户端旅游到游戏地图
			World->ServerTravel(FString("/Game/Maps/BlasterMap?listen"));
		}
	}

}