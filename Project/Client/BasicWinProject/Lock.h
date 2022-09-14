#pragma once

#include <Windows.h>

/// <summary>
/// Critical Section�� Enter, Leave�� ���, ������ �ߴ� �� ����
/// 2021. 05. 10
/// ������
/// </summary>
class Lock
{
public:
	Lock(CRITICAL_SECTION* cs) : m_CS(nullptr) { if (cs == nullptr) return; m_CS = cs; EnterCriticalSection(m_CS); };
	~Lock() { LeaveCriticalSection(m_CS); };

private:
	CRITICAL_SECTION* m_CS;

public:
	void Unlock() { LeaveCriticalSection(m_CS); }	// �Ҹ��ڿ��� LeaveCS�� ���ϴ� ��쵵 �ִ�.
};