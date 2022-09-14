#include "ServerManager.h"

#include "ErrManager.h"
#include <signal.h>
#include <process.h>
#include <vector>
#include <assert.h>

#include "IOCPThread.h"
#include "GameManager.h"

BOOL ServerManager::m_bExit = FALSE;
SOCKET ServerManager::m_ListenSock = INVALID_SOCKET;

ServerManager::ServerManager()
	: m_hIOCP(nullptr), m_IOCPThread(nullptr), m_CS_SocketList(), m_GameManager(nullptr)
{

}

ServerManager::~ServerManager()
{
	
}

void ServerManager::SignalFunction(int iSignalNumber)
{
	if (SIGINT == iSignalNumber)
	{
		// ���� �÷��� ��
		m_bExit = TRUE;

		// for main()
		if (INVALID_SOCKET != m_ListenSock)
		{
			closesocket(m_ListenSock);
			m_ListenSock = INVALID_SOCKET;
		}
	}
}

int CALLBACK ServerManager::AcceptCondition(LPWSABUF lpCallerId, LPWSABUF lpCallerData, LPQOS lpSQOS, LPQOS lpGQOS, LPWSABUF lpCalleeId, LPWSABUF lpCalleeData, GROUP FAR* g, DWORD_PTR dwCallbackData)
{
	// �ִ� ���� ������ �������� ����
	if (MAX_USER_COUNT == 0)
	{
		return CF_ACCEPT;   // ���� ����
	}

	// WSAAccept() �� ������ ���ڿ� �־��� ��
	// �� ���������� �ִ� ���� �������� �����ϱ� ���ؼ� g_listSockets ������ �ּҸ� �Ѱ��.
	const std::list< SSocket* >* const cpclistClients = reinterpret_cast<std::list< SSocket* > *>(dwCallbackData);

	// �ִ� ���� ������ ����
	if (cpclistClients->size() >= MAX_USER_COUNT)
	{
		return CF_REJECT;   // ���� ����
	}

	return CF_ACCEPT;       // ���� ����
}

BOOL ServerManager::AddSocket(SSocket* psSocket)
{
	assert(nullptr != psSocket);

	// Ŭ���̾�Ʈ ������ list �� �߰�

	{	// �Ӱ� ����
		Lock _lock(&m_CS_SocketList);

		// Ŭ���̾�Ʈ �ߺ� �˻�
		for (auto it : m_SocketList)
		{
			if (it == psSocket)
			{
				// Leave Critical Section
				_lock.Unlock();
				return FALSE;
			}
		}

		// Ŭ���̾�Ʈ ���
		m_SocketList.push_back(psSocket);
	}

	return TRUE;
}

BOOL ServerManager::RemoveSocket(SOCKET socket)
{
	assert(INVALID_SOCKET != socket);

	// �Ӱ� ����
	{
		Lock _lock(&m_CS_SocketList);

		// Ŭ���̾�Ʈ ���� �� ����
		for (auto it = m_SocketList.begin(); it != m_SocketList.end(); ++it)
		{
			SSocket* psSocket = *it;

			if (psSocket->socket == socket)
			{
				// �÷��̾� �� ���� �������Ƿ� ������ �����Ű�� ���� ���� �� ���� Ŭ���̾�Ʈ���� ��Ŷ�� �����ش�.
				S2C_GameState endPacket;

				endPacket.bIsGameStart = false;
				endPacket.bIsGameWin = true;
				SendPacketToRelativePlayer(psSocket, &endPacket);

				m_SocketList.erase(it);

				// Leave Critical Section
				_lock.Unlock();

				printf_s("[TCP ����] [%15s:%5d] Ŭ���̾�Ʈ�� ����\n", psSocket->strIP.c_str(), psSocket->usPort);

				// shutdown()�� ȣ���� ���� ���� ������ �����Ų��.
				shutdown(psSocket->socket, SD_BOTH);

				if (!m_GameManager->IsGameStart())
					m_GameManager->LeavePlayer(socket);

				// ������ ī��Ʈ�� 0�̶�� delete (close socket)�� �Ѵ�.
				if (psSocket->useCount == 0)
				{
					delete psSocket;
				}
				// 0�� �ƴ϶�� ���� �������� ��û�� ���� �Ϸ� ó���� �ȵ� ���̹Ƿ� ���� �� �Ϸ� ó�� �� �ݱ� ���� Ȯ�ο� ������ true�� �ٲ��ش�.
				else
				{
					psSocket->checkClose = true;
				}

				return TRUE;
			}
		}
	}

	return FALSE;
}

