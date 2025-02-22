#pragma once

/// <summary>
/// 武器类型枚举
/// </summary>
UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	/// <summary>
	/// 突击步枪
	/// </summary>
	EWT_AssaultRifle UMETA(DisplayName = "Assault Rifle"),
	/// <summary>
	/// 火箭筒
	/// </summary>
	EWT_RocketLauncher UMETA(DisplayName = "Rocket Launcher"),

	/// <summary>
	/// 枚举数量
	/// </summary>
	EWT_MAX UMETA(DisplayName = "DefaultMAX")
};