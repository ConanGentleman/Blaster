// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InteractWithCrosshairsInterface.generated.h"

///UE4 Interface原理与使用 https://zhuanlan.zhihu.com/p/60851912
//ue文档 https://dev.epicgames.com/documentation/zh-cn/unreal-engine/interfaces-in-unreal-engine
// This class does not need to be modified.
///接口类有助于确保一组（可能）不相关的类实现一组通用函数
UINTERFACE(MinimalAPI)
class UInteractWithCrosshairsInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class BLASTER_API IInteractWithCrosshairsInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
};
