// Fill out your copyright notice in the Description page of Project Settings.


#include "OverheadWidget.h"
#include "Components/TextBlock.h"
/// <summary>
/// �����ı�����
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
	//GetLocalRole����GetRemoteRole����ݵ������Ļ����Ĳ�ͬ���·��ز�ͬ�Ľ��
	ENetRole RemoteRole = InPawn->GetRemoteRole();
	FString Role;
	switch (RemoteRole) {//������ͺ�˵�� �鿴��https://blog.csdn.net/m0_53278265/article/details/129503448
	case ENetRole::ROLE_Authority: //�ڱ��������ɫGetLocalRole()�У��������ϴ��ڵ�����Pawn����ΪAuthority
		Role = FString("Authority"); //��Զ�������ɫRemoteRole()�У����෴���������ϵ������ƵĽ�ɫΪAutonomous Proxy������������������ɫΪROLE_SimulatedProxy
		break;
	case ENetRole::ROLE_AutonomousProxy://�ڱ��������ɫ�У��ͻ����ϵ������ƵĽ�ɫΪAutonomous Proxy�����ͻ�����������������ɫΪROLE_SimulatedProxy
		Role = FString("Autonomous Proxy");//��Զ�������ɫRemoteRole()�У����෴���ͻ����ϴ��ڵ�����Pawn��ΪAuthority
		break;
	case ENetRole::ROLE_SimulatedProxy:
		Role = FString("Simulated Proxy");
		break;
	case ENetRole::ROLE_None:
		Role = FString("None");
		break;
	}//ͨ��������ʽ����ȷ������������һ��
	FString RemoteRoleString = FString::Printf(TEXT("Remote Role: %s"), *Role);
	SetDisplayText(RemoteRoleString);

	///����������������ʾ��������ģ���ԭ�������޹ء�����˵��43�Ŀ�ѡ��ս
	FString Name = InPawn->GetName();
	SetDisplayText(Name);
	///
}
/// <summary>
/// �����ɵ���ͬ�Ĺؿ����뿪�ؿ�ʱ����
/// </summary>
/// <param name="InLevel"></param>
/// <param name="InWorld"></param>
void UOverheadWidget::OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld)
{
	//���ӿ�ɾ��
	RemoveFromParent();
	Super::OnLevelRemovedFromWorld(InLevel, InWorld);
}
