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
	/// MultiplayerSessionsSubsystem���Զ���ί�лص� ��UFUNTION����Ϊ�ûص���ʹ�õĶ�̬ί�У���Ҫ����ͼ���ݣ�����Ҫ��UFUNCTION
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
	UPROPERTY(meta = (BindWidget)) //����ͼ�е�ͬ��������������ť
	class UButton* HostButton;

	UPROPERTY(meta = (BindWidget)) //����ͼ�е�ͬ��������������ť
	UButton* JoinButton;

	UFUNCTION()
	void HostButtonClicked();//��ͼ�е��HostButton��Ӧ�ĺ���

	UFUNCTION()
	void JoinButtonClicked();

	void MenuTearDown();

	//ʹ�ã��Զ���ģ����˻Ự��ϵͳ������һ��������ڴ����������߻Ự���ܵ���ϵͳ
	class UMultiplayerSessionsSubsystem* MultiplayerSessionsSubsystem;
	/// <summary>
	/// ��󹫹������� {4}��������Ĭ��ֵΪ4
	/// </summary>
	int32 NumPublicConnections{4};
	/// <summary>
	/// ƥ������
	/// </summary>
	FString MatchType{TEXT("FreeForAll")};
	/// <summary>
	/// ��Ϸ����·��
	/// </summary>
	FString PathToLobby{TEXT("")};
};
