#include "pch.h"
#include "ServerManager.h"
#include "ErrManager.h"
#include <signal.h>

#include "IOCPThread.h"

BOOL ServerManager::m_bExit = FALSE;
ServerManager* ServerManager::m_Instance = nullptr;

ServerManager* ServerManager::GetInstance()
{
	if (nullptr == m_Instance)
	{
		m_Instance = new ServerManager;
	}

	return m_Instance;
}

void ServerManager::DeleteInstance()
{
	if (m_Instance)
	{
		delete m_Instance;
		m_Instance = nullptr;
	}
}

ServerManager::ServerManager()
	: m_hIOCP(nullptr), m_IOCPThread(nullptr), m_psSocket(nullptr)
{

}

ServerManager::~ServerManager()
{
	DeleteInstance();
}

void ServerManager::SignalFunction(int iSignalNumber)
{
	if (SIGINT == iSignalNumber)
	{
		// ���� �÷��� ��
		m_bExit = TRUE;
	}
}

BOOL ServerManager::Receive(SOverlapped* psOverlapped)
{
	BOOL bRecycleOverlapped = TRUE;

	// ����� �������带 ���� �ʾ����� ����
	if (nullptr == psOverlapped)
	{
		psOverlapped = new SOverlapped;
		bRecycleOverlapped = FALSE;
	}

	// �������� ����
	psOverlapped->eIOType = SOverlapped::EIOType::EIOType_Recv;
	psOverlapped->socket = m_psSocket->socket;

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
		sprintf_s(szBuffer, "[TCP Ŭ���̾�Ʈ] ���� �߻� -- WSARecv() :");
		ErrManager::ErrDisplay(szBuffer);

		if (!bRecycleOverlapped)
		{
			delete psOverlapped;
		}
		return FALSE;
	}

	return TRUE;
}

BOOL ServerManager::SendPacket(SHeader* psPacket)
{
	assert(nullptr != psPacket);

	// ��ϵ��� ���� ��Ŷ�� ������ �� ����.
	// [Ŭ��] -> [����]
	if (EC2S_TYPE_MAX <= psPacket->usType)
	{
		return FALSE;
	}

	// �������� ����
	SOverlapped* psOverlapped = new SOverlapped;
	psOverlapped->eIOType = SOverlapped::EIOType::EIOType_Send;
	psOverlapped->socket = m_psSocket->socket;

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
		sprintf_s(szBuffer, "[TCP Ŭ���̾�Ʈ] ���� �߻� -- WSASend() :");
		ErrManager::ErrDisplay(szBuffer);

		delete psOverlapped;
		return FALSE;
	}

	return TRUE;
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
		sprintf_s(szBuffer, "[TCP Ŭ���̾�Ʈ] ���� �߻� -- WSAStartup() :");
		ErrManager::ErrDisplay(szBuffer);

		return -1;
	}

	// ���� ����ü ����
	m_psSocket = new SSocket;

	// 1. IOCP ����
	m_hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 0);
	assert(nullptr != m_hIOCP);
	
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// 2. WorkerThread() �� ����
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// WorkerThread() �� iThreadCount ��ŭ ����
	// IOCP �����带 iThreadCount ��ŭ ����
	m_IOCPThread = new IOCPThread(this, m_hIOCP);
	m_IOCPThread->Create(CREATE_SUSPENDED, 2);
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	printf_s("[TCP Ŭ���̾�Ʈ] ����\n");

	return true;
}

void ServerManager::Release()
{
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// WorkerThread() ���� ó�� �� ���
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// WorkerThread() ���� ���Ḧ ���� ���� ��ȣ
	PostQueuedCompletionStatus(m_hIOCP, 0, 0, nullptr);

	// ������ ����
	m_IOCPThread->Destroy();

	// IOCP ����
	CloseHandle(m_hIOCP);
	m_hIOCP = nullptr;
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// ���� ����
	WSACleanup();

	printf_s("[TCP Ŭ���̾�Ʈ] ����\n");
}

