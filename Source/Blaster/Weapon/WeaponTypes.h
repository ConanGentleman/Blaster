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
	/// 手枪
	/// </summary>
	EWT_Pistol UMETA(DisplayName = "Pistol"),
	/// <summary>
	/// 冲锋枪
	/// </summary>
	EWT_SubmachineGun UMETA(DisplayName = "Submachine Gun"),

	/// <summary>
	/// 枚举数量
	/// </summary>
	EWT_MAX UMETA(DisplayName = "DefaultMAX")
};