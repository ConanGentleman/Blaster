// Fill out your copyright notice in the Description page of Project Settings.


#include "OverheadWidget.h"
#include "Components/TextBlock.h"
/// <summary>
/// 设置文本内容
/// </summary>
/// <param name="TextToDisplay"></param>
void UOverheadWidget::SetDisplayText(FString TextToDisplay)
{
	if (DisplayText) {
		DisplayText->SetText(FText::FromString(TextToDisplay));
	}
}
void UOverheadWidget::ShowPlayerNetRole(APawn* InPawn)
{
	//GetLocalRole或者GetRemoteRole会根据调用它的机器的不同导致返回不同的结果
	ENetRole RemoteRole = InPawn->GetRemoteRole();
	FString Role;
	switch (RemoteRole) {//相关类型和说明 查看：https://blog.csdn.net/m0_53278265/article/details/129503448
	case ENetRole::ROLE_Authority: //在本地网络角色GetLocalRole()中，服务器上存在的所有Pawn都将为Authority
		Role = FString("Authority"); //在远程网络角色RemoteRole()中，则相反，服务器上的所控制的角色为Autonomous Proxy，而所看到的其他角色为ROLE_SimulatedProxy
		break;
	case ENetRole::ROLE_AutonomousProxy://在本地网络角色中，客户端上的所控制的角色为Autonomous Proxy，而客户端所看到的其他角色为ROLE_SimulatedProxy
		Role = FString("Autonomous Proxy");//在远程网络角色RemoteRole()中，则相反，客户端上存在的所有Pawn都为Authority
		break;
	case ENetRole::ROLE_SimulatedProxy:
		Role = FString("Simulated Proxy");
		break;
	case ENetRole::ROLE_None:
		Role = FString("None");
		break;
	}//通过上述方式可以确定服务器是哪一个
	FString RemoteRoleString = FString::Printf(TEXT("Remote Role: %s"), *Role);
	SetDisplayText(RemoteRoleString);

	///下面两行是用于显示玩家姓名的，与原有内容无关。或者说是43的可选挑战
	FString Name = InPawn->GetName();
	SetDisplayText(Name);
	///
}
/// <summary>
/// 当过渡到不同的关卡或离开关卡时调用
/// </summary>
/// <param name="InLevel"></param>
/// <param name="InWorld"></param>
void UOverheadWidget::OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld)
{
	//从视口删除
	RemoveFromParent();
	Super::OnLevelRemovedFromWorld(InLevel, InWorld);
}
