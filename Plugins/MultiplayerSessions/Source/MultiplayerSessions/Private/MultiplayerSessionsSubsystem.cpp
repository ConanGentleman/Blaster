// Fill out your copyright notice in the Description page of Project Settings.


#include "MultiplayerSessionsSubsystem.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"

UMultiplayerSessionsSubsystem::UMultiplayerSessionsSubsystem():
	CreateSessionCompleteDelegate(FOnCreateSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnCreateSessionComplete)),
	FindSessionsCompleteDelegate(FOnFindSessionsCompleteDelegate::CreateUObject(this, &ThisClass::OnFindSessionsComplete)),
	JoinSessionCompleteDelegate(FOnJoinSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnJoinSessionComplete)),
	DestroySessionCompleteDelegate(FOnDestroySessionCompleteDelegate::CreateUObject(this, &ThisClass::OnDestroySessionComplete)),
	StartSessionCompleteDelegate(FOnStartSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnStartSessionComplete))
{
	//һ����ȡ����ϵͳ MultiplayerSessions��������ϵ�ͻ���������
	IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
	if (Subsystem) {
		SessionInterface = Subsystem->GetSessionInterface();
	}
}

void UMultiplayerSessionsSubsystem::CreateSession(int32 NumPublicConnections, FString MatchType)
{
	if (!SessionInterface.IsValid()) {
		return;
	}

	//����лỰ��������Ҫ���������Ա����ǿ�����ʹ���������лỰ��
	auto ExistingSession = SessionInterface->GetNamedSession(NAME_GameSession);
	if (ExistingSession != nullptr) {
		//�ȱ��洴���Ự�������Ϣ
		bCreateSessionOnDestroy = true;
		LastNumPublicConnections = NumPublicConnections;
		LastMatchType = MatchType;

		DestroySessions();
		
	}

	//�Ự������ɵİ�ί�У����洢ί�о�����Ա��Ժ���Խ����ί���б���ɾ��
	CreateSessionCompleteDeletegateHandle = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegate);
	
	//�����ûỰ
	LastSessionSettings = MakeShareable(new FOnlineSessionSettings());
	//�Ƿ��Ǿ��������� (IOnlineSubsystem::Get()->GetSubsystemName() ���δʹ��������ϵͳ�򷵻�NULL�ַ����������steam�򷵻�steam������ʹ����ϵͳ���þ�����
	LastSessionSettings->bIsLANMatch = IOnlineSubsystem::Get()->GetSubsystemName() == "NULL" ? true : false;
	//������������������
	LastSessionSettings->NumPublicConnections = NumPublicConnections;
	//�Ự�������У���������Ƿ���Լ��롣
	LastSessionSettings->bAllowJoinInProgress = true;
	//steam��һ����״̬�Ķ������������Ϊ�������ǵ�������������
	LastSessionSettings->bAllowJoinViaPresence = true;
	//��ƥ���Ƿ������߷����Ϲ�������[����������ҵ�����Ự]
	LastSessionSettings->bShouldAdvertise = true;
	//bUsesPresence :Whether to display user presence information or not
	//��ʾ�û�״̬�����������������ڽ��еĻỰ
	LastSessionSettings->bUsesPresence = true;
	//bUseLobbiesIfAvailable ��Whether to prefer lobbies APls if the platform supports them
	//���ƽ̨֧�֣��Ƿ�ѡ�������API(5.0���ϲ��������ܻ��޷��������Ự
	LastSessionSettings->bUseLobbiesIfAvailable = true;
	//����ƥ������(��ֵ�Ա������趨ƥ�����ͣ�����������ʹ�����������ƥ������)���Ựͨ�����߷����Լ�ping���й㲥
	LastSessionSettings->Set(FName("MatchType"), MatchType, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	//BuildUniqueId��������ֹ��ͬ�Ĺ����������������໥��������������Ϊ1��Ϊ�˿����ö���û������Լ��Ĺ�����Host
	//������������Ч����Ϸ�Ựʱ�����ǽ����������Ự���ܹ����룬�����޷�������
	// ���ǳ��Լ����1����Host�ĻỰ���������������Ϊ1��Ŀ�ģ���
	//
	LastSessionSettings->BuildUniqueId = 1;
	
	
	//�ӱ�����һ�ȡΨһ����ID	
	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	//�����Ự��������Ψһ����ID���Ự���ƣ��滭����
	if (!SessionInterface->CreateSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, *LastSessionSettings)) {
		//�������ʧ�ܣ�����ί���б���ɾ������ί��
		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDeletegateHandle);
		// �㲥�Զ���ĻỰ����״̬ί�е��˵���
		MultiplayerOnCreateSessionComplete.Broadcast(false);
	}
}
/// <summary>
/// Ѱ�һỰ
/// </summary>
/// <param name="MaxSearchResults"></param>
void UMultiplayerSessionsSubsystem::FindSessions(int32 MaxSearchResults)
{
	//�ҵ���Ϸ�Ự
	if (!SessionInterface.IsValid()) {
		return;
	}

	//���ί��.һ���ҵ��Ự���ص������ͻ�󶨵����ί����
	FindSessionCompleteDelegateHandle=SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegate);

	//����һ���Ự����
	LastSessionSearch = MakeShareable(new FOnlineSessionSearch());
	//�ԻỰ������������
	///�����������steamͨ�ò���IDΪ480���п����кܶ�����ͬʱ���ԣ���˰�ֵ���ø�һ�㣩
	LastSessionSearch->MaxSearchResults = MaxSearchResults;
	// ���Ƿ�رվ�������ѯ��IOnlineSubsystem::Get()->GetSubsystemName() ���δʹ��������ϵͳ�򷵻�NULL�ַ����������steam�򷵻�steam������ʹ����ϵͳ���þ�����
	LastSessionSearch->bIsLanQuery = IOnlineSubsystem::Get()->GetSubsystemName() == "NULL" ? true : false;
	//��ѯ���ã�ֻ��ѯ����ʹ��״̬����using presence�� ֵΪ true �ģ�Ҳ��������ʹ�õ�Session
	LastSessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);

	//�ӱ�����һ�ȡΨһ����ID	
	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	//���һỰ,������Ψһ����ID���Ự����
	if (!SessionInterface->FindSessions(*LocalPlayer->GetPreferredUniqueNetId(), LastSessionSearch.ToSharedRef())) {
		//û���ҵ��Ự ��ɾ��ί��
		SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionCompleteDelegateHandle);
		if (GEngine) {
			GEngine->AddOnScreenDebugMessage(
				-1,
				15.f,
				FColor::Blue,
				FString::Printf(TEXT("NoFindSessions"))
			);
		}
		//�㲥���һỰʧ�ܵĽ�����˵��࣬ ��˲�������һ���յĽ�����顣
		MultiplayerOnFindSessionComplete.Broadcast(TArray<FOnlineSessionSearchResult>(), false);
	}
}

