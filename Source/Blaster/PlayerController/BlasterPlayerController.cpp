﻿// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterPlayerController.h"
#include "Blaster/HUD/BlasterHUD.h"
#include "Blaster/HUD/CharacterOverlay.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Blaster/Character/BlasterCharacter.h"

void ABlasterPlayerController::BeginPlay()
{
	Super::BeginPlay();

	BlasterHUD = Cast<ABlasterHUD>(GetHUD()); 
}

void ABlasterPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	//倒计时
	SetHUDTime();
	CheckTimeSync(DeltaTime);
}
/// <summary>
/// 检查是否该同步时间了
/// </summary>
/// <param name="DeltaTime"></param>
void ABlasterPlayerController::CheckTimeSync(float DeltaTime)
{
	TimeSyncRunningTime += DeltaTime;
	if (IsLocalController() && TimeSyncRunningTime > TimeSyncFrequency)
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
		TimeSyncRunningTime = 0.f;
	}
}

/// <summary>
/// 当一个角色被控制器拥有时，即玩家开始控制该角色时，OnPossess函数就会被调用。（可以用在角色淘汰被重生时使用，因为其发生了控制器对角色控制的分离和新角色的控制
/// </summary>
/// <param name="InPawn">被控制器控制的棋子</param>
void ABlasterPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(InPawn);
	if (BlasterCharacter)
	{
		//棋子被开始控制时，更新血量信息
		SetHUDHealth(BlasterCharacter->GetHealth(), BlasterCharacter->GetMaxHealth());
	}
}

void ABlasterPlayerController::SetHUDHealth(float Health, float MaxHealth)
{
	//确保BlasterHUD不为空
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->HealthBar &&
		BlasterHUD->CharacterOverlay->HealthText;
	//满足条件设置血量进度条和文字
	if (bHUDValid)
	{
		const float HealthPercent = Health / MaxHealth;
		//设置进度条
		BlasterHUD->CharacterOverlay->HealthBar->SetPercent(HealthPercent);
		FString HealthText = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Health), FMath::CeilToInt(MaxHealth));
		//设置血量文字
		BlasterHUD->CharacterOverlay->HealthText->SetText(FText::FromString(HealthText));
	}
}

void ABlasterPlayerController::SetHUDScore(float Score)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->ScoreAmount;
	if (bHUDValid)
	{
		FString ScoreText = FString::Printf(TEXT("%d"), FMath::FloorToInt(Score));
		//设置分数
		BlasterHUD->CharacterOverlay->ScoreAmount->SetText(FText::FromString(ScoreText));
	}
}

void ABlasterPlayerController::SetHUDDefeats(int32 Defeats)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->DefeatsAmount;
	if (bHUDValid)
	{
		FString DefeatsText = FString::Printf(TEXT("%d"), Defeats);
		//设置死亡数
		BlasterHUD->CharacterOverlay->DefeatsAmount->SetText(FText::FromString(DefeatsText));
	}
}

void ABlasterPlayerController::SetHUDWeaponAmmo(int32 Ammo)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->WeaponAmmoAmount;
	if (bHUDValid)
	{
		FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
		BlasterHUD->CharacterOverlay->WeaponAmmoAmount->SetText(FText::FromString(AmmoText));
	}
}

/// <summary>
/// 设置角色所携带的子弹量
/// </summary>
/// <param name="Ammo"></param>
void ABlasterPlayerController::SetHUDCarriedAmmo(int32 Ammo)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->CarriedAmmoAmount;
	if (bHUDValid)
	{
		FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
		BlasterHUD->CharacterOverlay->CarriedAmmoAmount->SetText(FText::FromString(AmmoText));
	}
}

/// <summary>
/// 设置倒计时文本
/// </summary>
/// <param name="CountdownTime"></param>
void ABlasterPlayerController::SetHUDMatchCountdown(float CountdownTime)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->MatchCountdownText;
	if (bHUDValid)
	{
		int32 Minutes = FMath::FloorToInt(CountdownTime / 60.f);
		int32 Seconds = CountdownTime - Minutes * 60;

		FString CountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		BlasterHUD->CharacterOverlay->MatchCountdownText->SetText(FText::FromString(CountdownText));
	}
}
/// <summary>
/// 倒计时
/// </summary>
void ABlasterPlayerController::SetHUDTime()
{
	// GetWorld()->GetTimeSeconds()获取自比赛以来所经过的时间
	uint32 SecondsLeft = FMath::CeilToInt(MatchTime - GetServerTime());
	if (CountdownInt != SecondsLeft)
	{
		
		SetHUDMatchCountdown(MatchTime - GetServerTime());
	}

	CountdownInt = SecondsLeft;
}

/// <summary>
/// 请求当前服务器时间,并传入发送请求时客户端的时间
/// </summary>
/// <param name="TimeOfClientRequest">传入时间在服务器中只用来后续返回给客户端以计算往返时间</param>
void ABlasterPlayerController::ServerRequestServerTime_Implementation(float TimeOfClientRequest)
{
	float ServerTimeOfReceipt = GetWorld()->GetTimeSeconds();//获取服务器当前时间
	ClientReportServerTime(TimeOfClientRequest, ServerTimeOfReceipt);
}

/// <summary>
/// 服务器响应erverRequestServerTime，将服务器时间发送给客户端
/// 服务器把客户端请求时的时间以及服务器的当前时间发送回客户端
/// </summary>
/// <param name="TimeOfClientRequest">客户端请求时的时间</param>
/// <param name="TimeServerReceivedClientRequest">服务器的当前时间</param>
void ABlasterPlayerController::ClientReportServerTime_Implementation(float TimeOfClientRequest, float TimeServerReceivedClientRequest)
{
	//计算客户端从请求获取时间到接收到返回结果的总时间
	float RoundTripTime = GetWorld()->GetTimeSeconds() - TimeOfClientRequest;
	//客户端的时间 = 服务器的时间 + （客户端发送请求获取服务器的时间+客户端接收到服务器返回时的时间)/2
	float CurrentServerTime = TimeServerReceivedClientRequest + (0.5f * RoundTripTime);
	//保存服务器时间和客户端时间的时间差
	ClientServerDelta = CurrentServerTime - GetWorld()->GetTimeSeconds();
}

float ABlasterPlayerController::GetServerTime()
{
	if (HasAuthority()) return GetWorld()->GetTimeSeconds(); //如果当前是服务器则直接返回
	else return GetWorld()->GetTimeSeconds() + ClientServerDelta; //否则，当前服务器时间= 客户端时间+时间差
}

/// <summary>
/// 在此 PlayerController 的视区/网络连接与此播放器控制器关联后调用。
/// </summary>
void ABlasterPlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();
	if (IsLocalController()) //是否是本地玩家控制
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds()); //将请求发送到服务器，以便计算增量
	}
}