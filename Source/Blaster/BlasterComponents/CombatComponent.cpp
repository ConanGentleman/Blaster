// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatComponent.h"
#include "Blaster/Weapon/Weapon.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Components/SphereComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"

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
void UCombatComponent::SetAiming(bool bIsAiming)
{
	bAiming = bIsAiming;
	//如果没有服务器权限，则表示是客户端调用的，因此需要调用RPC函数。
	// 由于RPC函数的特性，在服务器上调用ServerSetAiming也只是是在服务器上执行ServerSetAiming（相当于就是一个函数调用）；在客户端调用也是在服务器上执行ServerSetAiming
	//所以就直接去掉了HasAuthority()的判断
	//把上一行bAiming = bIsAiming;留下来的原因是，在客户端调用RPC必须等待RPC到达服务器并且将bAiming复制回来，存在一定时间，我们可以立即将其设置，反正复制回来的Aiming的值是一致的
	ServerSetAiming(bIsAiming);
}
/// <summary>
/// 用于武器
/// </summary>
void UCombatComponent::OnRep_EquippedWeapon()
{
	if (EquippedWeapon && Character) {
		//为true时，朝向跟移动方向一致，也就是说角色不会横着走
		Character->GetCharacterMovement()->bOrientRotationToMovement = false;
		//为true,设置角色朝向和Controller的朝向一致。也是朝向和相机一致
		Character->bUseControllerRotationYaw = true;
	}
}
/// <summary>
/// 用于客户端调用服务器执行的函数
/// </summary>
/// <param name="bIsAiming"></param>
void UCombatComponent::ServerSetAiming_Implementation(bool bIsAiming)
{
	bAiming = bIsAiming;
}
// Called every frame
void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}
void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UCombatComponent, EquippedWeapon);
	DOREPLIFETIME(UCombatComponent, bAiming);
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
	//为true时，朝向跟移动方向一致，也就是说角色不会横着走
	Character->GetCharacterMovement()->bOrientRotationToMovement = false;
	//为true,设置角色朝向和Controller的朝向一致。也是朝向和相机一致
	Character->bUseControllerRotationYaw = true;
}

