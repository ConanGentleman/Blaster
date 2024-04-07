// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"

///战斗组件
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BLASTER_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UCombatComponent();
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	//由于BlasterCharacter与CombatComponent紧密相连，有点相互依赖，需要彼此访问函数或者变量，
	//因此让BlasterCharacter成为该CombatComponent类的友元类，以便于BlasterCharacter可以完全访问该类上的所有变量和函数
	friend class ABlasterCharacter;
	///装备武器
	void EquipWeapon(class AWeapon* WeaponToEquip);
protected:
	// Called when the game starts
	virtual void BeginPlay() override;
private:
	/// <summary>
	/// 当前对应的角色。这样就可以访问角色来调用其上的函数并执行附加武器之类的操作 
	/// </summary>
	class ABlasterCharacter* Character;
	/// <summary>
	/// 当前装备的武器
	/// </summary>
	class AWeapon* EquippedWeapon;
};