BOOL ServerManager::Receive(SSocket* psSocket, SOverlapped* psOverlapped /*= nullptr*/)
{
	assert(nullptr != socket);

	// ����� �������带 ���� �ʾ����� ����
	if (nullptr == psOverlapped)
	{
		psOverlapped = new SOverlapped;
	}

	// �������� ����
	psOverlapped->eIOType = SOverlapped::EIOType::EIOType_Recv;
	psOverlapped->socket = psSocket->socket;

	// WSABUF ����
	WSABUF wsaBuffer;
	wsaBuffer.buf = psOverlapped->szBuffer + psOverlapped->iDataSize;
	wsaBuffer.len = sizeof(psOverlapped->szBuffer) - psOverlapped->iDataSize;

	// WSARecv() �������� �ɱ�
	DWORD dwNumberOfBytesRecvd = 0, dwFlag = 0;

	int iResult = WSARecv(psOverlapped->socket,
		&wsaBuffer,
		1,
		&dwNumberOfBytesRecvd,
		&dwFlag,
		&psOverlapped->wsaOverlapped,
		nullptr);

	if ((SOCKET_ERROR == iResult) && (WSAGetLastError() != WSA_IO_PENDING))
	{
		char szBuffer[BUFSIZE] = { 0, };
		sprintf_s(szBuffer, "[TCP ����] ���� �߻� -- WSARecv() :");
		ErrManager::ErrDisplay(szBuffer);

		delete psOverlapped;
		return FALSE;
	}

	{   // �Ӱ� ����
		Lock _lock(&psSocket->CS_useCount);

		// ó���Ǿ��ٸ� ī��Ʈ ����
		psSocket->useCount++;
	}

	return TRUE;
}

BOOL ServerManager::SendPacket(SSocket* psSocket, SHeader* psPacket)
{
	assert(nullptr != psSocket);
	assert(nullptr != psPacket);

	// ��ϵ��� ���� ��Ŷ�� ������ �� ����.
	// [����] -> [Ŭ��]
	if (ES2C_TYPE_MAX <= psPacket->usType)
	{
		return FALSE;
	}

	// �������� ����
	SOverlapped* psOverlapped = new SOverlapped;
	psOverlapped->eIOType = SOverlapped::EIOType::EIOType_Send;
	psOverlapped->socket = psSocket->socket;

	// ��Ŷ ����
	psOverlapped->iDataSize = 2 + psPacket->usSize;

	if (sizeof(psOverlapped->szBuffer) < psOverlapped->iDataSize)
	{
		delete psOverlapped;
		return FALSE;
	}

	memcpy_s(psOverlapped->szBuffer, sizeof(psOverlapped->szBuffer), psPacket, psOverlapped->iDataSize);

	// WSABUF ����
	WSABUF wsaBuffer;
	wsaBuffer.buf = psOverlapped->szBuffer;
	wsaBuffer.len = psOverlapped->iDataSize;

	// WSASend() �������� �ɱ�
	DWORD dwNumberOfBytesSent = 0;

	int iResult = WSASend(psOverlapped->socket,
		&wsaBuffer,
		1,
		&dwNumberOfBytesSent,
		0,
		&psOverlapped->wsaOverlapped,
		nullptr);

	if ((SOCKET_ERROR == iResult) && (WSAGetLastError() != WSA_IO_PENDING))
	{
		char szBuffer[BUFSIZE] = { 0, };
		sprintf_s(szBuffer, "[TCP ����] ���� �߻� -- WSASend() :");
		ErrManager::ErrDisplay(szBuffer);

		delete psOverlapped;
		return FALSE;
	}

	{   // �Ӱ� ����
		Lock _lock(&psSocket->CS_useCount);

		// ó���Ǿ��ٸ� ī��Ʈ ����
		psSocket->useCount++;
	}

	return TRUE;
}