void UMultiplayerSessionsSubsystem::JoinSession(const FOnlineSessionSearchResult& SessionResult)
{
	if (!SessionInterface.IsValid()) {
		//�㲥һ�� ����ʧ��
		MultiplayerOnJoinSessionComplete.Broadcast(EOnJoinSessionCompleteResult::Type::UnknownError);
		return;
	}
	//���ί��.һ������Ự���ص������ͻ�󶨵����ί����
	JoinSessionCompleteDelegateHandle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegate);

	//�ӱ�����һ�ȡΨһ����ID	
	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	//����Ự.������Ψһ����ID���Ự���ƣ��Ự�������
	if (!SessionInterface->JoinSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, SessionResult)) {
		//�������Ựʧ��

		//����ί��
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
		//�㲥����Ự���
		MultiplayerOnJoinSessionComplete.Broadcast(EOnJoinSessionCompleteResult::Type::UnknownError);
	}
}

/// <summary>
/// ���ٻỰ
/// </summary>
void UMultiplayerSessionsSubsystem::DestroySessions()
{
	if (!SessionInterface.IsValid()) {
		//�㲥���ٻỰ
		MultiplayerOnDestroySessionComplete.Broadcast(false);
		return;
	}
	//���ί��.һ������Ự���ص������ͻ�󶨵����ί����
	DestroySessionCompleteDelegateHandle = SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegate);
	
	//���ٻỰ���������Ự����
	if (!SessionInterface->DestroySession(NAME_GameSession)) {
		//����ί��
		SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
		//�㲥���ٻỰ
		MultiplayerOnDestroySessionComplete.Broadcast(false);
	}
}

void UMultiplayerSessionsSubsystem::StartSession()
{
}

/// <summary>
/// �Ự����ί�лص����Իص��ɹ��Ĵ���
/// </summary>
/// <param name="SessionName"></param>
/// <param name="bWasSuccessful"></param>
void UMultiplayerSessionsSubsystem::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (SessionInterface) {
		//�Ự������ɣ���ɾ���Ựί��
		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(FindSessionCompleteDelegateHandle);
	}

	// �㲥�Զ���ĻỰ����ί��״̬���˵���
	MultiplayerOnCreateSessionComplete.Broadcast(bWasSuccessful);
}
/// <summary>
/// �Ự����ί�лص����Իص��ɹ��Ĵ���
/// </summary>
/// <param name="bWasSuccessful"></param>
void UMultiplayerSessionsSubsystem::OnFindSessionsComplete(bool bWasSuccessful)
{
	if (SessionInterface) {
		//�Ự������ɣ���ɾ���Ựί��
		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDeletegateHandle);
	}

	if (LastSessionSearch->SearchResults.Num() <= 0) {
		//�㲥���һỰʧ�ܵĽ�����˵��࣬ ��˲�������һ���յĽ�����顣
		MultiplayerOnFindSessionComplete.Broadcast(TArray<FOnlineSessionSearchResult>(), false);
		return;
	}
	// �㲥�Զ���ĻỰ����ί�н�����˵��ࣨ���ҳɹ���
	MultiplayerOnFindSessionComplete.Broadcast(LastSessionSearch->SearchResults ,bWasSuccessful);
}
/// <summary>
/// �Ự����ί�лص����Իص��ɹ��Ĵ���
/// </summary>
/// <param name="SessionName"></param>
/// <param name="Result"></param>
void UMultiplayerSessionsSubsystem::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	if (SessionInterface) {
		//����ί��
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
	}

	MultiplayerOnJoinSessionComplete.Broadcast(Result);
}

void UMultiplayerSessionsSubsystem::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (SessionInterface) {
		SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
	}
	//���ٳɹ�����֮ǰ���ڴ����Ự������
	if (bWasSuccessful && bCreateSessionOnDestroy) {
		bCreateSessionOnDestroy = false;
		//�����Ự
		CreateSession(LastNumPublicConnections, LastMatchType);
	}
	MultiplayerOnDestroySessionComplete.Broadcast(bWasSuccessful);
}

void UMultiplayerSessionsSubsystem::OnStartSessionComplete(FName SessionName, bool bWasSuccessful)
{
}
