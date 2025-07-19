#pragma once

//射线检测的距离长度
#define TRACE_LENGTH 80000.f

//深度颜色，用于场地里未拾取的武器描边
#define CUSTOM_DEPTH_PURPLE 250
#define CUSTOM_DEPTH_BLUE 251
#define CUSTOM_DEPTH_TAN 252

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
	/// 冲锋枪
	/// </summary>
	EWT_Shotgun UMETA(DisplayName = "Shotgun"),
	/// <summary>
	/// 狙击枪
	/// </summary>
	EWT_SniperRifle UMETA(DisplayName = "Sniper Rifle"),
	/// <summary>
	/// 榴弹发射器
	/// </summary>
	EWT_GrenadeLauncher UMETA(DisplayName = "Grenade Launcher"),

	/// <summary>
	/// 枚举数量
	/// </summary>
	EWT_MAX UMETA(DisplayName = "DefaultMAX")
};