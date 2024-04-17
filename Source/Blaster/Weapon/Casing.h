// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Casing.generated.h"

UCLASS()
class BLASTER_API ACasing : public AActor
{
	GENERATED_BODY()
	
public:	
	ACasing();
private:
	/// <summary>
	/// 子弹网格体（子弹模型）
	/// </summary>
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* CasingMesh;
protected:
	virtual void BeginPlay() override;

//public:	
//	virtual void Tick(float DeltaTime) override;

};
