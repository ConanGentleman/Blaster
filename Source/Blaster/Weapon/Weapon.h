﻿// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Weapon.generated.h"

/// <summary>
/// 武器状态枚举（且为蓝图中的类型）
/// </summary>
UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	/// <summary>
	/// 最初始状态，从来没有被捡起过 EWS是EWeaponState缩写
	/// UMETA(DisplayName = "Initial State")用于设置其显示名称，并在蓝图中显示
	/// </summary>
	EWS_Initial UMETA(DisplayName = "Initial State"),
	/// <summary>
	/// 被捡起
	/// </summary>
	EWS_Equipped UMETA(DisplayName = "Equipped"),
	/// <summary>
	/// 被丢弃
	/// </summary>
	EWS_Dropped UMETA(DisplayName = "Dropped"),

	/// <summary>
	/// 大部分枚举有个默认的最大常量。这样如果需要知道这个枚举中实际有多少个场景，可以检查EWS_MAX的大小
	/// </summary>
	EWS_MAX UMETA(DisplayName = "DefaultMAX")
};


UCLASS()
class BLASTER_API AWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWeapon();
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	void ShowPickupWidget(bool bShowWidget);
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	/// <summary>
	/// 碰撞体重叠函数回调（该函数应当只在服务端调用）。
	/// </summary>
	UFUNCTION()
	virtual void OnSphereOverlap(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	/// <summary>
	/// 碰撞体重叠函数结束回调（该函数应当只在服务端调用）。
	/// </summary>
	UFUNCTION()
	virtual void OnSphereEndOverlap(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex
	);
private:
	//用于武器和拥有的所有资产的骨架网格组件
	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	USkeletalMeshComponent* WeaponMesh;

	//重叠体积（碰撞体），用于角色靠近武器时，能够识别该武器
	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	class USphereComponent* AreaSphere;

	/// <summary>
	/// 武器状态（初始化、捡起、扔下）
	/// </summary>
	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	EWeaponState WeaponState;

	/// <summary>
	/// 用于提示操作捡起物体的UI
	/// </summary>
	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	class UWidgetComponent* PickupWidget;
public:	
	///设置武器状态 FORCEINLINE表示内联函数
	FORCEINLINE void SetWeaponState(EWeaponState State) { WeaponState = State; }

};