﻿// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BlasterPlayerController.generated.h"

/**
 * 目前的功能：更新HUD的血量信息
 */
UCLASS()
class BLASTER_API ABlasterPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	/// <summary>
	/// 设置HUD中的血量信息
	/// </summary>
	void SetHUDHealth(float Health, float MaxHealth);
	/// <summary>
	/// 设置HUD中的护盾信息
	/// </summary>
	/// <param name="Shield"></param>
	/// <param name="MaxShield"></param>
	void SetHUDShield(float Shield, float MaxShield);
	/// <summary>
	/// 设置HUD中的分数信息
	/// </summary>
	/// <param name="Score"></param>
	void SetHUDScore(float Score);
	/// <summary>
	/// 设置HUD中的死亡信息
	/// </summary>
	/// <param name="Defeats"></param>
	void SetHUDDefeats(int32 Defeats);
	/// <summary>
	/// 设置HUD中的子弹数量
	/// </summary>
	/// <param name="Ammo"></param>
	void SetHUDWeaponAmmo(int32 Ammo);
	/// <summary>
	/// 设置HUD中所携带的子弹数量
	/// </summary>
	/// <param name="Ammo"></param>
	void SetHUDCarriedAmmo(int32 Ammo);
	/// <summary>
	/// 匹配倒计时
	/// </summary>
	/// <param name="CountdownTime"></param>
	void SetHUDMatchCountdown(float CountdownTime);
	/// <summary>
	/// 设置游戏预热时间
	/// </summary>
	/// <param name="CountdownTime"></param>
	void SetHUDAnnouncementCountdown(float CountdownTime);
	/// <summary>
	/// 设置手榴弹数量文本
	/// </summary>
	/// <param name="Grenades"></param>
	void SetHUDGrenades(int32 Grenades);
	/// <summary>
	/// 当一个角色被控制器拥有时，即玩家开始控制该角色时，OnPossess函数就会被调用。（可以用在角色淘汰被重生时使用，因为其发生了控制器对角色控制的分离和新角色的控制
	/// 测试了一下好像刚开始获取的时候并不会调用
	/// </summary>
	/// <param name="InPawn"></param>
	virtual void OnPossess(APawn* InPawn) override;
	/// <summary>
	/// 每帧执行（用于倒计时）
	/// </summary>
	/// <param name="DeltaTime"></param>
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	/// <summary>
	/// 获取服务器时间（与服务器时间同步）
	/// </summary>
	/// <returns></returns>
	virtual float GetServerTime(); 
	virtual void ReceivedPlayer() override; // 尽快同步服务器时间
	/// <summary>
	/// 用于BlasterGameMode类将游戏模式状态设置过来（但只会发生在服务器上，因为只有服务器才有游戏模式）
	/// </summary>
	/// <param name="State"></param>
	void OnMatchStateSet(FName State);
	/// <summary>
	/// 统一处理当倒计时结束（即游戏开始时）的逻辑
	/// </summary>
	void HandleMatchHasStarted();
	/// <summary>
	/// 统一处理当游戏结束时的逻辑
	/// </summary>
	void HandleCooldown();