BOOL ServerManager::SendPacketToRelativePlayer(SSocket* psSocket, SHeader* psPacket)
{
	assert(nullptr != psSocket);
	assert(nullptr != psPacket);

	// ��ϵ��� ���� ��Ŷ�� ������ �� ����.
	// [����] -> [Ŭ��]
	if (ES2C_TYPE_MAX <= psPacket->usType)
	{
		return FALSE;
	}

	// �������� ����
	SOverlapped* psOverlapped = new SOverlapped;
	psOverlapped->eIOType = SOverlapped::EIOType::EIOType_Send;

	// ��Ŷ�� �� �÷��̾ �ƴ� ��� �÷��̾�Ը� ������.
	psOverlapped->socket = m_GameManager->GetRelativePlayer(psSocket->socket);

	// ��Ŷ ����
	psOverlapped->iDataSize = 2 + psPacket->usSize;

	if (sizeof(psOverlapped->szBuffer) < psOverlapped->iDataSize)
	{
		delete psOverlapped;
		return FALSE;
	}

	memcpy_s(psOverlapped->szBuffer, sizeof(psOverlapped->szBuffer), psPacket, psOverlapped->iDataSize);

	// WSABUF ����
	WSABUF wsaBuffer;
	wsaBuffer.buf = psOverlapped->szBuffer;
	wsaBuffer.len = psOverlapped->iDataSize;

	// WSASend() �������� �ɱ�
	DWORD dwNumberOfBytesSent = 0;

	int iResult = WSASend(psOverlapped->socket,
		&wsaBuffer,
		1,
		&dwNumberOfBytesSent,
		0,
		&psOverlapped->wsaOverlapped,
		nullptr);

	if ((SOCKET_ERROR == iResult) && (WSAGetLastError() != WSA_IO_PENDING))
	{
		char szBuffer[BUFSIZE] = { 0, };
		sprintf_s(szBuffer, "[TCP ����] ���� �߻� -- WSASend() :");
		ErrManager::ErrDisplay(szBuffer);

		delete psOverlapped;
		return FALSE;
	}

	// ����÷��̾ ã�´�.
	for (auto it : m_SocketList)
	{
		if (it != psSocket)
		{
			psSocket = it;
			break;
		}
	}

	{   // �Ӱ� ����
		Lock _lock(&psSocket->CS_useCount);

		// ó���Ǿ��ٸ� ī��Ʈ ����
		psSocket->useCount++;
	}

	return TRUE;
}

void ServerManager::BroadcastPacket(SHeader* psPacket)
{
	assert(nullptr != psPacket);

	// �Ӱ� ����
	{
		Lock _lock(&m_CS_SocketList);

		// ��ü Ŭ���̾�Ʈ ���Ͽ� ��Ŷ �۽�
		for (auto it : m_SocketList)
		{
			// ��Ŷ �۽�
			SendPacket(it, psPacket);
		}
	}
}

bool ServerManager::Initialize()
{
	// Ctrl - C �� ������,
   // �� �����带 ������ ��� ��������� ���� ���Ḧ �� �� �ֵ��� ��
	signal(SIGINT, ServerManager::SignalFunction);

	char szBuffer[BUFSIZE] = { 0, };

	// ���� �ʱ�ȭ
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		sprintf_s(szBuffer, "[TCP ����] ���� �߻� -- WSAStartup() :");
		ErrManager::ErrDisplay(szBuffer);

		return false;
	}

	// 1. WSASocket()
	m_ListenSock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, WSA_FLAG_OVERLAPPED);

	if (INVALID_SOCKET == m_ListenSock)
	{
		sprintf_s(szBuffer, "[TCP ����] ���� �߻� -- WSASocket() :");
		ErrManager::ErrDisplay(szBuffer);

		WSACleanup();
		return false;
	}

	// 2. bind()
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(PORT);
	if (SOCKET_ERROR == bind(m_ListenSock, reinterpret_cast<SOCKADDR*>(&serveraddr), sizeof(serveraddr)))
	{
		sprintf_s(szBuffer, "[TCP ����] ���� �߻� -- bind() :");
		ErrManager::ErrDisplay(szBuffer);

		closesocket(m_ListenSock);
		m_ListenSock = INVALID_SOCKET;

		WSACleanup();
		return false;
	}

	// 3. listen()
	if (SOCKET_ERROR == listen(m_ListenSock, 5))
	{
		sprintf_s(szBuffer, "[TCP ����] ���� �߻� -- listen() :");
		ErrManager::ErrDisplay(szBuffer);

		closesocket(m_ListenSock);
		m_ListenSock = INVALID_SOCKET;

		WSACleanup();
		return false;
	}

	// ũ��Ƽ�� ���� �ʱ�ȭ
	InitializeCriticalSection(&m_CS_SocketList);

	// 4. IOCP ����
	m_hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 0);
	assert(nullptr != m_hIOCP);

	/// ���� �Ŵ��� ����
	m_GameManager = new GameManager(this);
	m_GameManager->Create();

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// 5. WorkerThread() �� ����
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// IOCP �� ������ Ǯ�� ������ �������� [�ھ��] ~ [�ھ�� * 2] ����
	SYSTEM_INFO SystemInfo;
	GetSystemInfo(&SystemInfo);
	int iThreadCount = SystemInfo.dwNumberOfProcessors * 2;

	// IOCP �����带 iThreadCount ��ŭ ����
	m_IOCPThread = new IOCPThread(this, m_GameManager, m_hIOCP);
	m_IOCPThread->Create(CREATE_SUSPENDED, iThreadCount);

	printf_s("[TCP ����] ����\n");
	return true;
}

