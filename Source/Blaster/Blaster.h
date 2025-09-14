// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

//在项目设置中自定义的object通道类型 在这里定义为ECC_SkeletalMesh。因为他不会自动去定义为我在设置里面同名的通道
//这样就是更具描述性，不然使用的时候使用的是ECollisionChannel::ECC_GameTraceChannel1 根本不知道是什么
#define ECC_SkeletalMesh ECollisionChannel::ECC_GameTraceChannel1

#define ECC_HitBox ECollisionChannel::ECC_GameTraceChannel2