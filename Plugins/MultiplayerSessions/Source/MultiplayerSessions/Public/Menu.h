// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Menu.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERSESSIONS_API UMenu : public UUserWidget
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable)
	void MenuSetup(int32 NumberOfPublicConnections = 4, FString TypeOfMatch = FString(TEXT("FreeForAll")), FString LobbyPath = FString(TEXT("/Game/ThirdPerson/Maps/Lobby")));

protected:

	virtual bool Initialize() override;
	virtual void OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld) override;

	/// <summary>
	/// MultiplayerSessionsSubsystem的自定义委托回调 加UFUNTION是因为该回调是使用的动态委托，需要与蓝图兼容，则需要加UFUNCTION
	/// </summary>
	UFUNCTION()
	void OnCreateSession(bool bWasSuccessful);

	void OnFindSessions(const TArray<FOnlineSessionSearchResult>& SessionResults, bool bWasSuccessful);

	void OnJoinSession(EOnJoinSessionCompleteResult::Type Result);
	UFUNCTION()
	void OnDestroySession(bool bWasSuccessful);
	UFUNCTION()
	void OnStartSession(bool bWasSuccessful);

private:
	UPROPERTY(meta = (BindWidget)) //绑定蓝图中的同名（变量名）按钮
	class UButton* HostButton;

	UPROPERTY(meta = (BindWidget)) //绑定蓝图中的同名（变量名）按钮
	UButton* JoinButton;

	UFUNCTION()
	void HostButtonClicked();//蓝图中点击HostButton响应的函数

	UFUNCTION()
	void JoinButtonClicked();

	void MenuTearDown();

	//使用（自定义的）多人会话子系统，它是一个设计用于处理所有在线会话功能的子系统
	class UMultiplayerSessionsSubsystem* MultiplayerSessionsSubsystem;
	/// <summary>
	/// 最大公共连接数 {4}代表设置默认值为4
	/// </summary>
	int32 NumPublicConnections{4};
	/// <summary>
	/// 匹配类型
	/// </summary>
	FString MatchType{TEXT("FreeForAll")};
	/// <summary>
	/// 游戏大厅路径
	/// </summary>
	FString PathToLobby{TEXT("")};
};
