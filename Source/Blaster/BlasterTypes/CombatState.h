#pragma once

/// <summary>
/// ս�����״̬
/// </summary>
UENUM(BlueprintType)
enum class ECombatState : uint8
{
	/// <summary>
	/// ����״̬�����Խ��п�����Ϊ��
	/// </summary>
	ECS_Unoccupied UMETA(DisplayName = "Unoccupied"),
	/// <summary>
	/// ����״̬
	/// </summary>
	ECS_Reloading UMETA(DisplayName = "Reloading"),

	/// <summary>
	/// ö������
	/// </summary>
	ECS_MAX UMETA(DisplayName = "DefaultMAX")
};