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
	//由于BlasterCharacter与CombatComponent紧密相连，有点相互依赖，需要彼此访问函数或者变量，
	//因此让BlasterCharacter成为该CombatComponent类的友元类，以便于BlasterCharacter可以完全访问该类上的所有变量和函数
	friend class ABlasterCharacter;
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	/// <summary>
	/// 函数内部是注册要replicated（复制）的变量的地方。便于将服务器上的replicated变量同步到各个客户端
	/// </summary>
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

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
	/// 当前装备的武器(设置为复制变量是因为，在此之前装备的武器在所有客户端上都将为空，
	/// 因为我们只在服务器上设置它，即BlasterCharacter中的EquipButtonPressed调用EquipWeapon
	/// ，然后在EquipWeapon中对EquippedWeapon变量赋值，然而EquippedWeapon之前并没有设置为Replicated变量
	/// 因此这里将EquippedWeapon设置为Replicated以便在其改变时，复制到客户端。
	/// 注意Replicated变量一定要在GetLifetimeReplicatedProps函数在里面注册该变量
	/// </summary>
	UPROPERTY(Replicated)
	class AWeapon* EquippedWeapon;
};
