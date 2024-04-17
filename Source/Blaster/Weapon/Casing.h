// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Casing.generated.h"

UCLASS()
class BLASTER_API ACasing : public AActor
{
	GENERATED_BODY()
	
public:	
	ACasing();

protected:
	virtual void BeginPlay() override;

	/// <summary>
	/// 弹壳碰撞函数回调（主要是弹壳落地的碰撞
	/// </summary>
	/// https://dev.epicgames.com/documentation/zh-cn/unreal-engine/events-in-unreal-engine
	/// <param name="HitComp">进行碰撞的组件（这里就是CollisionBox）</param>
	/// <param name="OtherActor">参与碰撞的其他 Actor</param>
	/// <param name="OtherComp">被击中的另一个组件</param>
	/// <param name="NormalImpulse">Actor 碰撞的力/param>
	/// <param name="Hit">击中结果</param>
	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

private:
	/// <summary>
	/// 弹壳网格体（弹壳模型）
	/// </summary>
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* CasingMesh;

	/// <summary>
	/// 弹壳生成时给予的力
	/// </summary>
	UPROPERTY(EditAnywhere)
	float ShellEjectionImpulse;

	/// <summary>
	/// 弹壳掉落的音效
	/// </summary>
	UPROPERTY(EditAnywhere)
	class USoundCue* ShellSound;

//public:	
//	virtual void Tick(float DeltaTime) override;

};
