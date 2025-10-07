// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "BlasterHUD.generated.h"

USTRUCT(BlueprintType)
/// <summary>
/// 准星贴图结构体
/// </summary>
struct FHUDPackage
{
	GENERATED_BODY()
public:
	class UTexture2D* CrosshairsCenter;
	UTexture2D* CrosshairsLeft;
	UTexture2D* CrosshairsRight;
	UTexture2D* CrosshairsTop;
	UTexture2D* CrosshairsBottom;
	///准星扩散
	float CrosshairSpread;
	/// <summary>
	/// 准星颜色
	/// </summary>
	FLinearColor CrosshairsColor;
};

/**
 * 
 */
UCLASS()
class BLASTER_API ABlasterHUD : public AHUD
{
	GENERATED_BODY()
public:
	/// <summary>
	/// 在这里绘制准星
	/// </summary>
	virtual void DrawHUD() override;

	/// <summary>
	/// 血量显示的用户界面
	/// </summary>
	UPROPERTY(EditAnywhere, Category = "Player Stats")
	TSubclassOf<class UUserWidget> CharacterOverlayClass;

	/// <summary>
	/// （用于在游戏开始时）添加血量界面到屏幕
	/// </summary>
	void AddCharacterOverlay();
	/// <summary>
	/// 角色各项信息的显示
	/// </summary>
	UPROPERTY()
	class UCharacterOverlay* CharacterOverlay;

	/// <summary>
	/// 游戏开始倒计时前的用户界面
	/// </summary>
	UPROPERTY(EditAnywhere, Category = "Announcements")
	TSubclassOf<UUserWidget> AnnouncementClass;

	/// <summary>
	/// 游戏开始倒计时前各项信息的显示
	/// </summary>
	UPROPERTY()
	class UAnnouncement* Announcement;

	void AddAnnouncement();

	/// <summary>
	/// 添加玩家被击杀的公告
	/// </summary>
	/// <param name="Attacker"></param>
	/// <param name="Victim"></param>
	void AddElimAnnouncement(FString Attacker, FString Victim);

protected:
	virtual void BeginPlay() override;
private:

	UPROPERTY()
	class APlayerController* OwningPlayer;


	/// <summary>
	/// 准星贴图
	/// </summary>
	FHUDPackage HUDPackage;

	/// <summary>
	/// 绘制准星的函数
	/// </summary>
	/// <param name="Texture"></param>
	/// <param name="ViewportCenter"></param>
	/// <param name="Spread"></param>
	/// <param name="CrosshairColor">准星颜色</param>
	void DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter, FVector2D Spread, FLinearColor CrosshairColor);

	/// <summary>
	/// 准星扩散最大值（实际上是用于调整准星扩散的
	/// </summary>
	UPROPERTY(EditAnywhere)
	float CrosshairSpreadMax = 16.f;

	/// <summary>
	/// 死亡通知类
	/// </summary>
	UPROPERTY(EditAnywhere)
	TSubclassOf<class UElimAnnouncement> ElimAnnouncementClass;
public:
	/// <summary>
	/// 设置准星贴图
	/// </summary>
	/// <param name="Package"></param>
	FORCEINLINE void SetHUDPackage(const FHUDPackage& Package) { HUDPackage = Package; }
};

