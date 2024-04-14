#pragma once

/// <summary>
/// 角色当前转向状态的枚举 用于相机转向临界度数后，需要将人物的朝向进行调整
/// </summary>
UENUM(BlueprintType)
enum class ETurningInPlace : uint8
{
	/// <summary>
	/// 人物左转向
	/// </summary>
	ETIP_Left UMETA(DisplayName = "Turning Left"),
	/// <summary>
	/// 人物右转向
	/// </summary>
	ETIP_Right UMETA(DisplayName = "Turning Right"),
	/// <summary>
	/// 人物未转向
	/// </summary>
	ETIP_NotTurning UMETA(DisplayName = "Not Turning"),
	/// <summary>
	/// 用于获取转向状态的种类个数
	/// </summary>
	ETIP_MAX UMETA(DisplayName = "DefaultMAX")
};

