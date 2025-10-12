// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Blaster/BlasterTypes/Team.h"
#include "FlagZone.generated.h"

UCLASS()
class BLASTER_API AFlagZone : public AActor
{
	GENERATED_BODY()

public:
	AFlagZone();

	UPROPERTY(EditAnywhere)
		ETeam Team;
protected:
	virtual void BeginPlay() override;
	/// <summary>
	/// Åö×²¼ì²â£¨ÆìÖÄµ½´ï¼ì²âµÄÇøÓò
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
	/// Åö×²¼ì²âºÐ×Ó
	/// </summary>
	UPROPERTY(EditAnywhere)
	class USphereComponent* ZoneSphere;

public:

};