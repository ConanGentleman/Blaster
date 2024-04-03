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
	//添加到视口（添加到屏幕）
	AddToViewport();
	//设置可见性
	SetVisibility(ESlateVisibility::Visible);
	//如果将此标志设置为true，则允许该部件在单击或导航到时接受焦点.这行代码设置了一个布尔变量 bIsFocusable 为 true，表示菜单可以接受输入焦点。
	bIsFocusable = true;

	UWorld* World = GetWorld();
	if (World) {
		APlayerController* PlayerController = World->GetFirstPlayerController();
		if (PlayerController) {
			FInputModeUIOnly InputModeData;//该模式不会将输入应用于世界上的任何棋子，只关注用户界面
			//这行代码设置 InputModeData 的焦点小部件为当前菜单。TakeWidget 方法用于获取当前菜单的 UI 部件
			InputModeData.SetWidgetToFocus(TakeWidget());
			//锁定鼠标到视口（参数为不要锁定-鼠标光标不会锁定在视口上
			InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			//设置输入模式。
			PlayerController->SetInputMode(InputModeData);
			//设置光标为显示
			PlayerController->SetShowMouseCursor(true);
		}
	}

	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance) {
		//获取子系统
		MultiplayerSessionsSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
	}
	//绑定委托
	if (MultiplayerSessionsSubsystem) {
		MultiplayerSessionsSubsystem->MultiplayerOnCreateSessionComplete.AddDynamic(this, &ThisClass::OnCreateSession);
		//由于MultiplayerOnFindSessionComplete使用的是非动态委托，因此使用AddUObject而不是AddDynamic
		MultiplayerSessionsSubsystem->MultiplayerOnFindSessionsComplete.AddUObject(this, &ThisClass::OnFindSessions);
		MultiplayerSessionsSubsystem->MultiplayerOnJoinSessionComplete.AddUObject(this, &ThisClass::OnJoinSession);
		MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.AddDynamic(this, &ThisClass::OnDestroySession);
		MultiplayerSessionsSubsystem->MultiplayerOnStartSessionComplete.AddDynamic(this, &ThisClass::OnStartSession);
	}
}

bool UMenu::Initialize()
{
	if (!Super::Initialize()) {//如果父类返回false
		return false;
	}

	if (HostButton) {
		//绑定点击监听
		HostButton->OnClicked.AddDynamic(this, &ThisClass::HostButtonClicked);
	}
	if (JoinButton) {
		//绑定点击监听
		JoinButton->OnClicked.AddDynamic(this, &ThisClass::JoinButtonClicked);
	}

	return true;
}
/// <summary>
/// 当关卡从世界中移除时，所有UUserWidget会调用此函数（如场景跳转）
/// </summary>
/// <param name="InLevel"></param>
/// <param name="InWorld"></param>
void UMenu::OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld)
{
	MenuTearDown();
	//调用父类
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
			//会话创建成功后，跳转到大厅
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
		//创建会话失败则重新激活Host按钮
		HostButton->SetIsEnabled(true);
	}
}
/// <summary>
/// 来自MultiplayerSessionsSubsystem的会话查找结果回调
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
		//通过键值字符串：MatchType 找到其键值对并保存在变量MatchType中
		Result.Session.SessionSettings.Get(FName("MatchType"), SettingsValue);
		if (SettingsValue == MatchType) {//找到了会话
			if (GEngine) {
				GEngine->AddOnScreenDebugMessage(
					-1,
					15.f,
					FColor::Yellow,
					FString(TEXT("Session Find successfully!"))
				);
			}
			//加入会话
			MultiplayerSessionsSubsystem->JoinSession(Result);
			return;
		}
	}
	if (!bWasSuccessful || SessionResults.Num() == 0) {
		//查找会话失败则重新激活Join按钮
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
/// 来自MultiplayerSessionsSubsystem的会话加入结果回调
/// </summary>
/// <param name="Result"></param>
void UMenu::OnJoinSession(EOnJoinSessionCompleteResult::Type Result)
{
	//加入会话需要获取正确的ip地址，需要使用会话接口并调用获取解析的连接字符串，所以需要访问在线会话接口（OnlineSessionInterface)
		//一旦获取了子系统 MultiplayerSessions的依赖关系就会立即插入
	IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
	if (Subsystem) {
		IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();

		if (SessionInterface.IsValid()) {

			//IP地址
			FString Address;
			//返回用于加入匹配的特定于平台的连接信息,参数：会话名，接收IP的局部变量
			SessionInterface->GetResolvedConnectString(NAME_GameSession, Address);

			//由于菜单类不在角色或者pawn上，需使用GetGameInstance()获取
			//获取玩家控制器
			APlayerController* PlayerController = GetGameInstance()->GetFirstLocalPlayerController();
			if (PlayerController) {
				//客户端跳转场景（加入服务器场景）,参数：服务器IP地址，跳转类型
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
	//	//加入会话失败则重新激活Join按钮
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
	//禁用按钮（避免多次点击
	HostButton->SetIsEnabled(false);
	if (MultiplayerSessionsSubsystem) {
		MultiplayerSessionsSubsystem->CreateSession(NumPublicConnections, MatchType);
	}
}

void UMenu::JoinButtonClicked()
{
	//禁用按钮（避免多次点击
	JoinButton->SetIsEnabled(false);
	if (MultiplayerSessionsSubsystem) {
		//因为可能存在很多人使用steam的开发ID ： 480 ，所以将搜索结果数量设置大一点，以防找不到我们自己的
		MultiplayerSessionsSubsystem->FindSessions(10000);
	}
}

/// <summary>
/// 将玩家控制权从界面操作转回到人物操作
/// </summary>
void UMenu::MenuTearDown()
{
	//删除视口（移除UI）
	RemoveFromParent();
	UWorld* World = GetWorld();
	if (World) {
		APlayerController* PlayerController = World->GetFirstPlayerController();
		if (PlayerController) {
			FInputModeGameOnly InputModeData;//FInputModeGameOnly默认值就是人物控制，所以不再进行设置
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(false);//设置光标
		}
	}
}
