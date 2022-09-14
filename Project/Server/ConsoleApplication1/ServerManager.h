#pragma once

#include "ServerDefine.h"

class IOCPThread;
class GameManager;

/// <summary>
/// ������ �� ������
/// 2021. 05. 10
/// ������
/// </summary>
class ServerManager
{
public:
	ServerManager();
	~ServerManager();

private:
	static SOCKET			m_ListenSock;		// listen ����
	CRITICAL_SECTION        m_CS_SocketList;	// �Ʒ� ����Ʈ���� ���� ũ��Ƽ�� ����
	std::list< SSocket* >	m_SocketList;		// ����� Ŭ���̾�Ʈ ������

	HANDLE                  m_hIOCP;			// Input/Output Completion Port( ����� �Ϸ� ��Ʈ, ���������� Queue �� ���� ) �ڵ�
	static BOOL             m_bExit;			// �� ������ ���� �÷���
	
	IOCPThread*				m_IOCPThread;		// ��Ŀ ������
	GameManager*			m_GameManager;		// ���� �Ŵ���

private:
	static void SignalFunction(int iSignalNumber);
	static int CALLBACK AcceptCondition(LPWSABUF lpCallerId, LPWSABUF lpCallerData, LPQOS lpSQOS, LPQOS lpGQOS, LPWSABUF lpCalleeId, LPWSABUF lpCalleeData, GROUP FAR* g, DWORD_PTR dwCallbackData);
	BOOL AddSocket(SSocket* psSocket);

public:
	bool Initialize();
	void Release();
	int Run();

	BOOL SendPacket(SSocket* psSocket, SHeader* psPacket);
	BOOL SendPacketToRelativePlayer(SSocket* psSocket, SHeader* psPacket);
	void BroadcastPacket(SHeader* psPacket);

	BOOL RemoveSocket(SOCKET socket);
	BOOL Receive(SSocket* psSocket, SOverlapped* psOverlapped = nullptr);	// Ŭ���̾�Ʈ�κ��� ��Ŷ ���� �������� �ɱ�
};
