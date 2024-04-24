// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CharacterOverlay.generated.h"

/**
 * 该类作为BlasterHUD中的一部分
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

	/// <summary>
	/// 分数文字
	/// </summary>
	UPROPERTY(meta = (BindWidget))
	UTextBlock* ScoreAmount;

	/// <summary>
	/// 死亡次数文字
	/// </summary>
	UPROPERTY(meta = (BindWidget))
	UTextBlock* DefeatsAmount;

	/// <summary>
	/// 子弹文字
	/// </summary>
	UPROPERTY(meta = (BindWidget))
	UTextBlock* WeaponAmmoAmount;

	/// <summary>
	/// 所携带的子弹数量（角色自己的弹匣）
	/// </summary>
	UPROPERTY(meta = (BindWidget))
	UTextBlock* CarriedAmmoAmount;
};
