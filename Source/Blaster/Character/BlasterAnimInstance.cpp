﻿// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterAnimInstance.h"
#include "BlasterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Blaster/Weapon/Weapon.h"
#include "Blaster/BlasterTypes/CombatState.h"

void UBlasterAnimInstance::NativeInitializeAnimation() {
	Super::NativeInitializeAnimation();
	//设置角色 ，将Pawn转换为BlasterCharacter
	BlasterCharacter = Cast<ABlasterCharacter>(TryGetPawnOwner());
}
void UBlasterAnimInstance::NativeUpdateAnimation(float DeltaTime)
{

	Super::NativeUpdateAnimation(DeltaTime);

	if (BlasterCharacter == nullptr) {
		BlasterCharacter = Cast<ABlasterCharacter>(TryGetPawnOwner());
	}
	if (BlasterCharacter == nullptr) return;
	//获取速度
	FVector Velocity = BlasterCharacter->GetVelocity();
	//不关心z值
	Velocity.Z = 0.f;
	Speed = Velocity.Size();
	//直接通过API获取
	bIsInAir = BlasterCharacter->GetCharacterMovement()->IsFalling();
	//加速度大于0则是加速
	bIsAccelerating = BlasterCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f ? true : false;
	//是否装备了武器
	bWeaponEquipped = BlasterCharacter->IsWeaponEquipped();
	//设置武器
	EquippedWeapon = BlasterCharacter->GetEquippedWeapon();
	//是否蹲下
	bIsCrouched = BlasterCharacter->bIsCrouched; 
	bAiming = BlasterCharacter->IsAiming();
	//角色转向
	TurningInPlace = BlasterCharacter->GetTurningInPlace();
	//是否旋转根骨骼（平滑控制人物持枪旋转的
	bRotateRootBone = BlasterCharacter->ShouldRotateRootBone();
	//是否被淘汰（或死亡）
	bElimmed = BlasterCharacter->IsElimmed();

	//设置拿着装备的混合动画值
	//自带的获取基础目标旋转的函数
	//AimRotation.Yaw为全局的，与控制的角色无关。类似于东南西北一样的提前固定好的角度，这个输出看一下就清楚了。
	//相机直视世界x轴方向则AimRotation.Yaw为0（这里参照是BlasterCharacter上的相机，也就是人物朝向不变，但是相机朝向变化就会影响GetBaseAimRotation的值，这里涉及的都是Yaw的值
	FRotator AimRotation = BlasterCharacter->GetBaseAimRotation();
	//该函数通常用来制作一些特殊的旋转效果。根据你指定的方向,计算出在世界坐标下的相对旋转.
	//通过给定的X轴，构建一个rotator，Y和Z不需要指定（其实引擎会指定一个隐式的Z轴（0,0,1），用来保证XYZ三个轴向的平面正交
	//另外的解释：MakeRotFromX 就是给定一个方向的actor 以这个方向作为一个物体的forward方向 需要在世界坐标的旋转rotator
	//还有的解释：设置物体rotation的时候，由向量差得到x轴的朝向，无论物体怎么移动，这个物体的x轴的朝向始终是这个向量差，可以得到一个物体始终朝向另一个物体；最多设置一个物体两个朝向；
	//经过如上的转换使得MovementRotation的参照物变成了BlasterCharacter上的人物角色，也就是人物角色的朝向影响了MovementRotation的值，这里涉及的都是Yaw的值
	//所以根据MovementRotation的角度就能设置混合动画的值进行动画的变化 
	FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(BlasterCharacter->GetVelocity());
	//计算从MovementRotation到AimRotation方向的一个旋转差。
	FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation);
	//UE_LOG(LogTemp, Warning, TEXT("AimRotation Yaw %f:"), AimRotation.Yaw);
	//用于旋转的插值函数。返回值为从“当前值”过渡到“期望的目标值”的一个中间值。参数：当前值；期望的目标值，时间变化值，插值速度
	DeltaRotation = FMath::RInterpTo(DeltaRotation, DeltaRot, DeltaTime, 6.f);
	//YawOffset用于扫射移动的动画
	YawOffset = DeltaRotation.Yaw;
	
	//Lean用于角色移动倾斜的动画
	CharacterRotationLastFrame = CharacterRotation;
	CharacterRotation = BlasterCharacter->GetActorRotation();
	//获取两帧之间的旋转差值
	const FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotation, CharacterRotationLastFrame);
	//由于Delta值过小，因此将其放大
	const float Target = Delta.Yaw / DeltaTime;
	//FInterpTo一般在tick事件中使用。返回值为从“当前值”过渡到“期望的目标值”的一个中间值。参数：当前值；期望的目标值，时间变化值，插值速度
	const float Interp = FMath::FInterpTo(Lean, Target, DeltaTime, 6.f);
	//把Interp的值限制在-90到90
	Lean = FMath::Clamp(Interp, -90.f, 90.f);

	AO_Yaw = BlasterCharacter->GetAO_Yaw();
	AO_Pitch = BlasterCharacter->GetAO_Pitch();

	if(bWeaponEquipped && EquippedWeapon && EquippedWeapon->GetWeaponMesh() && BlasterCharacter->GetMesh()){
		//获取武器上的名为LeftHandSocket的在世界空间上的transform信息
		LeftHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("LeftHandSocket"), ERelativeTransformSpace::RTS_World);
		//将一个位置和旋转从世界空间转换到骨骼空间。这个函数在需要将世界空间的变换应用到骨骼空间，并且是相对于右手的骨骼空间位置. 注：骨骼名不区分大小写.
		//参数：表示要转换的骨骼的名称,转换前的位置，转换前的旋转（这里用0代替），转换后的位置（引用），转换后的旋转（引用）
		FVector OutPosition;
		FRotator OutRotation;
		BlasterCharacter->GetMesh()->TransformToBoneSpace(FName("hand_r"), LeftHandTransform.GetLocation(),FRotator::ZeroRotator, OutPosition, OutRotation);
		//得到最终左手插槽的骨骼空间的位置
		LeftHandTransform.SetLocation(OutPosition);
		LeftHandTransform.SetRotation(FQuat(OutRotation));
		
		////获取枪口世界位置
		//FTransform MuzzleTipTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("MuzzleFlash"), ERelativeTransformSpace::RTS_World);
		////获取枪口尖端变换的x轴方向
		//FVector  MuzzleX(FRotationMatrix(MuzzleTipTransform.GetRotation().Rotator()).GetUnitAxis(EAxis::X));
		////绘制调试射线 并于看清枪口朝向
		//DrawDebugLine(GetWorld(), MuzzleTipTransform.GetLocation(), MuzzleTipTransform.GetLocation()+MuzzleX*1000.f,FColor::Red);
		////绘制射线检测结果，用于看清屏幕中心与碰撞物体间的射线，便于与上面的枪口射线比较，以便我们进一步调整
		//DrawDebugLine(GetWorld(), MuzzleTipTransform.GetLocation(), BlasterCharacter->GetHitTarget(),FColor::Orange);
		
		//只是用于本地角色视角方面更为好看，不需要复制到各个客户端，因此只在本地调整即可
		if (BlasterCharacter->IsLocallyControlled()) 
		{
			bLocallyControlled = true;
			//通过调整右手插槽位置来改变枪口朝向，所以获取右手插槽
			FTransform RightHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("Hand_R"), ERelativeTransformSpace::RTS_World);
			//右手骨骼朝向调整到与命中目标朝向的旋转差值。参数：起始向量，目标向量。返回与旋转差值
			//RightHandTransform.GetLocation() - BlasterCharacter->GetHitTarget() 右手位置减去命中位置得到向量，该向量为从命中目标到右手位置的向量，如果再加上RightHandTransform.GetLocation()，则会得到从右手位置开始沿着命中目标的相反方向的反向运动。
			//上一行主要说明的就是为了解决调整时，枪管被调整为了朝正确方向的反方向的情况
			//(被弃用)RightHandRotation = UKismetMathLibrary::FindLookAtRotation(RightHandTransform.GetLocation(), RightHandTransform.GetLocation() + (RightHandTransform.GetLocation() - BlasterCharacter->GetHitTarget()));
			
			//让武器插值改变角度，避免突兀
			FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(RightHandTransform.GetLocation(), RightHandTransform.GetLocation() + (RightHandTransform.GetLocation() - BlasterCharacter->GetHitTarget()));
			RightHandRotation = FMath::RInterpTo(RightHandRotation, LookAtRotation, DeltaTime, 30.f);
		}
	}
	//如果在空闲状态，则打开IK
	bUseFABRIK = BlasterCharacter->GetCombatState() == ECombatState::ECS_Unoccupied;
	//在空闲状态，可以瞄准偏移（后者条件是游戏是否处于禁止输入状态
	bUseAimOffsets = BlasterCharacter->GetCombatState() == ECombatState::ECS_Unoccupied && !BlasterCharacter->GetDisableGameplay();
	//在空闲状态，可以右手变换（后者条件是游戏是否处于禁止输入状态
	bTransformRightHand = BlasterCharacter->GetCombatState() == ECombatState::ECS_Unoccupied && !BlasterCharacter->GetDisableGameplay();
}
 