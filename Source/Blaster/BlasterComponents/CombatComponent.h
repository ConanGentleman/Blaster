// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Blaster/HUD/BlasterHUD.h"
#include "Blaster/Weapon/WeaponTypes.h"
#include "CombatComponent.generated.h"
//射线检测的距离长度
#define TRACE_LENGTH 80000.f

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
	/// <summary>
	/// 设置瞄准状态
	/// </summary>
	/// <param name="bIsAiming"></param>
	void SetAiming(bool bIsAiming);

	/// <summary>
	/// RPC用于客户端调用服务器执行的函数，保证客户端同样能够瞄准
	/// 同理，见BlasterCharacter中的ServerEquipButtonPressed函数说明
	/// </summary>
	UFUNCTION(Server, Reliable)
	void ServerSetAiming(bool bIsAiming);

	/// <summary>
	/// 用于复制变量发生改变（被赋值），服务器通知客户端调用的函数
	/// </summary>
	UFUNCTION()
	void OnRep_EquippedWeapon();
	/// <summary>
	/// 按下开火键，调用开火，并且进行射线检测
	/// </summary>
	/// <param name="bPressed"></param>
	void FireButtonPressed(bool bPressed);

	/// <summary>
	/// 开火
	/// </summary>
	void Fire();

	/// <summary>
	/// 开火RPC。用于客户端调用，服务器执行的武器开火函数。在定义时需在函数名后补充_Implementation
	/// </summary>
	/// <param name="TraceHitTarget">用于传递开火后射线检测到的目标位置传递到服务器。FVector_NetQuantize是为了便于网络传输对FVector的封装（序列化），截断小数点，四舍五入取整，使消息大小降低。这里当成正常的FVector即可。</param>
	UFUNCTION(Server, Reliable)
	void ServerFire(const FVector_NetQuantize& TraceHitTarget);

	/// <summary>
	/// 开火 多播RPC。如果在服务器上调用多播RPC，那么将在服务器以及所有客户端上调用。在定义时需在函数名后补充_Implementation
	/// </summary>
/// <param name="TraceHitTarget">用于同步开火后射线检测到的目标位置到服务器盒所有客户端。FVector_NetQuantize是为了便于网络传输对FVector的封装（序列化），截断小数点，四舍五入取整，使消息大小降低。这里当成正常的FVector即可。</param>
	UFUNCTION(NetMulticast, Reliable)
	void MulticastFire(const FVector_NetQuantize& TraceHitTarget);

	/// <summary>
	/// 从屏幕中心发射射线，进行用于射击的射线检测
	/// </summary>
	/// <param name="TraceHitResult">命中信息</param>
	void TraceUnderCrosshairs(FHitResult& TraceHitResult);

	/// <summary>
	/// 设置准星的贴图并用于BlasterHUD绘制
	/// </summary>
	/// <param name="DeltaTime"></param>
	void SetHUDCrosshairs(float DeltaTime);
