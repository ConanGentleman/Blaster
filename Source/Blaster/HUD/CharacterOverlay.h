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
	/// 护盾进度条
	/// </summary>
	UPROPERTY(meta = (BindWidget))
	UProgressBar* ShieldBar;

	/// <summary>
	/// 护盾文本
	/// </summary>
	UPROPERTY(meta = (BindWidget))
	UTextBlock* ShieldText;

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

	/// <summary>
	/// 匹配倒计时文字
	/// </summary>
	UPROPERTY(meta = (BindWidget))
	UTextBlock* MatchCountdownText;

	/// <summary>
	/// 手榴弹数量文本
	/// </summary>
	UPROPERTY(meta = (BindWidget))
	UTextBlock* GrenadesText;

	/// <summary>
	/// 高ping图片
	/// </summary>
	UPROPERTY(meta = (BindWidget))
	class UImage* HighPingImage;

	/// <summary>
	/// 高ping动画
	/// BindWidgetAnim表示绑定的是动画
	/// Transient 这是一个属性说明符，表示该变量是 临时的，不会被保存（序列化）。 通常用于运行时动态生成的变量，或者不需要持久化的数据。例如，动画实例可能在运行时动态创建，不需要保存到磁盘。
	/// </summary>
	UPROPERTY(meta = (BindWidgetAnim), Transient)
	UWidgetAnimation* HighPingAnimation;
};
