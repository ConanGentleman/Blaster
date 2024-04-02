// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyGameMode.h"
#include "GameFramework/GameStateBase.h"
/// <summary>
/// ÿ�����˼�����Ϸʱ�ͻ���øú���
/// </summary>
/// <param name="NewPlayer"></param>
void ALobbyGameMode::PostLogin(APlayerController* NewPlayer) {
	Super::PostLogin(NewPlayer);
	//��ǰ�����ǵ�һ�����԰�ȫ���ʸոռ������ҿ������ĵط�
	//ͨ��GameState��ȡ�������,�õ��������
	//����������ǣ���һ���˷����������󣬻���ת���������棻
	//���ȴ������������Ҵﵽһ�������󣬲�֪ͨ��������ת����Ϸ������
	//������ת����Ϸ����ʱ����Ҫ���ȼ��س�������˻�����ת�����ȳ���������ת����Ϸ����
	//���ȳ���������Map���У����ڵȴ���Ϸ�����ļ��أ���������ɺ������������ת�����س��������������UE�ṩ��������Ŀ�����е�transition������ת��ͼ���ɣ�
	int32 NumberOfPlayer = GameState.Get()->PlayerArray.Num();
	if (NumberOfPlayer == 2) {
		
		UWorld* World = GetWorld();
		if (World) {
			//�����޷����Σ��޷����������ʱ�����ȶϿ�����������ͬ�ķ�������
			bUseSeamlessTravel = true;
			//���÷�����������ʹ���������ӵĿͻ������ε���Ϸ��ͼ
			World->ServerTravel(FString("/Game/Maps/BlasterMap?listen"));
		}
	}

}