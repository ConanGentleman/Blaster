// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OverheadWidget.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API UOverheadWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	//UPROPERTY(meta = (BindWidget)) 就可以绑定蓝图中的同名组件
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* DisplayText;
	//设置文本内容
	void SetDisplayText(FString TextToDisplay);
	//显示玩家网络角色ENetRole类型，通过传入的参数获取role。并在角色蓝图中调用
	UFUNCTION(BlueprintCallable)
	void ShowPlayerNetRole(APawn* InPawn);
protected:
	//当过渡到不同的关卡或离开关卡时调用
	virtual void OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld) override;

};
