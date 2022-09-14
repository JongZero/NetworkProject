#pragma once
#include "ServerDefine.h"
#include "BaseThread.h"

class IOCPThread;
class GameManager;

/// <summary>
/// Ŭ�� ������ �����Ŵ
/// 2021. 05. 10
/// ������
/// </summary>
class ServerManager : public BaseThread
{
private:
	ServerManager();
	virtual ~ServerManager();

private:
	static ServerManager* m_Instance;

private:
	SSocket*				m_psSocket;

	HANDLE                  m_hIOCP;			// Input/Output Completion Port( ����� �Ϸ� ��Ʈ, ���������� Queue �� ���� ) �ڵ�
	static BOOL             m_bExit;			// �� ������ ���� �÷���

	IOCPThread* m_IOCPThread;		// ��Ŀ ������

public:
	static ServerManager* GetInstance();
	void DeleteInstance();

private:
	virtual void			Begin();
	virtual void			End();
	virtual unsigned int	Run(int dwIndex);

private:
	static void SignalFunction(int iSignalNumber);

public:
	bool Initialize();
	void Release();

	BOOL SendPacket(SHeader* psPacket);
	BOOL Receive(SOverlapped* psOverlapped = nullptr);	// Ŭ���̾�Ʈ�κ��� ��Ŷ ���� �������� �ɱ�
};
