// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterHUD.h"
#include "GameFramework/PlayerController.h"
#include "CharacterOverlay.h"
#include "Announcement.h"

void ABlasterHUD::BeginPlay()
{
	Super::BeginPlay();
}
/// <summary>
/// （用于在游戏开始时）添加血量界面到屏幕
/// </summary>
void ABlasterHUD::AddCharacterOverlay()
{
	//为了调用CreateWidget，需要获取一个玩家控制器
	APlayerController* PlayerController = GetOwningPlayerController();
	if (PlayerController && CharacterOverlayClass)
	{
		/// <summary>
		/// 创捷用户界面
		/// </summary>
		CharacterOverlay = CreateWidget<UCharacterOverlay>(PlayerController, CharacterOverlayClass);
		/// <summary>
		/// 将用户界面添加到屏幕
		/// </summary>
		CharacterOverlay->AddToViewport();
	}
}

/// <summary>
/// 添加游戏开始前倒计时的显示
/// </summary>
void ABlasterHUD::AddAnnouncement()
{
	APlayerController* PlayerController = GetOwningPlayerController();
	if (PlayerController && AnnouncementClass)
	{
		/// <summary>
		/// 创建倒计时界面
		/// </summary>
		Announcement = CreateWidget<UAnnouncement>(PlayerController, AnnouncementClass);
		/// <summary>
		/// 将倒计时界面添加到屏幕
		/// </summary>
		Announcement->AddToViewport();
	}
}


/// <summary>
/// 在这里绘制射击的十字准星
/// </summary>
void ABlasterHUD::DrawHUD() {
	Super::DrawHUD();

	FVector2D ViewportSize;
	if (GEngine)
	{
		//获取游戏视口大小
		GEngine->GameViewport->GetViewportSize(ViewportSize);
		const FVector2D ViewportCenter(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);

		float SpreadScaled = CrosshairSpreadMax * HUDPackage.CrosshairSpread;

		if (HUDPackage.CrosshairsCenter)
		{
			FVector2D Spread(0.f, 0.f);//中心不需要偏移
			DrawCrosshair(HUDPackage.CrosshairsCenter, ViewportCenter, Spread, HUDPackage.CrosshairsColor);
		}
		if (HUDPackage.CrosshairsLeft)
		{
			FVector2D Spread(-SpreadScaled, 0.f);//向左偏移
			DrawCrosshair(HUDPackage.CrosshairsLeft, ViewportCenter, Spread, HUDPackage.CrosshairsColor);
		}
		if (HUDPackage.CrosshairsRight)
		{
			FVector2D Spread(SpreadScaled, 0.f);//向右偏移
			DrawCrosshair(HUDPackage.CrosshairsRight, ViewportCenter, Spread, HUDPackage.CrosshairsColor);
		}
		if (HUDPackage.CrosshairsTop)
		{
			FVector2D Spread(0.f, -SpreadScaled);
			DrawCrosshair(HUDPackage.CrosshairsTop, ViewportCenter, Spread, HUDPackage.CrosshairsColor);
		}
		if (HUDPackage.CrosshairsBottom)
		{
			FVector2D Spread(0.f, SpreadScaled);
			DrawCrosshair(HUDPackage.CrosshairsBottom, ViewportCenter, Spread, HUDPackage.CrosshairsColor);
		}
	}
}
/// <summary>
/// 绘制准星的函数
/// </summary>
/// <param name="Texture">贴图</param>
/// <param name="ViewportCenter">屏幕中心位置</param>
/// <param name="Spread">准星扩散</param>
/// <param name="CrosshairColor">准星颜色</param>
void ABlasterHUD::DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter, FVector2D Spread, FLinearColor CrosshairColor) {
	//获取纹理的长宽
	const float TextureWidth = Texture->GetSizeX();
	const float TextureHeight = Texture->GetSizeY();
	//回追位置
	const FVector2D TextureDrawPoint(
		ViewportCenter.X - (TextureWidth / 2.f) + Spread.X,//位置加上扩散值
		ViewportCenter.Y - (TextureHeight / 2.f) + Spread.Y//位置加上扩散值
	);
	//绘制纹理
	DrawTexture(
		Texture,
		TextureDrawPoint.X,
		TextureDrawPoint.Y,
		TextureWidth,
		TextureHeight,
		0.f, //纹理的U坐标
		0.f,//纹理的V坐标
		1.f, //纹理的高度
		1.f, //纹理的宽度
		CrosshairColor
	);
}