#pragma once

/// <summary>
/// 战斗组件状态
/// </summary>
UENUM(BlueprintType)
enum class ECombatState : uint8
{
	/// <summary>
	/// 空闲状态（可以进行开火行为）
	/// </summary>
	ECS_Unoccupied UMETA(DisplayName = "Unoccupied"),
	/// <summary>
	/// 换弹状态
	/// </summary>
	ECS_Reloading UMETA(DisplayName = "Reloading"),
	/// <summary>
	/// 投掷手榴弹状态
	/// </summary>
	ECS_ThrowingGrenade UMETA(DisplayName = "Throwing Grenade"),
	/// <summary>
	/// 切换武器状态
	/// </summary>
	ECS_SwappingWeapons UMETA(DisplayName = "Swapping Weapons"),


	/// <summary>
	/// 枚举数量
	/// </summary>
	ECS_MAX UMETA(DisplayName = "DefaultMAX")
};