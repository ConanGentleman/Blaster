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
/// ����������Ϸ��ʼʱ�����Ѫ�����浽��Ļ
/// </summary>
void ABlasterHUD::AddCharacterOverlay()
{
	//Ϊ�˵���CreateWidget����Ҫ��ȡһ����ҿ�����
	APlayerController* PlayerController = GetOwningPlayerController();
	if (PlayerController && CharacterOverlayClass)
	{
		/// <summary>
		/// �����û�����
		/// </summary>
		CharacterOverlay = CreateWidget<UCharacterOverlay>(PlayerController, CharacterOverlayClass);
		/// <summary>
		/// ���û�������ӵ���Ļ
		/// </summary>
		CharacterOverlay->AddToViewport();
	}
}

/// <summary>
/// �����Ϸ��ʼǰ����ʱ����ʾ
/// </summary>
void ABlasterHUD::AddAnnouncement()
{
	APlayerController* PlayerController = GetOwningPlayerController();
	if (PlayerController && AnnouncementClass)
	{
		/// <summary>
		/// ��������ʱ����
		/// </summary>
		Announcement = CreateWidget<UAnnouncement>(PlayerController, AnnouncementClass);
		/// <summary>
		/// ������ʱ������ӵ���Ļ
		/// </summary>
		Announcement->AddToViewport();
	}
}


/// <summary>
/// ��������������ʮ��׼��
/// </summary>
void ABlasterHUD::DrawHUD() {
	Super::DrawHUD();

	FVector2D ViewportSize;
	if (GEngine)
	{
		//��ȡ��Ϸ�ӿڴ�С
		GEngine->GameViewport->GetViewportSize(ViewportSize);
		const FVector2D ViewportCenter(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);

		float SpreadScaled = CrosshairSpreadMax * HUDPackage.CrosshairSpread;

		if (HUDPackage.CrosshairsCenter)
		{
			FVector2D Spread(0.f, 0.f);//���Ĳ���Ҫƫ��
			DrawCrosshair(HUDPackage.CrosshairsCenter, ViewportCenter, Spread, HUDPackage.CrosshairsColor);
		}
		if (HUDPackage.CrosshairsLeft)
		{
			FVector2D Spread(-SpreadScaled, 0.f);//����ƫ��
			DrawCrosshair(HUDPackage.CrosshairsLeft, ViewportCenter, Spread, HUDPackage.CrosshairsColor);
		}
		if (HUDPackage.CrosshairsRight)
		{
			FVector2D Spread(SpreadScaled, 0.f);//����ƫ��
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
/// ����׼�ǵĺ���
/// </summary>
/// <param name="Texture">��ͼ</param>
/// <param name="ViewportCenter">��Ļ����λ��</param>
/// <param name="Spread">׼����ɢ</param>
/// <param name="CrosshairColor">׼����ɫ</param>
void ABlasterHUD::DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter, FVector2D Spread, FLinearColor CrosshairColor) {
	//��ȡ����ĳ���
	const float TextureWidth = Texture->GetSizeX();
	const float TextureHeight = Texture->GetSizeY();
	//��׷λ��
	const FVector2D TextureDrawPoint(
		ViewportCenter.X - (TextureWidth / 2.f) + Spread.X,//λ�ü�����ɢֵ
		ViewportCenter.Y - (TextureHeight / 2.f) + Spread.Y//λ�ü�����ɢֵ
	);
	//��������
	DrawTexture(
		Texture,
		TextureDrawPoint.X,
		TextureDrawPoint.Y,
		TextureWidth,
		TextureHeight,
		0.f, //�����U����
		0.f,//�����V����
		1.f, //����ĸ߶�
		1.f, //����Ŀ��
		CrosshairColor
	);
}