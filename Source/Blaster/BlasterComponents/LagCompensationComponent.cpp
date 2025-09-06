// Fill out your copyright notice in the Description page of Project Settings.


#include "LagCompensationComponent.h"
#include "Blaster/Character/BlasterCharacter.h"
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
		InterpBoxInfo.BoxExtent = YoungerBox.BoxExtent;

		InterpFramePackage.HitBoxInfo.Add(BoxInfoName, InterpBoxInfo);
	}

	return InterpFramePackage;
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
void ULagCompensationComponent::ServerSideRewind(ABlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, float HitTime)
{
	bool bReturn =
		HitCharacter == nullptr ||
		HitCharacter->GetLagCompensation() == nullptr ||//获取延迟补偿组件
		HitCharacter->GetLagCompensation()->FrameHistory.GetHead() == nullptr ||//获取延迟补偿组件帧数据头
		HitCharacter->GetLagCompensation()->FrameHistory.GetTail() == nullptr;//获取延迟补偿组件帧数据尾部
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
		return;
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
	}

	if (bReturn) return;
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