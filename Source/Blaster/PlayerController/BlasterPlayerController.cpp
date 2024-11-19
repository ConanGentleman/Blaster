// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterPlayerController.h"
#include "Blaster/HUD/BlasterHUD.h"
#include "Blaster/HUD/CharacterOverlay.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Net/UnrealNetwork.h"
#include "Blaster/GameMode/BlasterGameMode.h"
#include "Blaster/PlayerState/BlasterPlayerState.h"
#include "Blaster/HUD/Announcement.h"
#include "Kismet/GameplayStatics.h"

void ABlasterPlayerController::BeginPlay()
{
	Super::BeginPlay();

	BlasterHUD = Cast<ABlasterHUD>(GetHUD()); 
	//游戏开始调用服务器状态检查
	ServerCheckMatchState();
}

/// <summary>
/// 重写生命周期复制变量函数，以便于注册复制变量（这里的复制变量是游戏模式状态）
/// </summary>
/// <param name="OutLifetimeProps"></param>
void ABlasterPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABlasterPlayerController, MatchState);
}


void ABlasterPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	//倒计时
	SetHUDTime();
	CheckTimeSync(DeltaTime);
	/// <summary>
	/// 由于倒计时期间没有HUD，但此时已经调用了OnPossess，因此OnPossess里面的并没有正常设置HUDHealth相关的内容
	/// </summary>
	/// <param name="DeltaTime"></param>
	PollInit();
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
/// RPC函数，获取游戏模式中的各种信息，存储到本类中
/// </summary>
void ABlasterPlayerController::ServerCheckMatchState_Implementation()
{
	/// 获取游戏模式
	ABlasterGameMode* GameMode = Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(this));
	if (GameMode)
	{
		//将游戏模式的相关状态，更新到玩家控制器上
		WarmupTime = GameMode->WarmupTime;
		MatchTime = GameMode->MatchTime;
		CooldownTime = GameMode->CooldownTime;
		LevelStartingTime = GameMode->LevelStartingTime;
		//此时由于设置MatchState，使得在客户端的变量接收到服务器的改变时，在客户端上调用的OnRep_MatchState函数
		//但是准备阶段相关的数据没有在OnRep_MatchState中赋值，所以相关的数据还是得走ServerCheckMatchState_Implementation()
		MatchState = GameMode->GetMatchState(); 
		ClientJoinMidgame(MatchState, WarmupTime, MatchTime, CooldownTime, LevelStartingTime);
	}
}

