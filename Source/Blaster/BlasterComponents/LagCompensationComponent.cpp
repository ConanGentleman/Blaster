// Fill out your copyright notice in the Description page of Project Settings.


#include "LagCompensationComponent.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Components/BoxComponent.h"
#include "Components/BoxComponent.h"
#include "DrawDebugHelpers.h"

ULagCompensationComponent::ULagCompensationComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

}

void ULagCompensationComponent::BeginPlay()
{
	Super::BeginPlay();
}

/// <summary>
/// 存储延迟补偿数据包
/// </summary>
/// <param name="Package"></param>
void ULagCompensationComponent::SaveFramePackage(FFramePackage& Package)
{
	Character = Character == nullptr ? Cast<ABlasterCharacter>(GetOwner()) : Character;
	if (Character)
	{
		Package.Time = GetWorld()->GetTimeSeconds(); //用于只会在服务器上调用该函数，因此这里获取的时间是服务器上的时间
		//遍历（包含所有碰撞块的）映射表并将相应信息存到数据包中
		for (auto& BoxPair : Character->HitCollisionBoxes)
		{
			FBoxInformation BoxInformation;
			BoxInformation.Location = BoxPair.Value->GetComponentLocation();
			BoxInformation.Rotation = BoxPair.Value->GetComponentRotation();
			BoxInformation.BoxExtent = BoxPair.Value->GetScaledBoxExtent();
			Package.HitBoxInfo.Add(BoxPair.Key, BoxInformation);//Key代表命中框(碰撞体）  命名大部分跟骨骼名一致
		}
	}
}

/// <summary>
/// 依据击中时间进行插值以计算命中框的位置、旋转等信息
/// </summary>
/// <param name="OlderFrame"></param>
/// <param name="YoungerFrame"></param>
/// <param name="HitTime"></param>
/// <returns></returns>
FFramePackage ULagCompensationComponent::InterpBetweenFrames(const FFramePackage& OlderFrame, const FFramePackage& YoungerFrame, float HitTime)
{
	const float Distance = YoungerFrame.Time - OlderFrame.Time;
	//计算插值百分比
	const float InterpFraction = FMath::Clamp((HitTime - OlderFrame.Time) / Distance, 0.f, 1.f);
	//插值结果存储变量
	FFramePackage InterpFramePackage;
	InterpFramePackage.Time = HitTime;
	//对每一个命中框进行插值
	for (auto& YoungerPair : YoungerFrame.HitBoxInfo)
	{
		const FName& BoxInfoName = YoungerPair.Key;

		const FBoxInformation& OlderBox = OlderFrame.HitBoxInfo[BoxInfoName];
		const FBoxInformation& YoungerBox = YoungerFrame.HitBoxInfo[BoxInfoName];

		FBoxInformation InterpBoxInfo;

		InterpBoxInfo.Location = FMath::VInterpTo(OlderBox.Location, YoungerBox.Location, 1.f, InterpFraction);
		InterpBoxInfo.Rotation = FMath::RInterpTo(OlderBox.Rotation, YoungerBox.Rotation, 1.f, InterpFraction);
		InterpBoxInfo.BoxExtent = YoungerBox.BoxExtent; //碰撞框大小不会变，所以取哪一个都可以

		InterpFramePackage.HitBoxInfo.Add(BoxInfoName, InterpBoxInfo);
	}

	return InterpFramePackage;
}


/// <summary>
/// 击中判定
/// </summary>
/// <param name="Package">判定的延迟补偿数据包</param>
/// <param name="HitCharacter">击中的玩家</param>
/// <param name="TraceStart">检测的起始位置</param>
/// <param name="HitLocation">击中的位置</param>
/// <returns></returns>
FServerSideRewindResult ULagCompensationComponent::ConfirmHit(const FFramePackage& Package, ABlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation)
{
	// 没有命中返回空
	if (HitCharacter == nullptr) return FServerSideRewindResult();

	//缓存击中的玩家最初的帧信息（便于判定击中成功后，将角色命中框移回最初位置）
	FFramePackage CurrentFrame;
	
	CacheBoxPositions(HitCharacter, CurrentFrame);
	//根据延迟补偿包将角色的命中框移动到包所对应的位置
	MoveBoxes(HitCharacter, Package);
	//先关闭所有角色网格的碰撞检测，避免对命中框的碰撞检测造成影响！！！！！！！！！！！！！！！！！！！！
	EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::NoCollision);

	// Enable collision for the head first 先开启头部的碰撞检测，如果未能命中再对其他部分进行检测
	UBoxComponent* HeadBox = HitCharacter->HitCollisionBoxes[FName("head")];
	//开启命中框的碰撞检测
	HeadBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	//追踪可见性碰撞通道
	HeadBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);

	FHitResult ConfirmHitResult;
	const FVector TraceEnd = TraceStart + (HitLocation - TraceStart) * 1.25f; //*1.25f是因为怕刚好在碰撞框表面无法触发碰撞，所以延长一下
	UWorld* World = GetWorld();
	if (World)
	{
		World->LineTraceSingleByChannel(
			ConfirmHitResult,
			TraceStart,
			TraceEnd,
			ECollisionChannel::ECC_Visibility
		);
		if (ConfirmHitResult.bBlockingHit) // we hit the head, return early 命中头部，则先返回击中结果
		{
			ResetHitBoxes(HitCharacter, CurrentFrame);//命中后将命中框重置回原来的位置
			//有命中框碰撞结果，复原角色网格的碰撞检测
			EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
			return FServerSideRewindResult{ true, true };
		}
		else // didn't hit head, check the rest of the boxes 如果头部没有命中，则对其他部位进行碰撞检测
		{
			for (auto& HitBoxPair : HitCharacter->HitCollisionBoxes)
			{
				if (HitBoxPair.Value != nullptr)
				{
					//开启命中框的碰撞检测
					HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
					HitBoxPair.Value->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
				}
			}
			World->LineTraceSingleByChannel(
				ConfirmHitResult,
				TraceStart,
				TraceEnd,
				ECollisionChannel::ECC_Visibility
			);
			if (ConfirmHitResult.bBlockingHit)
			{
				//命中后将命中框重置回原来的位置
				ResetHitBoxes(HitCharacter, CurrentFrame);
				//有命中框碰撞结果，复原角色网格的碰撞检测
				EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
				return FServerSideRewindResult{ true, false };
			}
		}
	}
	//将命中框重置回原来的位置
	ResetHitBoxes(HitCharacter, CurrentFrame);
	//复原角色网格的碰撞检测
	EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
	return FServerSideRewindResult{ false, false };
}

