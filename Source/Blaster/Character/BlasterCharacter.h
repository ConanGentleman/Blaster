// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
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
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	///控制人物移动和旋转
	void MoveForward(float Value);
	void MoveRight(float Value);
	void Turn(float Value);
	void LookUp(float Value);

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
	UPROPERTY(EditAnywhere,BlueprintReadOnly,meta = (AllowPrivateAccess = "true"))
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
	UPROPERTY(ReplicatedUsing = OnRep_OverleappingWeapon)
	class AWeapon* OverlappingWeapon;


	UFUNCTION()
	/// <summary>
	/// 当绑定重叠武器复制前调用的函数可以无参也可以有一个参数（该参数为复制变量）
	/// </summary>
	/// <param name="LastWeapon">为变量被复制之前的最后一个值</param>
	void OnRep_OverleappingWeapon(AWeapon* LastWeapon);
public:	
	/// <summary>
	/// 用于在武器类中设置复制变量OverlappingWeapon
	/// FORCEINLINE表示内敛函数
	/// </summary>
	void SetOverlappingWeapon(AWeapon* Weapon);

};
