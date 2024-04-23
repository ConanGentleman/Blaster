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
	/// <summary>
	/// 设置HUD中的分数信息
	/// </summary>
	/// <param name="Score"></param>
	void SetHUDScore(float Score);
	/// <summary>
	/// 当一个角色被控制器拥有时，即玩家开始控制该角色时，OnPossess函数就会被调用。（可以用在角色淘汰被重生时使用，因为其发生了控制器对角色控制的分离和新角色的控制
	/// 测试了一下好像刚开始获取的时候并不会调用
	/// </summary>
	/// <param name="InPawn"></param>
	virtual void OnPossess(APawn* InPawn) override;
protected:
	virtual void BeginPlay() override;

private:
	//角色HUD
	class ABlasterHUD* BlasterHUD;
	
};