/// <summary>
/// 将HitCharacter中的命中框信息赋值给OutFramePackage
/// </summary>
/// <param name="HitCharacter"></param>
/// <param name="OutFramePackage"></param>
void ULagCompensationComponent::CacheBoxPositions(ABlasterCharacter* HitCharacter, FFramePackage& OutFramePackage)
{
	if (HitCharacter == nullptr) return;
	for (auto& HitBoxPair : HitCharacter->HitCollisionBoxes)
	{
		if (HitBoxPair.Value != nullptr)
		{
			FBoxInformation BoxInfo;
			BoxInfo.Location = HitBoxPair.Value->GetComponentLocation();
			BoxInfo.Rotation = HitBoxPair.Value->GetComponentRotation();
			BoxInfo.BoxExtent = HitBoxPair.Value->GetScaledBoxExtent();
			OutFramePackage.HitBoxInfo.Add(HitBoxPair.Key, BoxInfo);
		}
	}
}

/// <summary>
/// 移动所有命中框，即将Package中的命中框位置旋转等信息赋值给与HitCharacter的命中框
/// 与CacheBoxPositions函数作用相反
/// </summary>
/// <param name="HitCharacter"></param>
/// <param name="Package"></param>
void ULagCompensationComponent::MoveBoxes(ABlasterCharacter* HitCharacter, const FFramePackage& Package)
{
	if (HitCharacter == nullptr) return;
	for (auto& HitBoxPair : HitCharacter->HitCollisionBoxes)
	{
		if (HitBoxPair.Value != nullptr)
		{
			HitBoxPair.Value->SetWorldLocation(Package.HitBoxInfo[HitBoxPair.Key].Location);
			HitBoxPair.Value->SetWorldRotation(Package.HitBoxInfo[HitBoxPair.Key].Rotation);
			HitBoxPair.Value->SetBoxExtent(Package.HitBoxInfo[HitBoxPair.Key].BoxExtent);
		}
	}
}

