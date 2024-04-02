// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"

#include "MultiplayerSessionsSubsystem.generated.h"

//
// 声明自己的自定义委托用于将回调绑定到菜单类
//（多播MULTICAST 意味着一旦广播，多个类能够将其函数绑定到该委托）
//（动态DYNAMIC 意味着可以序列化委托，并且可以从蓝图中保存或者加载他们
//值得注意的是因为其是动态的，意味着绑定到他的函数必须是UFUNTION，不然无法绑定成功
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiplayerOnCreateSessionComplete, bool, bWasSuccessful);
///
/// 在线会话搜索结果不是一个新的类或者结构体，如果想要动态多播委托，所有的类型都必须与蓝图兼容。因此，在线会话搜索结果不能设置为动态委托
/// 需使用另一种多播委托,原因就是会话结果变量无法与蓝图兼容。同时该委托与动态委托在参数上还有不同，即类型和名称之间是否有逗号。
/// 
DECLARE_MULTICAST_DELEGATE_TwoParams(FMultiplayerOnFindSessionComplete, const TArray<FOnlineSessionSearchResult>& SessionResults, bool bWasSuccessful);
DECLARE_MULTICAST_DELEGATE_OneParam(FMultiplayerOnJoinSessionComplete, EOnJoinSessionCompleteResult::Type Result);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiplayerOnDestroySessionComplete, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiplayerOnStartSessionComplete, bool, bWasSuccessful);

/**
 * 
 */
UCLASS()
class MULTIPLAYERSESSIONS_API UMultiplayerSessionsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
public:
	UMultiplayerSessionsSubsystem();

	//
	// 处理会话功能。 菜单类可以调用它们
	//
	
	/// <summary>
	/// 创建会话
	/// </summary>
	/// <param name="NumPublicConnections">玩家数量</param>
	/// <param name="MatchType">匹配类型</param>
	void CreateSession(int32 NumPublicConnections,FString MatchType);
	/// <summary>
	/// 搜索会话
	/// </summary>
	/// <param name="MaxSearchResults">最大搜索结果数量</param>
	void FindSessions(int32 MaxSearchResults);
	/// <summary>
	/// 加入会话
	/// </summary>
	/// <param name="SessionResult"></param>
	void JoinSession(const FOnlineSessionSearchResult& SessionResult);
	/// <summary>
	/// 销毁会话
	/// </summary>
	void DestroySessions();
	/// <summary>
	/// 开始会话
	/// </summary>
	void StartSession();

	/// <summary>
	/// 这是我们为menu类绑定回调的自定义委托
	/// </summary>
	FMultiplayerOnCreateSessionComplete MultiplayerOnCreateSessionComplete;
	FMultiplayerOnFindSessionComplete MultiplayerOnFindSessionComplete;
	FMultiplayerOnJoinSessionComplete MultiplayerOnJoinSessionComplete;
	FMultiplayerOnDestroySessionComplete MultiplayerOnDestroySessionComplete;
	FMultiplayerOnStartSessionComplete MultiplayerOnStartSessionComplete;

protected:

	//
	// 对于委托的内部回调将添加到OnlineSessionInterface的委托列表
	// 这些不需要在类外调用
	//

	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnFindSessionsComplete(bool bWasSuccessful);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);
	void OnStartSessionComplete(FName SessionName, bool bWasSuccessful);

private:
	/// <summary>
	/// 会话接口
	/// </summary>
	IOnlineSessionPtr SessionInterface;
	//存储最后的会话设置（因为再创建会话时，若存在会话，则会先销毁再创建再设置，所以是最后的会话）
	TSharedPtr<FOnlineSessionSettings> LastSessionSettings;
	/// <summary>
	/// 会话搜索
	/// </summary>
	TSharedPtr<FOnlineSessionSearch> LastSessionSearch;

	//
	// 添加到在线会话接口委托列表
	// 将MultiplayerSessionsSubsystem 内部回调到绑定这里
	// （ps:下面就是一些委托

	/// <summary>
	/// 会话创建委托
	/// </summary>
	FOnCreateSessionCompleteDelegate CreateSessionCompleteDelegate;
	//句柄（用来标识不同的代理处理对象, 是一个 uint64 的ID，接收返回的委托句柄） 用于委托的删除
	FDelegateHandle CreateSessionCompleteDelegateHandle;
	/// <summary>
	/// 会话查找委托
	/// </summary>
	FOnFindSessionsCompleteDelegate FindSessionsCompleteDelegate;
	//句柄
	FDelegateHandle FindSessionsCompleteDelegateHandle;
	/// <summary>
	/// 会话加入委托
	/// </summary>
	FOnJoinSessionCompleteDelegate JoinSessionCompleteDelegate;
	//句柄
	FDelegateHandle JoinSessionCompleteDelegateHandle;
	/// <summary>
	/// 会话销毁委托
	/// </summary>
	FOnDestroySessionCompleteDelegate DestroySessionCompleteDelegate;
	//句柄
	FDelegateHandle DestroySessionCompleteDelegateHandle;
	/// <summary>
	/// 会话开始委托
	/// </summary>
	FOnStartSessionCompleteDelegate StartSessionCompleteDelegate;
	//句柄
	FDelegateHandle StartSessionCompleteDelegateHandle;

	/// <summary>
	/// 用以标记销毁会话是否完成（销毁会话需要一定的时间）
	/// </summary>
	bool bCreateSessionOnDestroy{ false };
	/// <summary>
	/// 用于销毁会话过程中，如果点击了创建会话，则先保存这些信息，等销毁会话完成后，使用该信息创建新的会话
	/// </summary>
	int32 LastNumPublicConnections;
	/// <summary>
	/// 用于销毁会话过程中，如果点击了创建会话，则先保存这些信息，等销毁会话完成后，使用该信息创建新的会话
	/// </summary>
	FString LastMatchType;
};