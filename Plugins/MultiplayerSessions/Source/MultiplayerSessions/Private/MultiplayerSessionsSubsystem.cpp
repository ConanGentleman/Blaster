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
	//一旦获取了子系统 MultiplayerSessions的依赖关系就会立即插入
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

	//如果有会话，我们需要销毁它，以便我们可以用使用所有现有会话。
	auto ExistingSession = SessionInterface->GetNamedSession(NAME_GameSession);
	if (ExistingSession != nullptr) {
		//先保存创建会话的相关信息
		bCreateSessionOnDestroy = true;
		LastNumPublicConnections = NumPublicConnections;
		LastMatchType = MatchType;

		DestroySessions();
		
	}

	//会话创建完成的绑定委托，并存储委托句柄，以便稍后可以将其从委托列表中删除
	CreateSessionCompleteDeletegateHandle = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegate);
	
	//先设置会话
	LastSessionSettings = MakeShareable(new FOnlineSessionSettings());
	//是否是局域网连接 (IOnlineSubsystem::Get()->GetSubsystemName() 如果未使用在线子系统则返回NULL字符串，如果是steam则返回steam。所以使用子系统则用局域网
	LastSessionSettings->bIsLANMatch = IOnlineSubsystem::Get()->GetSubsystemName() == "NULL" ? true : false;
	//最大玩家数（连接数）
	LastSessionSettings->NumPublicConnections = NumPublicConnections;
	//会话正在运行，其他玩家是否可以加入。
	LastSessionSettings->bAllowJoinInProgress = true;
	//steam有一个叫状态的东西，这个打开是为了让我们的连接正常工作
	LastSessionSettings->bAllowJoinViaPresence = true;
	//该匹配是否在在线服务上公开宣传[让其他玩家找到这个会话]
	LastSessionSettings->bShouldAdvertise = true;
	//bUsesPresence :Whether to display user presence information or not
	//显示用户状态，查找所在区域正在进行的会话
	LastSessionSettings->bUsesPresence = true;
	//bUseLobbiesIfAvailable ：Whether to prefer lobbies APls if the platform supports them
	//如果平台支持，是否选择大厅的API(5.0以上不开启可能会无法搜索到会话
	LastSessionSettings->bUseLobbiesIfAvailable = true;
	//设置匹配类型(键值对本身不必设定匹配类型，但这里我们使用这个来定义匹配类型)，会话通过在线服务以及ping进行广播
	LastSessionSettings->Set(FName("MatchType"), MatchType, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	//BuildUniqueId是用来防止不同的构建在搜索过程中相互看到，这里设置为1是为了可以让多个用户启动自己的构建和Host
	//这样当搜索有效的游戏会话时，我们将看到其他会话并能够加入，否则将无法看到。
	// 我们尝试加入第1个被Host的会话（估计这才是设置为1的目的？）
	//
	LastSessionSettings->BuildUniqueId = 1;
	
	
	//从本地玩家获取唯一网络ID	
	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	//创建会话，参数：唯一网络ID，会话名称，绘画设置
	if (!SessionInterface->CreateSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, *LastSessionSettings)) {
		//如果创建失败，则在委托列表中删除创建委托
		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDeletegateHandle);
		// 广播自定义的会话创建状态委托到菜单类
		MultiplayerOnCreateSessionComplete.Broadcast(false);
	}
}
/// <summary>
/// 寻找会话
/// </summary>
/// <param name="MaxSearchResults"></param>
void UMultiplayerSessionsSubsystem::FindSessions(int32 MaxSearchResults)
{
	//找到游戏会话
	if (!SessionInterface.IsValid()) {
		return;
	}

	//添加委托.一旦找到会话，回调函数就会绑定到这个委托上
	FindSessionCompleteDelegateHandle=SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegate);

	//定义一个会话搜索
	LastSessionSearch = MakeShareable(new FOnlineSessionSearch());
	//对会话搜索进行设置
	///搜索最大结果（steam通用测试ID为480，有可能有很多人在同时测试，因此把值设置高一点）
	LastSessionSearch->MaxSearchResults = MaxSearchResults;
	// 关是否关闭局域网查询。IOnlineSubsystem::Get()->GetSubsystemName() 如果未使用在线子系统则返回NULL字符串，如果是steam则返回steam。所以使用子系统则用局域网
	LastSessionSearch->bIsLanQuery = IOnlineSubsystem::Get()->GetSubsystemName() == "NULL" ? true : false;
	//查询设置，只查询处于使用状态（即using presence） 值为 true 的，也就是正在使用的Session
	LastSessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);

	//从本地玩家获取唯一网络ID	
	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	//查找会话,参数：唯一网络ID，会话搜索
	if (!SessionInterface->FindSessions(*LocalPlayer->GetPreferredUniqueNetId(), LastSessionSearch.ToSharedRef())) {
		//没有找到会话 则删除委托
		SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionCompleteDelegateHandle);
		if (GEngine) {
			GEngine->AddOnScreenDebugMessage(
				-1,
				15.f,
				FColor::Blue,
				FString::Printf(TEXT("NoFindSessions"))
			);
		}
		//广播查找会话失败的结果到菜单类， 因此参数传入一个空的结果数组。
		MultiplayerOnFindSessionComplete.Broadcast(TArray<FOnlineSessionSearchResult>(), false);
	}
}

