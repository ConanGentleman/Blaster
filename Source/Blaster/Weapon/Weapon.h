// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponTypes.h"
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
	/// 被捡起或者被装备
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
	/// <summary>
	/// 函数内部是注册要replicated（复制）的变量的地方。便于将服务器上的replicated变量同步到各个客户端
	/// </summary>
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	/// <summary>
	/// 当武器所有者改变时调用的函数（AActor已内置的owner复制变量）
	/// </summary>
	virtual void OnRep_Owner() override;
	/// <summary>
	/// 设置HUD上的子弹信息
	/// </summary>
	void SetHUDAmmo();

	void ShowPickupWidget(bool bShowWidget);
	/// <summary>
	/// 开火
	/// </summary>
	/// <param name="HitTarget">已通过射线检测（或其他方法）得到的命中目标的位置</param>
	virtual void Fire(const FVector& HitTarget);

	/// <summary>
	/// 丢弃武器
	/// </summary>
	void Dropped();

	/// <summary>
	/// 添加子弹
	/// </summary>
	/// <param name="AmmoToAdd"></param>
	void AddAmmo(int32 AmmoToAdd);

	/**
	* 武器十字准星的贴图
	*/
	UPROPERTY(EditAnywhere, Category = Crosshairs)
	class UTexture2D* CrosshairsCenter;//准星中心
	UPROPERTY(EditAnywhere, Category = Crosshairs)
	UTexture2D* CrosshairsLeft;//准星左
	UPROPERTY(EditAnywhere, Category = Crosshairs)
	UTexture2D* CrosshairsRight;//准星右
	UPROPERTY(EditAnywhere, Category = Crosshairs)
	UTexture2D* CrosshairsTop;//准星上
	UPROPERTY(EditAnywhere, Category = Crosshairs)
	UTexture2D* CrosshairsBottom;//准星下

	
	/** 
	* 当瞄准时缩放视野
	*/

	//缩放视野范围
	UPROPERTY(EditAnywhere)
	float ZoomedFOV = 30.f;

	//缩放插值速度
	UPROPERTY(EditAnywhere)
	float ZoomInterpSpeed = 20.f;

	/** 
	* 自动开火（就是长按按键就一直射击
	*/

	/// <summary>
	/// 开火延迟时间。函数执行的时间间隔(也就是武器射速
	/// </summary>
	UPROPERTY(EditAnywhere, Category = Combat)
	float FireDelay = .15f;

	/// <summary>
	/// 武器是否自动开火（长按连续开火
	/// </summary>
	UPROPERTY(EditAnywhere, Category = Combat)
	bool bAutomatic = true;

	/// <summary>
	/// 装备武器的音效
	/// </summary>
	UPROPERTY(EditAnywhere)
	class USoundCue* EquipSound;

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
	/// 武器状态（初始化、捡起、扔下） 是一个复制变量
	/// </summary>
	UPROPERTY(ReplicatedUsing = OnRep_WeaponState, VisibleAnywhere, Category = "Weapon Properties")
	EWeaponState WeaponState;

	/// <summary>
	/// 当绑定的武器状态 复制前 调用的函数可以无参也可以有一个参数（该参数为复制变量）
	/// </summary>
	UFUNCTION()
	void OnRep_WeaponState();

	/// <summary>
	/// 用于提示操作捡起物体的UI
	/// </summary>
	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	class UWidgetComponent* PickupWidget;

	/// <summary>
	/// 武器开火火焰动画（特效）
	/// </summary>
	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	class UAnimationAsset* FireAnimation;

	/// <summary>
	/// 武器开火后用于生成的掉落的弹壳
	/// </summary>
	UPROPERTY(EditAnywhere)
	TSubclassOf<class ACasing> CasingClass;

	/// <summary>
	/// 子弹数量（复制变量）
	/// </summary>
	UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_Ammo)
	int32 Ammo;

	/// <summary>
	/// 子弹数量复制时调用的函数
	/// </summary>
	UFUNCTION()
	void OnRep_Ammo();

	/// <summary>
	/// 花费子弹（由于设定的子弹都是一个一个的，所以在这里面就直接--。同时更新HUD上的子弹信息
	/// </summary>
	void SpendRound();

	/// <summary>
	/// 子弹容量
	/// </summary>
	UPROPERTY(EditAnywhere)
	int32 MagCapacity;

	UPROPERTY()
	class ABlasterCharacter* BlasterOwnerCharacter;
	UPROPERTY()
	class ABlasterPlayerController* BlasterOwnerController;

	/// <summary>
	/// 武器类型
	/// </summary>
	UPROPERTY(EditAnywhere)
	EWeaponType WeaponType;

public:	
	///设置武器状态 
	void SetWeaponState(EWeaponState State);
	//FORCEINLINE表示内联函数
	FORCEINLINE USphereComponent* GetAreaSphere() const { return AreaSphere; }
	FORCEINLINE USkeletalMeshComponent* GetWeaponMesh() const { return WeaponMesh; }
	FORCEINLINE float GetZoomedFOV() const { return ZoomedFOV; }
	FORCEINLINE float GetZoomInterpSpeed() const { return ZoomInterpSpeed; }
	bool IsEmpty();
	FORCEINLINE EWeaponType GetWeaponType() const { return WeaponType; }
	FORCEINLINE int32 GetAmmo() const { return Ammo; }
	FORCEINLINE int32 GetMagCapacity() const { return MagCapacity; }
};
