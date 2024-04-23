// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CharacterOverlay.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API UCharacterOverlay : public UUserWidget
{
	GENERATED_BODY()
	
public:
	/// <summary>
	/// 血量进度条
	/// </summary>
	UPROPERTY(meta = (BindWidget))
	class UProgressBar* HealthBar;

	/// <summary>
	/// 血量文字
	/// </summary>
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* HealthText;
};
