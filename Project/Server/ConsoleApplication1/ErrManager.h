#pragma once

/// <summary>
/// ���� �޽����� ������ִ� Ŭ����
/// 2021. 05. 10
/// ������
/// </summary>
class ErrManager
{
public:
	ErrManager() {};
	~ErrManager() {};

	// ���� �Լ� ���� ���
	static void ErrDisplay(const char* const cpcMSG);
};
