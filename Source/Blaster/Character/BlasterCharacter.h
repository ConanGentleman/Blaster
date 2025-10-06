// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Blaster/BlasterTypes/TurningInPlace.h"
#include "Blaster/Interfaces/InteractWithCrosshairsInterface.h"
#include "Components/TimelineComponent.h"
#include "Blaster/BlasterTypes/CombatState.h"
#include "BlasterCharacter.generated.h"

UCLASS()
class BLASTER_API ABlasterCharacter : public ACharacter, public IInteractWithCrosshairsInterface
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


	/**
	* Play montages
	* 播放蒙太奇
	*/

	/// <summary>
	/// 播放开火蒙太奇动画
	/// </summary>
	void PlayFireMontage(bool bAiming);
	/// <summary>
	/// 播放换弹动画
	/// </summary>
	void PlayReloadMontage();
	/// <summary>
	/// 播放淘汰（死亡）蒙太奇动画
	/// </summary>
	void PlayElimMontage();
	/// <summary>
	/// 投掷手榴弹蒙太奇动画
	/// </summary>
	void PlayThrowGrenadeMontage();
	/// <summary>
	/// 播放切换武器蒙太奇动画
	/// </summary>
	void PlaySwapMontage();

	///// <summary>
	///// 用于同步角色受击动画的 多播RPC。客户端调用，服务器执行的函数。如果在服务器上执行多播RPC，那么将在服务器以及所有客户端上调用。在定义时需在函数名后补充_Implementation
	///// Unreliable不可靠的，因为这种受击动画可有可无，是一种装饰物
	///// </summary>
	//UFUNCTION(NetMulticast, Unreliable)
	//void MulticastHit();

	/// <summary>
	/// 复制运动的函数通知（同步各个客户端和服务器的角色移动的函数），每次运动发生改变时会被调用。跟tick频率不同，因此设计移动方面的同步需要用到该函数
	/// </summary>
	virtual void OnRep_ReplicatedMovement() override;

	void Elim();

	/// <summary>
	/// 淘汰（角色死亡） 。多播RPC，客户端调用，服务器执行并让各个客户端同步执行。
	/// </summary>
	UFUNCTION(NetMulticast, Reliable)
	void MulticastElim();

	/// <summary>
	/// 角色生命周期函数，在角色销毁时调用,在服务器上销毁一个复制的actor的行为会传播到所有客户端。用以销毁淘汰机器人粒子组件
	/// </summary>
	virtual void Destroyed() override;
	
	/// <summary>
	/// 玩家是否处于冷却等待状态（是否关闭游戏输入指令）-复制变量
	/// </summary>
	UPROPERTY(Replicated)
	bool bDisableGameplay = false;

	/// <summary>
	/// 显示狙击枪开镜准星 BlueprintImplementableEvent表示可实现蓝图事件的函数，在角色蓝图中调用
	/// </summary>
	UFUNCTION(BlueprintImplementableEvent)
	void ShowSniperScopeWidget(bool bShowScope);

	/// <summary>
	/// 更新角色HUD的血量信息
	/// </summary>
	void UpdateHUDHealth();
	/// <summary>
	/// 更新角色HUD的护盾信息
	/// </summary>
	void UpdateHUDShield();
	/// <summary>
	///  生成默认武器后也应该更新一下子弹数量
	/// </summary>
	void UpdateHUDAmmo();
	/// <summary>
	/// 生成默认初始武器
	/// </summary>
	void SpawDefaultWeapon();

	/// <summary>
	/// 名字映射box组件表
	/// </summary>
	UPROPERTY()
	TMap<FName, class UBoxComponent*> HitCollisionBoxes;

	/// <summary>
	/// 是否完成切换武器(主要用于切换枪时左手附着IK的时机
	/// </summary>
	bool bFinishedSwapping = false;
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
	//换弹
	void ReloadButtonPressed();
	//瞄准
	void AimButtonPressed();
	//取消瞄准
	void AimButtonReleased();
	//动画偏移（动画叠加），这里用于获取角色枪口方向的值AO_Yaw和AO_Pitch来复制到BlasterAnimInstance中。 DeltaTime用于插值过渡动画
	void AimOffset(float DeltaTime);
	/// <summary>
	/// 计算角色上下朝向角度
	/// </summary>
	void CalculateAO_Pitch();
	//用于处理模拟代理（服务器上）的转向。用来解决角色转向同步时，转向抖动的问题
	void SimProxiesTurn();
	virtual void Jump() override;
	void FireButtonPressed();
	void FireButtonReleased();
	//播放受击动画
	void PlayHitReactMontage();
	/// <summary>
	/// 投掷手榴弹
	/// </summary>
	void GrenadeButtonPressed();
	/// <summary>
	/// 丢弃或销毁某个武器
	/// </summary>
	/// <param name="Weapon"></param>
	void DropOrDestroyWeapon(AWeapon* Weapon);
	/// <summary>
	/// 丢弃或销毁所有武器
	/// </summary>
	void DropOrDestroyWeapons();

	/// <summary>
	/// 用于接收来自子弹类（如ProjectileBullet）中的ApplyDamage委托的回调函数。
	/// 必须添加UFUNCTION()
	/// </summary>
	/// <param name="DamagedActor"></param>
	/// <param name="Damage"></param>
	/// <param name="DamageType"></param>
	/// <param name="InstigatorController"></param>
	/// <param name="DamageCauser"></param>
	UFUNCTION()
	void ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, class AController* InstigatorController, AActor* DamageCauser);

	// 轮询任何相关类并初始化我们的HUD。也就是当角色死亡重生后，需要将其玩家状态类的信息用以初始化HUD上的信息。
	void PollInit();
	/// <summary>
	/// 玩家原地旋转
	/// </summary>
	/// <param name="DeltaTime"></param>
	void RotateInPlace(float DeltaTime);

	/**
	* Hit boxes used for server-side rewind
	* 服务器端倒带的命中框(碰撞体） 命名大部分跟骨骼名一致
	*  即用于延迟补偿计算的碰撞框
	*/

	UPROPERTY(EditAnywhere)
	class UBoxComponent* head;

	UPROPERTY(EditAnywhere)
	UBoxComponent* pelvis;

	UPROPERTY(EditAnywhere)
	UBoxComponent* spine_02;

	UPROPERTY(EditAnywhere)
	UBoxComponent* spine_03;

	UPROPERTY(EditAnywhere)
	UBoxComponent* upperarm_l;

	UPROPERTY(EditAnywhere)
	UBoxComponent* upperarm_r;

	UPROPERTY(EditAnywhere)
	UBoxComponent* lowerarm_l;

	UPROPERTY(EditAnywhere)
	UBoxComponent* lowerarm_r;

	UPROPERTY(EditAnywhere)
	UBoxComponent* hand_l;

	UPROPERTY(EditAnywhere)
	UBoxComponent* hand_r;

	UPROPERTY(EditAnywhere)
	UBoxComponent* backpack;

	UPROPERTY(EditAnywhere)
	UBoxComponent* blanket;

	UPROPERTY(EditAnywhere)
	UBoxComponent* thigh_l;

	UPROPERTY(EditAnywhere)
	UBoxComponent* thigh_r;

	UPROPERTY(EditAnywhere)
	UBoxComponent* calf_l;

	UPROPERTY(EditAnywhere)
	UBoxComponent* calf_r;

	UPROPERTY(EditAnywhere)
	UBoxComponent* foot_l;

	UPROPERTY(EditAnywhere)
	UBoxComponent* foot_r;
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

	/**
	* Blaster 组件
	*/


	/// <summary>
	/// 战斗组件，用于处理角色所有雨战斗相关的功能（也是一个可以被复制的变量
	/// </summary>
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UCombatComponent* Combat1;

	UPROPERTY(VisibleAnywhere)
	class UBuffComponent* Buff;

	/// <summary>
	/// 延迟补偿组件
	/// </summary>
	UPROPERTY(VisibleAnywhere)
	class ULagCompensationComponent* LagCompensation;


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

	/**
	* 动画蒙太奇
	*/

	/// <summary>
	/// 开火动画的蒙太奇动画
	/// </summary>
	UPROPERTY(EditAnywhere, Category = Combat1)
	class UAnimMontage* FireWeaponMontage;

	/// <summary>
	/// 换弹动画蒙太奇
	/// </summary>
	UPROPERTY(EditAnywhere, Category = Combat1)
	UAnimMontage* ReloadMontage;


	/// <summary>
	/// 受击动画蒙太奇
	/// </summary>
	UPROPERTY(EditAnywhere, Category = Combat1)
	UAnimMontage* HitReactMontage;

	/// <summary>
	/// 淘汰（死亡）动画蒙太奇
	/// </summary>
	UPROPERTY(EditAnywhere, Category = Combat1)
	UAnimMontage* ElimMontage;

	/// <summary>
	/// 投掷手榴弹动画蒙太奇
	/// </summary>
	UPROPERTY(EditAnywhere, Category = Combat1)
	UAnimMontage* ThrowGrenadeMontage;

	/// <summary>
	/// 切换武器动画蒙太奇
	/// </summary>
	UPROPERTY(EditAnywhere, Category = Combat1)
	UAnimMontage* SwapMontage;

	/// <summary>
	/// 用于当角色靠墙时，角色模型就会挡住视野，靠墙时隐藏角色
	/// </summary>
	void HideCameraIfCharacterClose();

	UPROPERTY(EditAnywhere)
	float CameraThreshold = 200.f;
	//是否旋转根骨骼
	bool bRotateRootBone;
	//播放原地转向动画的阈值
	float TurnThreshold = 0.5f;
	//上一帧代理的旋转值
	FRotator ProxyRotationLastFrame;
	//当前代理的旋转值
	FRotator ProxyRotation;
	/// <summary>
	/// 代理转向的偏移量
	/// </summary>
	float ProxyYaw;
	/// <summary>
	/// 上次运动改变（复制）以来的时间间隔
	/// </summary>
	float TimeSinceLastMovementReplication;
	float CalculateSpeed();

	
	/**
	* 玩家血量
	*/

	/// <summary>
	/// 最大血量
	/// </summary>
	UPROPERTY(EditAnywhere, Category = "Player Stats")
	float MaxHealth = 100.f;

	/// <summary>
	/// 当前血量（可复制，复制通知为OnRep_Health）
	/// </summary>
	UPROPERTY(ReplicatedUsing = OnRep_Health, VisibleAnywhere, Category = "Player Stats")
	float Health = 100.f;

	/// <summary>
	/// （在服务器上）血量变化时，通知各客户端调用的回调函数。
	/// </summary>
	UFUNCTION()
	void OnRep_Health(float LastHealth);


	/**
	* 玩家护盾
	*/

	/// <summary>
	/// 最大护盾
	/// </summary>
	UPROPERTY(EditAnywhere, Category = "Player Stats")
	float MaxShield = 100.f;

	/// <summary>
	/// 当前护盾（可复制，复制通知为OnRep_Shield）
	/// </summary>
	UPROPERTY(ReplicatedUsing = OnRep_Shield, EditAnywhere, Category = "Player Stats")
	float Shield = 0.f;

	/// <summary>
	/// （在服务器上）护盾变化时，通知各客户端调用的回调函数。
	/// </summary>
	UFUNCTION()
	void OnRep_Shield(float LastShield);


	//加上UPROPERTY()的原因是让BlasterPlayerController初始化为nullptr，即与class ABlasterPlayerController* BlasterPlayerController=nullptr相同
	UPROPERTY()
	class ABlasterPlayerController* BlasterPlayerController;
	/// <summary>
	/// 角色是否被淘汰
	/// </summary>
	bool bElimmed = false;

	/// <summary>
	/// 角色淘汰计时器（用于角色重生）
	/// </summary>
	FTimerHandle ElimTimer;

	/// <summary>
	/// 淘汰的间隔时间（重生的时间）
	/// EditDefaultsOnly表示此成员变量只会在蓝图编辑器中被暴露出来。
	/// </summary>
	UPROPERTY(EditDefaultsOnly)
	float ElimDelay = 3.f;
	/// <summary>
	/// 淘汰计时器完成后调用的函数
	/// </summary>
	void ElimTimerFinished();

	/**
	* 溶解效果
	*/

	/// <summary>
	/// 时间轴组件（绘制一个时间曲线用的）
	/// </summary>
	UPROPERTY(VisibleAnywhere)
	UTimelineComponent* DissolveTimeline;
	/// <summary>
	/// 时间线中使用的轨道
	/// </summary>
	FOnTimelineFloat DissolveTrack;

	/// <summary>
	/// 时间轴上的曲线。时间轴上至少有一个曲线才能开始运行
	/// </summary>
	UPROPERTY(EditAnywhere)
	UCurveFloat* DissolveCurve;

	/// <summary>
	/// 材质溶解时间轴的回调函数
	/// </summary>
	UFUNCTION()
	void UpdateDissolveMaterial(float DissolveValue);
	/// <summary>
	/// 启动材质溶解时间轴
	/// </summary>
	void StartDissolve();

	// Dynamic instance that we can change at runtime
	//我们可以在运行时更改的动态(材质)实例。方便RPC通知同步
	UPROPERTY(VisibleAnywhere, Category = Elim)
	UMaterialInstanceDynamic* DynamicDissolveMaterialInstance;

	// Material instance set on the Blueprint, used with the dynamic material instance
	//在蓝图上设置材质实例，与动态材质实例一起使用。相当于初始材质了。
	UPROPERTY(EditAnywhere, Category = Elim)
	UMaterialInstance* DissolveMaterialInstance;

	/**
	* 淘汰机器人
	*/

	/// <summary>
	/// 粒子特效
	/// </summary>
	UPROPERTY(EditAnywhere)
	UParticleSystem* ElimBotEffect;

	/// <summary>
	/// 淘汰机器人例子组件
	/// </summary>
	UPROPERTY(VisibleAnywhere)
	UParticleSystemComponent* ElimBotComponent;

	/// <summary>
	/// 淘汰机器人音效
	/// </summary>
	UPROPERTY(EditAnywhere)
	class USoundCue* ElimBotSound;

	/// <summary>
	/// 当前玩家状态
	/// </summary>
	//加上UPROPERTY()的原因是让BlasterPlayerState初始化为nullptr，即与class ABlasterPlayerState* BlasterPlayerState=nullptr相同
	UPROPERTY()
	class ABlasterPlayerState* BlasterPlayerState;

	/**
	* 手榴弹网格组件（预制模型）
	*/
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* AttachedGrenade;

	/**
	* 默认武器
	*/
	UPROPERTY(EditAnywhere)
	TSubclassOf<AWeapon> DefaultWeaponClass;

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

	FORCEINLINE bool ShouldRotateRootBone() const { return bRotateRootBone; }
	FORCEINLINE bool IsElimmed() const { return bElimmed; }
	FORCEINLINE float GetHealth() const { return Health; }
	FORCEINLINE void SetHealth(float Amount) { Health = Amount; }
	FORCEINLINE float GetMaxHealth() const { return MaxHealth; }
	FORCEINLINE float GetShield() const { return Shield; }
	FORCEINLINE void SetShield(float Amount) { Shield = Amount; }
	FORCEINLINE float GetMaxShield() const { return MaxShield; }
	/// <summary>
	/// 获取战斗状态
	/// </summary>
	/// <returns></returns>
	ECombatState GetCombatState() const;
	FORCEINLINE UCombatComponent* GetCombat() const { return Combat1; }
	/// <summary>
	/// 获取游戏当前是否处于禁止输入状态（游戏冷却状态）
	/// </summary>
	/// <returns></returns>
	FORCEINLINE bool GetDisableGameplay() const { return bDisableGameplay; }
	/// <summary>
	/// 获取玩家当前是否处于装弹状态（霰弹枪一颗一颗装）
	/// </summary>
	/// <returns></returns>
	FORCEINLINE UAnimMontage* GetReloadMontage() const { return ReloadMontage; }
	/// <summary>
	/// 获取手榴弹预制是否显示
	/// </summary>
	/// <returns></returns>
	FORCEINLINE UStaticMeshComponent* GetAttachedGrenade() const { return AttachedGrenade; }
	/// <summary>
	/// 获取buff组件
	/// </summary>
	/// <returns></returns>
	FORCEINLINE UBuffComponent* GetBuff() const { return Buff; }
	/// <summary>
	/// 本地是否处于装弹状态
	/// </summary>
	/// <returns></returns>
	bool IsLocallyReloading();
	/// <summary>
	/// 获取延迟补偿组件
	/// </summary>
	/// <returns></returns>
	FORCEINLINE ULagCompensationComponent* GetLagCompensation() const { return LagCompensation; }
};
