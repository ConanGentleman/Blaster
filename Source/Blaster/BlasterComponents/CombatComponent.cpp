﻿// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatComponent.h"
#include "Blaster/Weapon/Weapon.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Engine/SkeletalMeshSocket.h"

// Sets default values for this component's properties
UCombatComponent::UCombatComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	//设为false则表示每帧不会执行TickComponent
	PrimaryComponentTick.bCanEverTick = false;
}

// Called when the game starts
void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();
}
// Called every frame
void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}
/// <summary>
/// 装备(捡起)武器
/// </summary>
/// <param name="WeaponToEquip"></param>
void UCombatComponent::EquipWeapon(AWeapon* WeaponToEquip)
{
	if (Character == nullptr || WeaponToEquip == nullptr) return;

	EquippedWeapon = WeaponToEquip; 
	//设置武器状态
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	
	//先获取人物网格的武器插槽
	const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName("RightHandSocket"));
	
	if (HandSocket) {
		//将武器放在人物网格插槽处
		HandSocket->AttachActor(EquippedWeapon, Character->GetMesh());
	}
	//设置武器的所有者
	EquippedWeapon->SetOwner(Character);
	//装备武器后，隐藏其文字提示
	EquippedWeapon->ShowPickupWidget(false);
}