void ServerManager::Release()
{
	S2C_ServerDown packet;
	packet.bIsServerDown = true;
	BroadcastPacket(&packet);

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// WorkerThread() ���� ó�� �� ���
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// WorkerThread() ���� ���Ḧ ���� ���� ��ȣ
	PostQueuedCompletionStatus(m_hIOCP, 0, 0, nullptr);

	// ������ ����
	m_IOCPThread->Destroy();

	// ���� �Ŵ��� ����
	m_GameManager->Destroy();

	// IOCP ����
	CloseHandle(m_hIOCP);
	m_hIOCP = nullptr;
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	for (auto it : m_SocketList)
	{
		delete it;
	}
	m_SocketList.clear();

	// ���� ����
	WSACleanup();

	// ũ��Ƽ�� ���� ����
	DeleteCriticalSection(&m_CS_SocketList);

	printf_s("[TCP ����] ����\n");
}

int ServerManager::Run()
{
	while (!m_bExit)
	{
		// 6. WSAAccept()
		SOCKADDR_IN clientaddr;
		int addrlen = sizeof(clientaddr);
		ZeroMemory(&clientaddr, addrlen);
		SOCKET socket = WSAAccept(m_ListenSock, reinterpret_cast<SOCKADDR*>(&clientaddr), &addrlen, ServerManager::AcceptCondition, reinterpret_cast<DWORD_PTR>(&m_SocketList));

		if (INVALID_SOCKET == socket)
		{
			// �������� ���� �ź�( AcceptCondition() )�� �� ���� ���� ����� ���ϰ� ��
			if (WSAGetLastError() != WSAECONNREFUSED)
			{
				char szBuffer[BUFSIZE] = { 0, };
				sprintf_s(szBuffer, "[TCP ����] ���� �߻� -- WSAAccept() :");
				ErrManager::ErrDisplay(szBuffer);
			}

			continue;
		}

		// Ŭ���̾�Ʈ ����ü ���� �� ���
		SSocket* psSocket = new SSocket;
		psSocket->socket = socket;
		psSocket->strIP = inet_ntoa(clientaddr.sin_addr);
		psSocket->usPort = ntohs(clientaddr.sin_port);

		// 7. ������ IOCP �� Ű ���� �Բ� ���
		if (CreateIoCompletionPort(reinterpret_cast<HANDLE>(psSocket->socket), m_hIOCP, reinterpret_cast<ULONG_PTR>(psSocket), 0) != m_hIOCP)
		{
			delete psSocket;
			continue;
		}

		// ����� Ŭ���̾�Ʈ ������ ���
		if (!AddSocket(psSocket))
		{
			delete psSocket;
			continue;
		}

		// �÷��̾�� �÷��̾��� ���� �� �������� �����ش�.
		S2C_PlayerNum playerNum;
		playerNum.usPlayerNum = m_GameManager->GetPlayerCount();
		SendPacket(psSocket, &playerNum);

		printf_s("[TCP ����] [%15s:%5d] �÷��̾�%d ����\n", psSocket->strIP.c_str(), psSocket->usPort, m_GameManager->GetPlayerCount());
		m_GameManager->EnterPlayer(psSocket->socket);

		// �÷��̾ ��� �����ϸ� ���� ����
		if (m_GameManager->GetPlayerCount() == MAX_USER_COUNT)
		{
			printf_s("[TCP ����] �÷��̾ ��� �����Ͽ� ������ �����մϴ�.\n");
			m_GameManager->Resume();
		}

		// 8. WSARecv() �ɱ�
		if (!Receive(psSocket))
		{
			// Ŭ���̾�Ʈ ����
			RemoveSocket(psSocket->socket);
			continue;
		}
	}

	return 0;
}
