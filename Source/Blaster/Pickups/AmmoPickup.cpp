// Fill out your copyright notice in the Description page of Project Settings.


#include "AmmoPickup.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/BlasterComponents/CombatComponent.h"

void AAmmoPickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	//调用父类的函数
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor);
	if (BlasterCharacter)
	{
		UCombatComponent* Combat = BlasterCharacter->GetCombat();
		if (Combat)
		{
			//执行捡起子弹的操作
			Combat->PickupAmmo(WeaponType, AmmoAmount);
		}
	}
	Destroy();
}