/// <summary>
/// RPC函数一旦客户端进入游戏，先调用服务器完成匹配状态检查（ServerCheckMatchState），然后客户端进入游戏，并保存游戏新的相关信息
/// </summary>
void ABlasterPlayerController::ClientJoinMidgame_Implementation(FName StateOfMatch, float Warmup, float Match, float Cooldown, float StartingTime)
{
	WarmupTime = Warmup;
	MatchTime = Match;
	CooldownTime = Cooldown;
	LevelStartingTime = StartingTime;
	MatchState = StateOfMatch;
	OnMatchStateSet(MatchState);
	if (BlasterHUD && MatchState == MatchState::WaitingToStart)
	{
		/// <summary>
		/// 添加游戏开始倒计时到屏幕
		/// </summary>
		BlasterHUD->AddAnnouncement();
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
	else //没有满足就现缓存下来，等HUD被生成后设置
	{
		bInitializeCharacterOverlay = true;
		HUDHealth = Health;
		HUDMaxHealth = MaxHealth;
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
	else
	{
		bInitializeCharacterOverlay = true;
		HUDScore = Score;
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
	else //没有HUD就现缓存下来，等HUD被生成后设置
	{
		bInitializeCharacterOverlay = true;
		HUDDefeats = Defeats;
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
		BlasterHUD->CharacterOverlay->HealthBar &&
		BlasterHUD->CharacterOverlay->MatchCountdownText;
	if (bHUDValid)
	{
		if (CountdownTime < 0.f) //出现负数时先不显示，设置为空文本
		{
			BlasterHUD->CharacterOverlay->MatchCountdownText->SetText(FText());
			return;
		}

		int32 Minutes = FMath::FloorToInt(CountdownTime / 60.f);
		int32 Seconds = CountdownTime - Minutes * 60;

		FString CountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		BlasterHUD->CharacterOverlay->MatchCountdownText->SetText(FText::FromString(CountdownText));
	}
}

/// <summary>
/// 设置游戏预热时间
/// </summary>
/// <param name="CountdownTime"></param>
void ABlasterPlayerController::SetHUDAnnouncementCountdown(float CountdownTime)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD &&
		BlasterHUD->Announcement &&
		BlasterHUD->Announcement->WarmupTime;
	if (bHUDValid)
	{
		if (CountdownTime < 0.f) //出现负数时先不显示，设置为空文本
		{
			BlasterHUD->Announcement->WarmupTime->SetText(FText());
			return;
		}
		int32 Minutes = FMath::FloorToInt(CountdownTime / 60.f);
		int32 Seconds = CountdownTime - Minutes * 60;

		FString CountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		BlasterHUD->Announcement->WarmupTime->SetText(FText::FromString(CountdownText));
	}
}

/// <summary>
/// 倒计时
/// </summary>
void ABlasterPlayerController::SetHUDTime()
{
	float TimeLeft = 0.f;
	//预热倒计时和游戏时间倒计时分开计算
	if (MatchState == MatchState::WaitingToStart) TimeLeft = WarmupTime - GetServerTime() + LevelStartingTime;
	else if (MatchState == MatchState::InProgress) TimeLeft = WarmupTime + MatchTime - GetServerTime() + LevelStartingTime;
	else if (MatchState == MatchState::Cooldown) TimeLeft = CooldownTime + WarmupTime + MatchTime - GetServerTime() + LevelStartingTime;
	// GetWorld()->GetTimeSeconds()获取自比赛以来所经过的时间
	uint32 SecondsLeft = FMath::CeilToInt(TimeLeft);

	////如果自身是服务器则直接获取倒计时
	//if (HasAuthority())
	//{
	//	if (BlasterGameMode == nullptr)
	//	{
	//		BlasterGameMode = Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(this));
	//		//LevelStartingTime = BlasterGameMode->LevelStartingTime;
	//	}
	//	BlasterGameMode = BlasterGameMode == nullptr ? Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(this)) : BlasterGameMode;
	//	if (BlasterGameMode)
	//	{
	//		SecondsLeft = FMath::CeilToInt(BlasterGameMode->GetCountdownTime() + LevelStartingTime);
	//	}
	//}
	
	if (CountdownInt != SecondsLeft)
	{
		if (MatchState == MatchState::WaitingToStart || MatchState == MatchState::Cooldown)
		{
			SetHUDAnnouncementCountdown(TimeLeft);
		}
		if (MatchState == MatchState::InProgress)
		{
			SetHUDMatchCountdown(TimeLeft);
		}
	}

	CountdownInt = SecondsLeft;
}

void ABlasterPlayerController::PollInit()
{
	if (CharacterOverlay == nullptr)
	{
		if (BlasterHUD && BlasterHUD->CharacterOverlay)
		{
			CharacterOverlay = BlasterHUD->CharacterOverlay;
			if (CharacterOverlay)
			{
				SetHUDHealth(HUDHealth, HUDMaxHealth);
				SetHUDScore(HUDScore);
				SetHUDDefeats(HUDDefeats);
			}
		}
	}
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


void ABlasterPlayerController::OnMatchStateSet(FName State)
{
	MatchState = State;

	if (MatchState == MatchState::InProgress) //如果是游戏模式是进行中状态
	{
		HandleMatchHasStarted();
	}
	else if (MatchState == MatchState::Cooldown) ///如果是游戏模式是游戏冷却状态
	{
		HandleCooldown();
	}
}

/// <summary>
/// 游戏模式状态复制变量被设置后的回调
/// </summary>
void ABlasterPlayerController::OnRep_MatchState()
{
	if (MatchState == MatchState::InProgress) //如果是游戏模式是进行中状态
	{
		HandleMatchHasStarted();
	}
	else if (MatchState == MatchState::Cooldown) ///如果是游戏模式是游戏冷却状态
	{
		HandleCooldown();
	}
}

/// <summary>
/// 统一处理当倒计时结束（即游戏开始时）的逻辑
/// </summary>
void ABlasterPlayerController::HandleMatchHasStarted()
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	if (BlasterHUD)
	{
		//当游戏模式状态为进行中时，才显示玩家的HUD界面
		BlasterHUD->AddCharacterOverlay();
		if (BlasterHUD->Announcement)
		{
			//当游戏模式状态为进行中时，设置倒计时显示为隐藏状态
			BlasterHUD->Announcement->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

/// <summary>
/// 统一处理当游戏结束时的逻辑
/// </summary>
void ABlasterPlayerController::HandleCooldown()
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	if (BlasterHUD)
	{
		//删除玩家游戏信息界面
		BlasterHUD->CharacterOverlay->RemoveFromParent();
		bool bHUDValid = BlasterHUD->Announcement &&
			BlasterHUD->Announcement->AnnouncementText &&
			BlasterHUD->Announcement->InfoText;
		if (bHUDValid)
		{
			//开启冷却倒计时界面（与游戏预热倒计时共用一个界面）
			BlasterHUD->Announcement->SetVisibility(ESlateVisibility::Visible);
			//调整文字显示
			FString AnnouncementText("New Match Starts In:");
			BlasterHUD->Announcement->AnnouncementText->SetText(FText::FromString(AnnouncementText));
			BlasterHUD->Announcement->InfoText->SetText(FText());
		}
	}
}