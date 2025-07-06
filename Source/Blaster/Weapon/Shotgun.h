// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HitScanWeapon.h"
#include "Shotgun.generated.h"

/**
 * 霰弹枪
 */
UCLASS()
class BLASTER_API AShotgun : public AHitScanWeapon
{
	GENERATED_BODY()
public:
	/// <summary>
	/// 重写开火函数
	/// </summary>
	virtual void Fire(const FVector& HitTarget) override; 
private:
	/// <summary>
	/// 霰弹数量（每发）
	/// </summary>
	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	uint32 NumberOfPellets = 10;
};