protected:
	virtual void BeginPlay() override;

	/// <summary>
	/// 倒计时
	/// </summary>
	void SetHUDTime();
	/// <summary>
	/// 初始化HUD信息
	/// </summary>
	void PollInit();
	/**
	* 同步客户端和服务器之间的时间（用于比赛时间显示）
	* 客户端的时间 = 服务器的时间 + （客户端发送请求获取服务器的时间+客户端接收到服务器返回时的时间)/2
	*/

	// 请求当前服务器时间,并传入发送请求时客户端的时间（传入时间在服务器中只用来后续返回给客户端以计算往返时间）
	UFUNCTION(Server, Reliable)
	void ServerRequestServerTime(float TimeOfClientRequest);

	//  服务器响应erverRequestServerTime，将服务器时间发送给客户端
	//  服务器把客户端请求时的时间以及服务器的当前时间发送回客户端
	UFUNCTION(Client, Reliable)
	void ClientReportServerTime(float TimeOfClientRequest, float TimeServerReceivedClientRequest);

	float ClientServerDelta = 0.f; // 客户端当前与服务器的时间差

	/// <summary>
	/// 每隔一段时间，同步一下服务器和客户端的时间及时间增量
	/// </summary>
	UPROPERTY(EditAnywhere, Category = Time)
	float TimeSyncFrequency = 5.f;
	/// <summary>
	/// 距离上一次时间同步的时间计时
	/// </summary>
	float TimeSyncRunningTime = 0.f;
	/// <summary>
	/// 检查是否到同步间隔时间了
	/// </summary>
	/// <param name="DeltaTime"></param>
	void CheckTimeSync(float DeltaTime);

	/// <summary>
	/// RPC函数，获取游戏模式中的各种信息，存储到本类中
	/// </summary>
	UFUNCTION(Server, Reliable)
	void ServerCheckMatchState();

	/// <summary>
	/// RPC函数一旦客户端进入游戏，先调用服务器完成匹配状态检查（ServerCheckMatchState），然后客户端进入游戏，并保存游戏新的相关信息
	/// </summary>
	UFUNCTION(Client, Reliable)
	void ClientJoinMidgame(FName StateOfMatch, float Warmup, float Match, float Cooldown, float StartingTime);

private:
	//角色HUD
	//加上UPROPERTY()的原因是让BlasterHUD初始化为nullptr，即与class ABlasterHUD* BlasterHUD=nullptr相同
	UPROPERTY()
	class ABlasterHUD* BlasterHUD;
	//存储BlasterGameMode类。以便于当角色为服务器上的角色时，直接获取BlasterGameMode中的数据
	UPROPERTY()
	class ABlasterGameMode* BlasterGameMode;

	/// <summary>
	/// 进入关卡的时间（但游戏还未开始
	/// </summary>
	float LevelStartingTime = 0.f;
	/// <summary>
	/// 游戏时间
	/// </summary>
	float MatchTime = 120.f;
	/// <summary>
	/// 游戏开始倒计时时间
	/// </summary>
	float WarmupTime = 0.f;
	/// <summary>
	/// 游戏中场冷却时间
	/// </summary>
	float CooldownTime = 0.f;
	/// <summary>
	/// 倒计时（游戏剩余时间）
	/// </summary>
	uint32 CountdownInt = 0;

	/// <summary>
	/// 保存当前游戏模式状态，用于BlasterGameMode类将游戏模式状态设置过来（但只会发生在服务器上，因为只有服务器才有游戏模式，所以这里要设置为复制变量）
	/// </summary>
	UPROPERTY(ReplicatedUsing = OnRep_MatchState)
	FName MatchState;

	UFUNCTION()
	void OnRep_MatchState();

	/// <summary>
	/// 作为BlasterHUD中的一部分，用于HUD显示的数据类
	/// </summary>
	UPROPERTY()
	class UCharacterOverlay* CharacterOverlay;
	/// <summary>
	/// 下面是用于在HUD还没有效的情况下的缓存数据存储
	/// </summary>
	float HUDHealth;
	/// <summary>
	/// 标记是否初始化完成
	/// </summary>
	bool bInitializeHealth = false;
	float HUDMaxHealth;
	float HUDScore;
	/// <summary>
	/// 标记是否初始化完成
	/// </summary>
	bool bInitializeScore = false;
	int32 HUDDefeats;
	/// <summary>
	/// 标记是否初始化完成
	/// </summary>
	bool bInitializeDefeats = false;
	/// <summary>
	/// 手榴弹数量
	/// </summary>
	int32 HUDGrenades;
	/// <summary>
	/// 标记是否初始化完成
	/// </summary>
	bool bInitializeGrenades = false;
	/// <summary>
	/// 护盾值
	/// </summary>
	float HUDShield;
	/// <summary>
	/// 标记是否初始化完成
	/// </summary>
	bool bInitializeShield = false;
	/// <summary>
	/// 最大护盾值
	/// </summary>
	float HUDMaxShield;
};