void ServerManager::Begin()
{
	Initialize();
}

void ServerManager::End()
{
	Release();
}

unsigned int ServerManager::Run(int dwIndex)
{
	while (!m_bExit)
	{
		// IP �ּҸ� �Է¹޴´�.
		printf_s("IP �ּҸ� �Է����ּ���.\n");
		printf_s("�ڱ� �ڽ��� IP�� �����Ϸ��� 0�� �Է����ּ���.\n");
		std::string strBuffer;
		std::getline(std::cin, strBuffer);

		// �Էµ� �޽����� ����
		if (strBuffer.empty())
		{
			printf_s("�Էµ� �޽����� �����ϴ�.\n");
			continue;
		}
		else if (strBuffer == "0")
		{
			strBuffer = IP;
		}

		// 3. WSASocket()
		if (INVALID_SOCKET == m_psSocket->socket)
		{
			m_psSocket->socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, WSA_FLAG_OVERLAPPED);
			assert(0 != m_psSocket->socket);

			if (INVALID_SOCKET == m_psSocket->socket)
			{
				char szBuffer[BUFSIZE] = { 0, };
				sprintf_s(szBuffer, "[TCP Ŭ���̾�Ʈ] ���� �߻� -- WSASocket() :");
				ErrManager::ErrDisplay(szBuffer);
				break;
			}
		}

		printf_s("[TCP Ŭ���̾�Ʈ] ���� �õ�\n");

		// 4. connect()
		while (!m_bExit)
		{
			SOCKADDR_IN serveraddr;
			ZeroMemory(&serveraddr, sizeof(serveraddr));
			serveraddr.sin_family = AF_INET;
			serveraddr.sin_port = htons(PORT);
			serveraddr.sin_addr.s_addr = inet_addr(strBuffer.c_str());
			if (SOCKET_ERROR == connect(m_psSocket->socket, reinterpret_cast<SOCKADDR*>(&serveraddr), sizeof(serveraddr)))
			{
				if (WSAGetLastError() == WSAECONNREFUSED)
				{
					printf_s("[TCP Ŭ���̾�Ʈ] ���� ���� ��õ�\n");
					continue;
				}

				closesocket(m_psSocket->socket);
				m_psSocket->socket = INVALID_SOCKET;
			}

			break;
		}

		// ����� �����ص� ���� üũ
		if ((m_bExit) || (INVALID_SOCKET == m_psSocket->socket))
		{
			continue;
		}

		m_psSocket->bConnected = TRUE;

		printf_s("[TCP Ŭ���̾�Ʈ] ������ ���� �Ϸ�\n");

		// 5. ������ IOCP �� Ű ���� �Բ� ���
		if (CreateIoCompletionPort(reinterpret_cast<HANDLE>(m_psSocket->socket), m_hIOCP, reinterpret_cast<ULONG_PTR>(m_psSocket), 0) != m_hIOCP)
		{
			shutdown(m_psSocket->socket, SD_BOTH);
			closesocket(m_psSocket->socket);
			m_psSocket->socket = INVALID_SOCKET;
			m_psSocket->bConnected = FALSE;

			printf_s("[TCP Ŭ���̾�Ʈ] ������ ���� ����\n");
			continue;
		}

		// 6. WSARecv() �ɱ�
		if (!Receive())
		{
			shutdown(m_psSocket->socket, SD_BOTH);
			closesocket(m_psSocket->socket);
			m_psSocket->socket = INVALID_SOCKET;
			m_psSocket->bConnected = FALSE;

			printf_s("[TCP Ŭ���̾�Ʈ] ������ ���� ����\n");
			continue;
		}
		
		// ���� ���
		while (!m_bExit)
		{
			// Ctrl - C ����
			if (std::cin.eof())
			{
				Sleep(1000);
				break;
			}

			// ������ ������� �ʾ�����, ���� �õ�
			if (!m_psSocket->bConnected)
			{
				m_psSocket->socket = INVALID_SOCKET;
				break;
			}
		}
	}

	return 0;
}
