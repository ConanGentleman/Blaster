// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Blaster/BlasterTypes/TurningInPlace.h"
#include "BlasterCharacter.generated.h"

UCLASS()
class BLASTER_API ABlasterCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ABlasterCharacter();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	/// <summary>
	/// 函数内部是注册要replicated（复制）的变量的地方。便于将服务器上的replicated变量同步到各个客户端
	/// </summary>
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostInitializeComponents() override;
	/// <summary>
	/// 播放开火蒙太奇动画
	/// </summary>
	void PlayFireMontage(bool bAiming);
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	///控制人物移动和旋转
	void MoveForward(float Value);
	void MoveRight(float Value);
	void Turn(float Value);
	void LookUp(float Value);
	//武器装备或切换
	void EquipButtonPressed();
	//蹲下
	void CrouchButtonPressed();
	//瞄准
	void AimButtonPressed();
	//取消瞄准
	void AimButtonReleased();
	//动画偏移（动画叠加），这里用于获取角色枪口方向的值AO_Yaw和AO_Pitch来复制到BlasterAnimInstance中。 DeltaTime用于插值过渡动画
	void AimOffset(float DeltaTime);
	virtual void Jump() override;
	void FireButtonPressed();
	void FireButtonReleased();
private:
	//弹簧臂组件 (向前声明
	UPROPERTY(VisibleAnywhere, Category = Camera)
	class USpringArmComponent* CameraBoom;
	//相机组件
	UPROPERTY(VisibleAnywhere, Category = Camera)
	class UCameraComponent* FollowCamera;
	/// <summary>
	/// 用于在人物头顶上显示的小部件。
	/// 不能对C++中私有的变量使用蓝图使用BlueprintReadOnly或者BlueprintReadWrite除非
	/// 添加了meta = (AllowPrivateAccess = "true")，这意味着将此变量暴露给蓝图
	/// </summary>
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UWidgetComponent* OverheadWidget;

	/// <summary>
	/// 希望replicate该变量，意味这当他在服务器上发生变更时，它可以在所有客户端上同步更改。
	/// 所以我们可以复制指向武器的指针
	/// 因此需要添加一个UPROPERTY(Replicated),这表示将OverlappingWeapon设置为复制变量。
	/// 当然完成上述操作后，还需要在计划赋值变量的任何类中重写函数GetLifetimeReplicatedProps
	/// 变量只会在更改的时候才会复制，并不在每一帧或者每个网络更新复制！！！
	/// </summary>
	//UPROPERTY(Replicated)
	///ReplicatedUsing 表示在客户端的变量接收到服务器的改变时，在客户端上调用绑定好的回调函数(函数名一般以OnRep开头)
	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
	class AWeapon* OverlappingWeapon;

	/// <summary>
	/// 当绑定的重叠武器 复制前 调用的函数可以无参也可以有一个参数（该参数为复制变量）
	/// </summary>
	/// <param name="LastWeapon">为变量被复制之前的最后一个值</param>
	UFUNCTION()
	void OnRep_OverlappingWeapon(AWeapon* LastWeapon);

	/// <summary>
	/// 战斗组件，用于处理角色所有雨战斗相关的功能（也是一个可以被复制的变量
	/// </summary>
	UPROPERTY(VisibleAnywhere)
	class UCombatComponent* Combat;

	//远程过程调用（RPC）具有在一台机器上调用并在另一台机器上执行的功能。例如可以从客户端上调用，并在服务器上执行对应函数
	//这里的函数打算在客户端调用并在服务器上执行
	//需要选择是可靠Reliable还是不可靠的执行，不可靠可能存在丢弃的问题。当服务器收到 RBC 时，客户端将收到确认，如果它没有发送确认，RBC 将再次发送
	//RPC函数名在CPP中的定义会在.h中的声明多加_Implementation，这是因为虚幻在幕后做了一些工作
	UFUNCTION(Server, Reliable)
	void ServerEquipButtonPressed();

	/// <summary>
	/// 方便赋值给BlasterAnimInstance中的同名变量
	/// </summary>
	float AO_Yaw;

	/// <summary>
	/// 用于记录角色插值转向角度
	/// </summary>
	float InterpAO_Yaw;

	/// <summary>
	/// 方便赋值给BlasterAnimInstance中的同名变量
	/// </summary>
	float AO_Pitch;

	/// <summary>
	/// 角色静止前的旋转（每帧都会储存
	/// </summary>
	FRotator StartingAimRotation;

	/// <summary>
	/// 角色转向状态
	/// </summary>
	ETurningInPlace TurningInPlace;

	/// <summary>
	/// 实时更新角色转向状态
	/// </summary>
	/// <param name="DeltaTime"></param>
	void TurnInPlace(float DeltaTime);

	/// <summary>
	/// 开火动画的蒙太奇动画
	/// </summary>
	UPROPERTY(EditAnywhere, Category = Combat)
	class UAnimMontage* FireWeaponMontage;
public:	
	/// <summary>
	/// 用于在武器类中设置复制变量OverlappingWeapon
	/// FORCEINLINE表示内敛函数
	/// </summary>
	void SetOverlappingWeapon(AWeapon* Weapon);
	/// <summary>
	/// 用于BlasterAnimInstance中访问判断是否装备武器，以播放相应的动画
	/// </summary>
	/// <returns></returns>
	bool IsWeaponEquipped();

	/// <summary>
	/// 是否瞄准状态
	/// </summary>
	/// <returns></returns>
	bool IsAiming();

	/// <summary>
	/// 返回给BlasterAnimInstance AO_Yaw变量的值
	/// </summary>
	FORCEINLINE float GetAO_Yaw() const { return AO_Yaw; }
	FORCEINLINE float GetAO_Pitch() const { return AO_Pitch; }
	AWeapon* GetEquippedWeapon();
	FORCEINLINE ETurningInPlace GetTurningInPlace() const { return TurningInPlace; }
	FVector GetHitTarget() const;
	/// <summary>
	/// 获取相机
	/// </summary>
	/// <returns></returns>
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};
