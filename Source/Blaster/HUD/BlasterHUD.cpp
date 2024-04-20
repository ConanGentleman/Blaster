// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterHUD.h"

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
			DrawCrosshair(HUDPackage.CrosshairsCenter, ViewportCenter, Spread);
		}
		if (HUDPackage.CrosshairsLeft)
		{
			FVector2D Spread(-SpreadScaled, 0.f);//����ƫ��
			DrawCrosshair(HUDPackage.CrosshairsLeft, ViewportCenter, Spread);
		}
		if (HUDPackage.CrosshairsRight)
		{
			FVector2D Spread(SpreadScaled, 0.f);//����ƫ��
			DrawCrosshair(HUDPackage.CrosshairsRight, ViewportCenter, Spread);
		}
		if (HUDPackage.CrosshairsTop)
		{
			FVector2D Spread(0.f, -SpreadScaled);
			DrawCrosshair(HUDPackage.CrosshairsTop, ViewportCenter, Spread);
		}
		if (HUDPackage.CrosshairsBottom)
		{
			FVector2D Spread(0.f, SpreadScaled);
			DrawCrosshair(HUDPackage.CrosshairsBottom, ViewportCenter, Spread);  
		}
	}
}
/// <summary>
/// ����׼�ǵĺ���
/// </summary>
/// <param name="Texture">��ͼ</param>
/// <param name="ViewportCenter">��Ļ����λ��</param>
/// <param name="Spread">׼����ɢ</param>
void ABlasterHUD::DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter, FVector2D Spread)
{
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
		FLinearColor::White
	);
}