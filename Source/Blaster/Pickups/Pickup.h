// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Pickup.generated.h"

UCLASS()
class BLASTER_API APickup : public AActor
{
	GENERATED_BODY()

public:
	APickup();
	virtual void Tick(float DeltaTime) override;
	virtual void Destroyed() override;
protected:
	virtual void BeginPlay() override;

	/// <summary>
	/// ��ײ���ص������ص����ú���Ӧ��ֻ�ڷ���˵��ã���
	/// </summary>
	UFUNCTION()
	virtual void OnSphereOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

private:
	/// <summary>
	/// ������ײ�塣�ص��������ײ�壩�����ڽ�ɫ����ʱ���ܹ�ʶ����ܱ����������
	/// </summary>
	UPROPERTY(EditAnywhere)
	class USphereComponent* OverlapSphere;
	/// <summary>
	/// ������Ч
	/// </summary>
	UPROPERTY(EditAnywhere)
	class USoundCue* PickupSound;
	/// <summary>
	/// ����
	/// </summary>
	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* PickupMesh;

public:

};