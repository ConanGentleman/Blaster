// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BlasterPlayerController.generated.h"

/**
 * 目前的功能：更新HUD的血量信息
 */
UCLASS()
class BLASTER_API ABlasterPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	/// <summary>
	/// 设置HUD中的血量信息
	/// </summary>
	void SetHUDHealth(float Health, float MaxHealth);
protected:
	virtual void BeginPlay() override;

private:
	//角色HUD
	class ABlasterHUD* BlasterHUD;
	
};
