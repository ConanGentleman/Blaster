// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileBullet.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"

/// <summary>
/// 重写子弹父类中的子弹碰撞函数。父类中的子弹碰撞函数已经完成了子弹销毁等功能。
/// 这里主要调用伤害函数。
/// </summary>
void AProjectileBullet::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	//获取子弹的所有者（ 在combatcomponent.cpp中，将装备武器的所有者设置为了角色）
	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (OwnerCharacter)
	{
		//获取控制器
		AController* OwnerController = OwnerCharacter->Controller;
		if (OwnerController)
		{
			//命中时施加伤害，使用ue自带的伤害函数。参数：受伤害的actor，伤害值，造成此伤害的控制器（例如射击武器的玩家），实际造成伤害的actor，描述所造成上海的类
			UGameplayStatics::ApplyDamage(OtherActor, Damage, OwnerController, this, UDamageType::StaticClass());
		}
	}
	//由于父类中是对子弹进行销毁，因此将父类的子弹碰撞函数调用放在最后，保证正常运行。
	Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);
}