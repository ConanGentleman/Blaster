// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "BlasterHUD.generated.h"

USTRUCT(BlueprintType)
/// <summary>
/// ׼����ͼ�ṹ��
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
	///׼����ɢ
	float CrosshairSpread;
	/// <summary>
	/// ׼����ɫ
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
	/// ���������׼��
	/// </summary>
	virtual void DrawHUD() override;
private:
	/// <summary>
	/// ׼����ͼ
	/// </summary>
	FHUDPackage HUDPackage;

	/// <summary>
	/// ����׼�ǵĺ���
	/// </summary>
	/// <param name="Texture"></param>
	/// <param name="ViewportCenter"></param>
	/// <param name="Spread"></param>
	/// <param name="CrosshairColor">׼����ɫ</param>
	void DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter, FVector2D Spread, FLinearColor CrosshairColor);

	/// <summary>
	/// ׼����ɢ���ֵ��ʵ���������ڵ���׼����ɢ��
	/// </summary>
	UPROPERTY(EditAnywhere)
	float CrosshairSpreadMax = 16.f;
public:
	/// <summary>
	/// ����׼����ͼ
	/// </summary>
	/// <param name="Package"></param>
	FORCEINLINE void SetHUDPackage(const FHUDPackage& Package) { HUDPackage = Package; }
};

