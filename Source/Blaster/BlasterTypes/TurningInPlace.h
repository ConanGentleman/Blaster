#pragma once

/// <summary>
/// ��ɫ��ǰת��״̬��ö�� �������ת���ٽ��������Ҫ������ĳ�����е���
/// </summary>
UENUM(BlueprintType)
enum class ETurningInPlace : uint8
{
	/// <summary>
	/// ������ת��
	/// </summary>
	ETIP_Left UMETA(DisplayName = "Turning Left"),
	/// <summary>
	/// ������ת��
	/// </summary>
	ETIP_Right UMETA(DisplayName = "Turning Right"),
	/// <summary>
	/// ����δת��
	/// </summary>
	ETIP_NotTurning UMETA(DisplayName = "Not Turning"),
	/// <summary>
	/// ���ڻ�ȡת��״̬���������
	/// </summary>
	ETIP_MAX UMETA(DisplayName = "DefaultMAX")
};

