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
	//UPROPERTY(meta = (BindWidget)) �Ϳ��԰���ͼ�е�ͬ�����
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* DisplayText;
	//�����ı�����
	void SetDisplayText(FString TextToDisplay);
	//��ʾ��������ɫENetRole���ͣ�ͨ������Ĳ�����ȡrole�����ڽ�ɫ��ͼ�е���
	UFUNCTION(BlueprintCallable)
	void ShowPlayerNetRole(APawn* InPawn);
protected:
	//�����ɵ���ͬ�Ĺؿ����뿪�ؿ�ʱ����
	virtual void OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld) override;

};
