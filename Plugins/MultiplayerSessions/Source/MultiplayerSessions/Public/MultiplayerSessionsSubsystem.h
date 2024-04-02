// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"

#include "MultiplayerSessionsSubsystem.generated.h"

//
// �����Լ����Զ���ί�����ڽ��ص��󶨵��˵���
//���ಥMULTICAST ��ζ��һ���㲥��������ܹ����亯���󶨵���ί�У�
//����̬DYNAMIC ��ζ�ſ������л�ί�У����ҿ��Դ���ͼ�б�����߼�������
//ֵ��ע�������Ϊ���Ƕ�̬�ģ���ζ�Ű󶨵����ĺ���������UFUNTION����Ȼ�޷��󶨳ɹ�
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiplayerOnCreateSessionComplete, bool, bWasSuccessful);
///
/// ���߻Ự�����������һ���µ�����߽ṹ�壬�����Ҫ��̬�ಥί�У����е����Ͷ���������ͼ���ݡ���ˣ����߻Ự���������������Ϊ��̬ί��
/// ��ʹ����һ�ֶಥί��,ԭ����ǻỰ��������޷�����ͼ���ݡ�ͬʱ��ί���붯̬ί���ڲ����ϻ��в�ͬ�������ͺ�����֮���Ƿ��ж��š�
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
	// ����Ự���ܡ� �˵�����Ե�������
	//
	
	/// <summary>
	/// �����Ự
	/// </summary>
	/// <param name="NumPublicConnections">�������</param>
	/// <param name="MatchType">ƥ������</param>
	void CreateSession(int32 NumPublicConnections,FString MatchType);
	/// <summary>
	/// �����Ự
	/// </summary>
	/// <param name="MaxSearchResults">��������������</param>
	void FindSessions(int32 MaxSearchResults);
	/// <summary>
	/// ����Ự
	/// </summary>
	/// <param name="SessionResult"></param>
	void JoinSession(const FOnlineSessionSearchResult& SessionResult);
	/// <summary>
	/// ���ٻỰ
	/// </summary>
	void DestroySessions();
	/// <summary>
	/// ��ʼ�Ự
	/// </summary>
	void StartSession();

	/// <summary>
	/// ��������Ϊmenu��󶨻ص����Զ���ί��
	/// </summary>
	FMultiplayerOnCreateSessionComplete MultiplayerOnCreateSessionComplete;
	FMultiplayerOnFindSessionComplete MultiplayerOnFindSessionComplete;
	FMultiplayerOnJoinSessionComplete MultiplayerOnJoinSessionComplete;
	FMultiplayerOnDestroySessionComplete MultiplayerOnDestroySessionComplete;
	FMultiplayerOnStartSessionComplete MultiplayerOnStartSessionComplete;

protected:

	//
	// ����ί�е��ڲ��ص�����ӵ�OnlineSessionInterface��ί���б�
	// ��Щ����Ҫ���������
	//

	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnFindSessionsComplete(bool bWasSuccessful);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);
	void OnStartSessionComplete(FName SessionName, bool bWasSuccessful);

private:
	/// <summary>
	/// �Ự�ӿ�
	/// </summary>
	IOnlineSessionPtr SessionInterface;
	//�洢���ĻỰ���ã���Ϊ�ٴ����Ựʱ�������ڻỰ������������ٴ��������ã����������ĻỰ��
	TSharedPtr<FOnlineSessionSettings> LastSessionSettings;
	/// <summary>
	/// �Ự����
	/// </summary>
	TSharedPtr<FOnlineSessionSearch> LastSessionSearch;

	//
	// ��ӵ����߻Ự�ӿ�ί���б�
	// ��MultiplayerSessionsSubsystem �ڲ��ص���������
	// ��ps:�������һЩί��

	/// <summary>
	/// �Ự����ί��
	/// </summary>
	FOnCreateSessionCompleteDelegate CreateSessionCompleteDelegate;
	//�����������ʶ��ͬ�Ĵ��������, ��һ�� uint64 ��ID�����շ��ص�ί�о���� ����ί�е�ɾ��
	FDelegateHandle CreateSessionCompleteDelegateHandle;
	/// <summary>
	/// �Ự����ί��
	/// </summary>
	FOnFindSessionsCompleteDelegate FindSessionsCompleteDelegate;
	//���
	FDelegateHandle FindSessionsCompleteDelegateHandle;
	/// <summary>
	/// �Ự����ί��
	/// </summary>
	FOnJoinSessionCompleteDelegate JoinSessionCompleteDelegate;
	//���
	FDelegateHandle JoinSessionCompleteDelegateHandle;
	/// <summary>
	/// �Ự����ί��
	/// </summary>
	FOnDestroySessionCompleteDelegate DestroySessionCompleteDelegate;
	//���
	FDelegateHandle DestroySessionCompleteDelegateHandle;
	/// <summary>
	/// �Ự��ʼί��
	/// </summary>
	FOnStartSessionCompleteDelegate StartSessionCompleteDelegate;
	//���
	FDelegateHandle StartSessionCompleteDelegateHandle;

	/// <summary>
	/// ���Ա�����ٻỰ�Ƿ���ɣ����ٻỰ��Ҫһ����ʱ�䣩
	/// </summary>
	bool bCreateSessionOnDestroy{ false };
	/// <summary>
	/// �������ٻỰ�����У��������˴����Ự�����ȱ�����Щ��Ϣ�������ٻỰ��ɺ�ʹ�ø���Ϣ�����µĻỰ
	/// </summary>
	int32 LastNumPublicConnections;
	/// <summary>
	/// �������ٻỰ�����У��������˴����Ự�����ȱ�����Щ��Ϣ�������ٻỰ��ɺ�ʹ�ø���Ϣ�����µĻỰ
	/// </summary>
	FString LastMatchType;
};