private:
	//下面三个变量加上UPROPERTY()的原因是让变量初始化为nullptr，即与变量=nullptr相同

	/// <summary>
	/// 当前对应的角色。这样就可以访问角色来调用其上的函数并执行附加武器之类的操作 
	/// </summary>
	UPROPERTY()
	class ABlasterCharacter* Character;
	/// <summary>
	/// 当前对应的玩家控制器。用于获取HUD
	/// </summary>
	UPROPERTY()
	class ABlasterPlayerController* Controller;
	/// <summary>
	/// 当前对应的HUD。从ABlasterPlayerController中获取
	/// </summary>
	UPROPERTY()
	class ABlasterHUD* HUD;
	/// <summary>
	/// 当前装备的武器(设置为复制变量是因为，在此之前装备的武器在所有客户端上都将为空，
	/// 因为我们只在服务器上设置它，即BlasterCharacter中的EquipButtonPressed调用EquipWeapon
	/// ，然后在EquipWeapon中对EquippedWeapon变量赋值，然而EquippedWeapon之前并没有设置为Replicated变量
	/// 因此这里将EquippedWeapon设置为Replicated以便在其改变时，复制到客户端。
	/// 注意Replicated变量一定要在GetLifetimeReplicatedProps函数在里面注册该变量
	/// 然后由于目前装备武器时的移动动画没有能够同步，因此使用ReplicatedUsing来使得服务器通知客户端调用OnRep_EquippedWeapon函数，
	/// OnRep_EquippedWeapon则用于对角色移动朝向的同步
	/// </summary>
	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
	class AWeapon* EquippedWeapon;

	/// <summary>
	/// 是否正在瞄准(由于跟EquippedWeapon一样需要将服务器瞄准的操作同步到客户端，因此设置为复制变量
	/// </summary>
	UPROPERTY(Replicated)
	bool bAiming;

	/// <summary>
	/// 行走速度
	/// </summary>
	UPROPERTY(EditAnywhere)
	float BaseWalkSpeed;
	/// <summary>
	/// 瞄准行走速度
	/// </summary>
	UPROPERTY(EditAnywhere)
	float AimWalkSpeed;

	/// <summary>
	/// 武器是否开火
	/// </summary>
	bool bFireButtonPressed;


	/**
	* 用于控制准星扩散位置的变量
	*/
	/// <summary>
	/// 速度影响因素（大小）
	/// </summary>
	float CrosshairVelocityFactor;
	/// <summary>
	/// 在空中（跳跃）的影响因素（大小）
	/// </summary>
	float CrosshairInAirFactor; 
	/// <summary>
	/// 瞄准的因素（瞄准时缩小准星
	/// </summary>
	float CrosshairAimFactor;
	/// <summary>
	/// 射击的因素（射击时扩大准星
	/// </summary>
	float CrosshairShootingFactor;

	/// <summary>
	/// 射线检测到的目标位置（进队本地有效，用于调试而声明的变量，后续应该会删除）
	/// </summary>
	FVector HitTarget;

	/// <summary>
	/// 准星结构体
	/// </summary>
	FHUDPackage HUDPackage;

	/** 
	* 瞄准和视野
	*/

	/// <summary>
	/// 不瞄准时的视野;在开始游戏中设置相机的基础视野
	/// </summary>
	float DefaultFOV;

	/// <summary>
	/// 缩放视野
	/// </summary>
	UPROPERTY(EditAnywhere, Category = Combat)
	float ZoomedFOV = 30.f;

	/// <summary>
	/// 当前视野
	/// </summary>
	float CurrentFOV;

	/// <summary>
	/// 缩放插值速度
	/// </summary>
	UPROPERTY(EditAnywhere, Category = Combat)
	float ZoomInterpSpeed = 20.f;

	/// <summary>
	/// 插值改变视野
	/// </summary>
	/// <param name="DeltaTime"></param>
	void InterpFOV(float DeltaTime);

	/**
	* 自动开火（就是长按按键就一直射击
	*/

	/// <summary>
	/// 开火计时器
	/// </summary>
	FTimerHandle FireTimer;
	bool bCanFire = true;

	/// <summary>
	/// 启动开火计时
	/// </summary>
	void StartFireTimer();
	/// <summary>
	/// 计时器完成时回调的函数
	/// </summary>
	void FireTimerFinished();

	/// <summary>
	/// 是否能够开火
	/// </summary>
	/// <returns></returns>
	bool CanFire();


	/// <summary>
	/// 所携带的对应当前装备武器类型的子弹。将该变量放在CombatComponent而没有放在PlayerState是因为PlayerState复制变量更慢
	/// </summary>
	UPROPERTY(ReplicatedUsing = OnRep_CarriedAmmo)
	int32 CarriedAmmo;

	/// <summary>
	/// 携带子弹数量复制时调用的函数
	/// </summary>
	UFUNCTION()
	void OnRep_CarriedAmmo();

	/// <summary>
	/// 武器类型与对应的携带的子弹数量
	/// </summary>
	TMap<EWeaponType, int32> CarriedAmmoMap;


};
