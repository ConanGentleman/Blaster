// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

//����Ŀ�������Զ����objectͨ������ �����ﶨ��ΪECC_SkeletalMesh����Ϊ�������Զ�ȥ����Ϊ������������ͬ����ͨ��
//�������Ǹ��������ԣ���Ȼʹ�õ�ʱ��ʹ�õ���ECollisionChannel::ECC_GameTraceChannel1 ������֪����ʲô
#define ECC_SkeletalMesh ECollisionChannel::ECC_GameTraceChannel1

#define ECC_HitBox ECollisionChannel::ECC_GameTraceChannel2