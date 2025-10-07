// Fill out your copyright notice in the Description page of Project Settings.


#include "ElimAnnouncement.h"
#include "Components/TextBlock.h"

/// <summary>
/// 某个玩家死亡时的通知文本（AttackerName击杀了VictimName
/// </summary>
/// <param name="AttackerName">击杀者</param>
/// <param name="VictimName">被击杀者</param>
void UElimAnnouncement::SetElimAnnouncementText(FString AttackerName, FString VictimName)
{
	FString ElimAnnouncementText = FString::Printf(TEXT("%s elimmed %s!"), *AttackerName, *VictimName);
	if (AnnouncementText)
	{
		AnnouncementText->SetText(FText::FromString(ElimAnnouncementText));
	}
}