void ULagCompensationComponent::ResetHitBoxes(ABlasterCharacter* HitCharacter, const FFramePackage& Package)
{
	if (HitCharacter == nullptr) return;
	for (auto& HitBoxPair : HitCharacter->HitCollisionBoxes)
	{
		if (HitBoxPair.Value != nullptr)
		{
			HitBoxPair.Value->SetWorldLocation(Package.HitBoxInfo[HitBoxPair.Key].Location);
			HitBoxPair.Value->SetWorldRotation(Package.HitBoxInfo[HitBoxPair.Key].Rotation);
			HitBoxPair.Value->SetBoxExtent(Package.HitBoxInfo[HitBoxPair.Key].BoxExtent);
			HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}
}

/// <summary>
/// 开启或关闭角色网格的碰撞检测(在倒带进行击中判定时需要关闭网格碰撞来避免影响到命中框命中的判定）
/// </summary>
/// <param name="HitCharacter"></param>
/// <param name="CollisionEnabled"></param>
void ULagCompensationComponent::EnableCharacterMeshCollision(ABlasterCharacter* HitCharacter, ECollisionEnabled::Type CollisionEnabled)
{
	if (HitCharacter && HitCharacter->GetMesh())
	{
		HitCharacter->GetMesh()->SetCollisionEnabled(CollisionEnabled);
	}
}


/// <summary>
/// 打印数据包中包含的命中狂（显示用，方便查看而已）
/// </summary>
/// <param name="Package"></param>
/// <param name="Color"></param>
void ULagCompensationComponent::ShowFramePackage(const FFramePackage& Package, const FColor& Color)
{
	for (auto& BoxInfo : Package.HitBoxInfo)
	{
		DrawDebugBox(
			GetWorld(),
			BoxInfo.Value.Location,
			BoxInfo.Value.BoxExtent,
			FQuat(BoxInfo.Value.Rotation),
			Color,
			false, //是否是持续性的（绘制出来不消失）
			4.f //多久消失
		);
	}
}

/// <summary>
/// 服务器延迟补偿倒带回放
/// </summary>
/// <param name="HitCharacter">击中的角色</param>
/// <param name="TraceStart">击中检测起始位置</param>
/// <param name="HitLocation">击中位置</param>
/// <param name="HitTime">击中时间</param>
FServerSideRewindResult ULagCompensationComponent::ServerSideRewind(ABlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, float HitTime)
{
	bool bReturn =
		HitCharacter == nullptr ||
		HitCharacter->GetLagCompensation() == nullptr ||//获取延迟补偿组件
		HitCharacter->GetLagCompensation()->FrameHistory.GetHead() == nullptr ||//获取延迟补偿组件帧数据头
		HitCharacter->GetLagCompensation()->FrameHistory.GetTail() == nullptr;//获取延迟补偿组件帧数据尾部
	if (bReturn) return FServerSideRewindResult();
	// Frame package that we check to verify a hit
	//用于返回最终倒带到历史的哪一帧
	FFramePackage FrameToCheck;
	bool bShouldInterpolate = true;
	// Frame history of the HitCharacter
	const TDoubleLinkedList<FFramePackage>& History = HitCharacter->GetLagCompensation()->FrameHistory;
	const float OldestHistoryTime = History.GetTail()->GetValue().Time;
	const float NewestHistoryTime = History.GetHead()->GetValue().Time;
	//如果击中时间低于了延迟补偿历史记录的最早时间（最旧的那组数据的时间），则直接不处理了
	if (OldestHistoryTime > HitTime)
	{
		// too far back - too laggy to do SSR (ServerSideRewind)
		// 太久了 - 延迟太高无法进行SSR
		return FServerSideRewindResult();
	}
	//如果击中时间正好等于了记录的最早时间（最旧的那组数据的时间），则直接取尾数据（最旧的那组数据）
	if (OldestHistoryTime == HitTime)
	{
		FrameToCheck = History.GetTail()->GetValue();
		bShouldInterpolate = false;
	}
	//如果击中时间早于了记录的最晚时间（最新的数据的时间），则直接取头数据（最新的数据）
	if (NewestHistoryTime <= HitTime)
	{
		FrameToCheck = History.GetHead()->GetValue();
		bShouldInterpolate = false;
	}

	TDoubleLinkedList<FFramePackage>::TDoubleLinkedListNode* Younger = History.GetHead();
	TDoubleLinkedList<FFramePackage>::TDoubleLinkedListNode* Older = Younger;
	//开始循环遍历，击中时间处于哪些数据之间（利用新旧指针）
	while (Older->GetValue().Time > HitTime) // is Older still younger than HitTime? 如果当前更早时间指针（旧指针） 指向的数据时间大于击中时间则继续往尾部遍历
	{
		// March back until: OlderTime < HitTime < YoungerTime
		//直到匹配到OlderTime < HitTime < YoungerTime结束
		if (Older->GetNextNode() == nullptr) break;
		Older = Older->GetNextNode();
		if (Older->GetValue().Time > HitTime)//如果更新旧指针后，指向的时间依旧大于击中时间，则更新新指针
		{
			Younger = Older;
		}
	}
	//如果击中时间正好等于了记录节点的时间，则不在需要插值
	if (Older->GetValue().Time == HitTime) // highly unlikely, but we found our frame to check
	{
		FrameToCheck = Older->GetValue();
		bShouldInterpolate = false;
	}
	//否则利用新旧指针进行插值
	if (bShouldInterpolate)
	{
		// Interpolate between Younger and Older
		FrameToCheck = InterpBetweenFrames(Older->GetValue(), Younger->GetValue(), HitTime);
	}

	return ConfirmHit(FrameToCheck, HitCharacter, TraceStart, HitLocation);
}

void ULagCompensationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (FrameHistory.Num() <= 1)
	{
		FFramePackage ThisFrame;
		SaveFramePackage(ThisFrame);
		FrameHistory.AddHead(ThisFrame);
	}
	else
	{
		float HistoryLength = FrameHistory.GetHead()->GetValue().Time - FrameHistory.GetTail()->GetValue().Time;
		//时间差过大的就不保留了
		while (HistoryLength > MaxRecordTime)
		{
			FrameHistory.RemoveNode(FrameHistory.GetTail());
			HistoryLength = FrameHistory.GetHead()->GetValue().Time - FrameHistory.GetTail()->GetValue().Time;
		}
		FFramePackage ThisFrame;
		SaveFramePackage(ThisFrame);
		FrameHistory.AddHead(ThisFrame);

		ShowFramePackage(ThisFrame, FColor::Red);
	}

}