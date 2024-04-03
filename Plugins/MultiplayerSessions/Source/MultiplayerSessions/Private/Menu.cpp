// Fill out your copyright notice in the Description page of Project Settings.


#include "Menu.h"
#include "Components/Button.h"
#include "MultiplayerSessionsSubsystem.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"
//#include "Interfaces/OnlineSessionInterface.h"

void UMenu::MenuSetup(int32 NumberOfPublicConnections, FString TypeOfMatch, FString LobbyPath) {
	PathToLobby = FString::Printf(TEXT("%s?listen"), *LobbyPath);
	NumPublicConnections = NumberOfPublicConnections;
	MatchType = TypeOfMatch;
	//��ӵ��ӿڣ���ӵ���Ļ��
	AddToViewport();
	//���ÿɼ���
	SetVisibility(ESlateVisibility::Visible);
	//������˱�־����Ϊtrue��������ò����ڵ����򵼺���ʱ���ܽ���.���д���������һ���������� bIsFocusable Ϊ true����ʾ�˵����Խ������뽹�㡣
	bIsFocusable = true;

	UWorld* World = GetWorld();
	if (World) {
		APlayerController* PlayerController = World->GetFirstPlayerController();
		if (PlayerController) {
			FInputModeUIOnly InputModeData;//��ģʽ���Ὣ����Ӧ���������ϵ��κ����ӣ�ֻ��ע�û�����
			//���д������� InputModeData �Ľ���С����Ϊ��ǰ�˵���TakeWidget �������ڻ�ȡ��ǰ�˵��� UI ����
			InputModeData.SetWidgetToFocus(TakeWidget());
			//������굽�ӿڣ�����Ϊ��Ҫ����-����겻���������ӿ���
			InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			//��������ģʽ��
			PlayerController->SetInputMode(InputModeData);
			//���ù��Ϊ��ʾ
			PlayerController->SetShowMouseCursor(true);
		}
	}

	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance) {
		//��ȡ��ϵͳ
		MultiplayerSessionsSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
	}
	//��ί��
	if (MultiplayerSessionsSubsystem) {
		MultiplayerSessionsSubsystem->MultiplayerOnCreateSessionComplete.AddDynamic(this, &ThisClass::OnCreateSession);
		//����MultiplayerOnFindSessionCompleteʹ�õ��ǷǶ�̬ί�У����ʹ��AddUObject������AddDynamic
		MultiplayerSessionsSubsystem->MultiplayerOnFindSessionsComplete.AddUObject(this, &ThisClass::OnFindSessions);
		MultiplayerSessionsSubsystem->MultiplayerOnJoinSessionComplete.AddUObject(this, &ThisClass::OnJoinSession);
		MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.AddDynamic(this, &ThisClass::OnDestroySession);
		MultiplayerSessionsSubsystem->MultiplayerOnStartSessionComplete.AddDynamic(this, &ThisClass::OnStartSession);
	}
}

bool UMenu::Initialize()
{
	if (!Super::Initialize()) {//������෵��false
		return false;
	}

	if (HostButton) {
		//�󶨵������
		HostButton->OnClicked.AddDynamic(this, &ThisClass::HostButtonClicked);
	}
	if (JoinButton) {
		//�󶨵������
		JoinButton->OnClicked.AddDynamic(this, &ThisClass::JoinButtonClicked);
	}

	return true;
}
/// <summary>
/// ���ؿ����������Ƴ�ʱ������UUserWidget����ô˺������糡����ת��
/// </summary>
/// <param name="InLevel"></param>
/// <param name="InWorld"></param>
void UMenu::OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld)
{
	MenuTearDown();
	//���ø���
	Super::OnLevelRemovedFromWorld(InLevel, InWorld);
}

