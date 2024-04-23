// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterPlayerController.h"

#include "Blaster/HUD/BlasterHUD.h"
#include "Blaster/HUD/CharacterOverlay.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"


void ABlasterPlayerController::BeginPlay()
{
	Super::BeginPlay();

	BlasterHUD = Cast<ABlasterHUD>(GetHUD());
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