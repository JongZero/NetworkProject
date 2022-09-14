#pragma once
#include "ServerDefine.h"

/// <summary>
/// �������� �θ� Ŭ����, ������ ����
/// 2021. 05. 10
/// ������
/// </summary>
class BaseThread
{
public:
	BaseThread();
	virtual ~BaseThread();

private:
	struct SThread
	{
		BaseThread* pThread;
		int				iIndex;
		unsigned int	uiThreadID;
		HANDLE			hHandle;
	};

protected:
	BOOL					m_bClose;			// ���� �÷���
	std::vector< SThread* >	m_ThreadVec;		// ������ ���� ������

private:
	static	unsigned int __stdcall ThreadFunction(void* pARG);

private:
	virtual void			Begin(void) {}
	virtual void			End(void) {}
	virtual unsigned int	Run(int dwIndex) = 0;	// �����尡 �� ������ ������ ����

public:
	size_t	Create(unsigned int uiThreadStat = CREATE_SUSPENDED, size_t nCount = 1);	// ������ ���� �� ���( ������ ������ �� ���� )
	size_t	Destroy(void);																// ������ ����( ����� ������ �� ���� )
	size_t	Resume(void);																// ������ �簳( �̹� ȣ��� ���� �簳�� ������ �� ���� )
	size_t	Suspend(void);
};