void UMenu::OnCreateSession(bool bWasSuccessful)
{
	if (bWasSuccessful) {
		if (GEngine) {
			GEngine->AddOnScreenDebugMessage(
				-1,
				15.f,
				FColor::Yellow,
				FString(TEXT("Session created successfully!"))
			);
		}

		UWorld* World = GetWorld();
		if (World) {
			//�Ự�����ɹ�����ת������
			World->ServerTravel(PathToLobby);
		}
	}
	else {
		if (GEngine) {
			GEngine->AddOnScreenDebugMessage(
				-1,
				15.f,
				FColor::Red,
				FString(TEXT("Failed to create session!"))
			);
		}
		//�����Ựʧ�������¼���Host��ť
		HostButton->SetIsEnabled(true);
	}
}
/// <summary>
/// ����MultiplayerSessionsSubsystem�ĻỰ���ҽ���ص�
/// </summary>
/// <param name="SessionResults"></param>
/// <param name="bWasSuccessful"></param>
void UMenu::OnFindSessions(const TArray<FOnlineSessionSearchResult>& SessionResults, bool bWasSuccessful)
{
	if (MultiplayerSessionsSubsystem == nullptr) {
		return;
	}
	for (auto Result : SessionResults) {
		//FString Id = Result.GetSessionIdStr();
		//FString User = Result.Session.OwningUserName;
		FString SettingsValue;
		//ͨ����ֵ�ַ�����MatchType �ҵ����ֵ�Բ������ڱ���MatchType��
		Result.Session.SessionSettings.Get(FName("MatchType"), SettingsValue);
		if (SettingsValue == MatchType) {//�ҵ��˻Ự
			if (GEngine) {
				GEngine->AddOnScreenDebugMessage(
					-1,
					15.f,
					FColor::Yellow,
					FString(TEXT("Session Find successfully!"))
				);
			}
			//����Ự
			MultiplayerSessionsSubsystem->JoinSession(Result);
			return;
		}
	}
	if (!bWasSuccessful || SessionResults.Num() == 0) {
		//���һỰʧ�������¼���Join��ť
		JoinButton->SetIsEnabled(true);
	}
	if (GEngine) {
		GEngine->AddOnScreenDebugMessage(
			-1,
			15.f,
			FColor::Yellow,
			//FString(TEXT("Session Find error!%d", SessionResults.Num()))
			FString::Printf(TEXT("Session Find error! %d"), SessionResults.Num())
		);
	}
}
/// <summary>
/// ����MultiplayerSessionsSubsystem�ĻỰ�������ص�
/// </summary>
/// <param name="Result"></param>
void UMenu::OnJoinSession(EOnJoinSessionCompleteResult::Type Result)
{
	//����Ự��Ҫ��ȡ��ȷ��ip��ַ����Ҫʹ�ûỰ�ӿڲ����û�ȡ�����������ַ�����������Ҫ�������߻Ự�ӿڣ�OnlineSessionInterface)
		//һ����ȡ����ϵͳ MultiplayerSessions��������ϵ�ͻ���������
	IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
	if (Subsystem) {
		IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();

		if (SessionInterface.IsValid()) {

			//IP��ַ
			FString Address;
			//�������ڼ���ƥ����ض���ƽ̨��������Ϣ,�������Ự��������IP�ľֲ�����
			SessionInterface->GetResolvedConnectString(NAME_GameSession, Address);

			//���ڲ˵��಻�ڽ�ɫ����pawn�ϣ���ʹ��GetGameInstance()��ȡ
			//��ȡ��ҿ�����
			APlayerController* PlayerController = GetGameInstance()->GetFirstLocalPlayerController();
			if (PlayerController) {
				//�ͻ�����ת���������������������,������������IP��ַ����ת����
				PlayerController->ClientTravel(Address, ETravelType::TRAVEL_Absolute);
			}
			if (GEngine) {
				GEngine->AddOnScreenDebugMessage(
					-1,
					15.f,
					FColor::Yellow,
					FString(TEXT("Session Join successfully!"))
				);
			}
		}
		if (GEngine) {
			GEngine->AddOnScreenDebugMessage(
				-1,
				15.f,
				FColor::Yellow,
				FString(TEXT("Session Join error!"))
			);
		}
	}
	//if (Result != EOnJoinSessionCompleteResult::Success) {
	//	//����Ựʧ�������¼���Join��ť
	//	JoinButton->SetIsEnabled(true);
	//}

	if (GEngine) {
		GEngine->AddOnScreenDebugMessage(
			-1,
			15.f,
			FColor::Yellow,
			FString(TEXT("Subsystem error!"))
		);
	}
	
}

void UMenu::OnDestroySession(bool bWasSuccessful)
{
}

void UMenu::OnStartSession(bool bWasSuccessful)
{
}

void UMenu::HostButtonClicked()
{
	//���ð�ť�������ε��
	HostButton->SetIsEnabled(false);
	if (MultiplayerSessionsSubsystem) {
		MultiplayerSessionsSubsystem->CreateSession(NumPublicConnections, MatchType);
	}
}

void UMenu::JoinButtonClicked()
{
	//���ð�ť�������ε��
	JoinButton->SetIsEnabled(false);
	if (MultiplayerSessionsSubsystem) {
		//��Ϊ���ܴ��ںܶ���ʹ��steam�Ŀ���ID �� 480 �����Խ���������������ô�һ�㣬�Է��Ҳ��������Լ���
		MultiplayerSessionsSubsystem->FindSessions(10000);
	}
}

/// <summary>
/// ����ҿ���Ȩ�ӽ������ת�ص��������
/// </summary>
void UMenu::MenuTearDown()
{
	//ɾ���ӿڣ��Ƴ�UI��
	RemoveFromParent();
	UWorld* World = GetWorld();
	if (World) {
		APlayerController* PlayerController = World->GetFirstPlayerController();
		if (PlayerController) {
			FInputModeGameOnly InputModeData;//FInputModeGameOnlyĬ��ֵ����������ƣ����Բ��ٽ�������
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(false);//���ù��
		}
	}
}