void UMultiplayerSessionsSubsystem::JoinSession(const FOnlineSessionSearchResult& SessionResult)
{
	if (!SessionInterface.IsValid()) {
		//广播一个 加入失败
		MultiplayerOnJoinSessionComplete.Broadcast(EOnJoinSessionCompleteResult::Type::UnknownError);
		return;
	}
	//添加委托.一旦加入会话，回调函数就会绑定到这个委托上
	JoinSessionCompleteDelegateHandle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegate);

	//从本地玩家获取唯一网络ID	
	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	//加入会话.参数：唯一网络ID，会话名称，会话搜索结果
	if (!SessionInterface->JoinSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, SessionResult)) {
		//如果加入会话失败

		//清理委托
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
		//广播加入会话结果
		MultiplayerOnJoinSessionComplete.Broadcast(EOnJoinSessionCompleteResult::Type::UnknownError);
	}
}

/// <summary>
/// 销毁会话
/// </summary>
void UMultiplayerSessionsSubsystem::DestroySessions()
{
	if (!SessionInterface.IsValid()) {
		//广播销毁会话
		MultiplayerOnDestroySessionComplete.Broadcast(false);
		return;
	}
	//添加委托.一旦加入会话，回调函数就会绑定到这个委托上
	DestroySessionCompleteDelegateHandle = SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegate);
	
	//销毁会话。参数：会话名称
	if (!SessionInterface->DestroySession(NAME_GameSession)) {
		//清理委托
		SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
		//广播销毁会话
		MultiplayerOnDestroySessionComplete.Broadcast(false);
	}
}

void UMultiplayerSessionsSubsystem::StartSession()
{
}

/// <summary>
/// 会话创建委托回调（对回调成功的处理）
/// </summary>
/// <param name="SessionName"></param>
/// <param name="bWasSuccessful"></param>
void UMultiplayerSessionsSubsystem::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (SessionInterface) {
		//会话创建完成，则删除会话委托
		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(FindSessionCompleteDelegateHandle);
	}

	// 广播自定义的会话创建委托状态到菜单类
	MultiplayerOnCreateSessionComplete.Broadcast(bWasSuccessful);
}
/// <summary>
/// 会话查找委托回调（对回调成功的处理）
/// </summary>
/// <param name="bWasSuccessful"></param>
void UMultiplayerSessionsSubsystem::OnFindSessionsComplete(bool bWasSuccessful)
{
	if (SessionInterface) {
		//会话查找完成，则删除会话委托
		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDeletegateHandle);
	}

	if (LastSessionSearch->SearchResults.Num() <= 0) {
		//广播查找会话失败的结果到菜单类， 因此参数传入一个空的结果数组。
		MultiplayerOnFindSessionComplete.Broadcast(TArray<FOnlineSessionSearchResult>(), false);
		return;
	}
	// 广播自定义的会话查找委托结果到菜单类（查找成功）
	MultiplayerOnFindSessionComplete.Broadcast(LastSessionSearch->SearchResults ,bWasSuccessful);
}
/// <summary>
/// 会话加入委托回调（对回调成功的处理）
/// </summary>
/// <param name="SessionName"></param>
/// <param name="Result"></param>
void UMultiplayerSessionsSubsystem::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	if (SessionInterface) {
		//清理委托
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
	}

	MultiplayerOnJoinSessionComplete.Broadcast(Result);
}

void UMultiplayerSessionsSubsystem::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (SessionInterface) {
		SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
	}
	//销毁成功并且之前存在创建会话的请求
	if (bWasSuccessful && bCreateSessionOnDestroy) {
		bCreateSessionOnDestroy = false;
		//创建会话
		CreateSession(LastNumPublicConnections, LastMatchType);
	}
	MultiplayerOnDestroySessionComplete.Broadcast(bWasSuccessful);
}

void UMultiplayerSessionsSubsystem::OnStartSessionComplete(FName SessionName, bool bWasSuccessful)
{
}
