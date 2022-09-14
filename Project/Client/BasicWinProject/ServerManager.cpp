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
		// 종료 플래그 켬
		m_bExit = TRUE;
	}
}

BOOL ServerManager::Receive(SOverlapped* psOverlapped)
{
	BOOL bRecycleOverlapped = TRUE;

	// 사용할 오버랩드를 받지 않았으면 생성
	if (nullptr == psOverlapped)
	{
		psOverlapped = new SOverlapped;
		bRecycleOverlapped = FALSE;
	}

	// 오버랩드 셋팅
	psOverlapped->eIOType = SOverlapped::EIOType::EIOType_Recv;
	psOverlapped->socket = m_psSocket->socket;

	// WSABUF 셋팅
	WSABUF wsaBuffer;
	wsaBuffer.buf = psOverlapped->szBuffer + psOverlapped->iDataSize;
	wsaBuffer.len = sizeof(psOverlapped->szBuffer) - psOverlapped->iDataSize;

	// WSARecv() 오버랩드 걸기
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
		sprintf_s(szBuffer, "[TCP 클라이언트] 에러 발생 -- WSARecv() :");
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

	// 등록되지 않은 패킷은 전송할 수 없다.
	// [클라] -> [서버]
	if (EC2S_TYPE_MAX <= psPacket->usType)
	{
		return FALSE;
	}

	// 오버랩드 셋팅
	SOverlapped* psOverlapped = new SOverlapped;
	psOverlapped->eIOType = SOverlapped::EIOType::EIOType_Send;
	psOverlapped->socket = m_psSocket->socket;

	// 패킷 복사
	psOverlapped->iDataSize = 2 + psPacket->usSize;

	if (sizeof(psOverlapped->szBuffer) < psOverlapped->iDataSize)
	{
		delete psOverlapped;
		return FALSE;
	}

	memcpy_s(psOverlapped->szBuffer, sizeof(psOverlapped->szBuffer), psPacket, psOverlapped->iDataSize);

	// WSABUF 셋팅
	WSABUF wsaBuffer;
	wsaBuffer.buf = psOverlapped->szBuffer;
	wsaBuffer.len = psOverlapped->iDataSize;

	// WSASend() 오버랩드 걸기
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
		sprintf_s(szBuffer, "[TCP 클라이언트] 에러 발생 -- WSASend() :");
		ErrManager::ErrDisplay(szBuffer);

		delete psOverlapped;
		return FALSE;
	}

	return TRUE;
}

bool ServerManager::Initialize()
{
	// Ctrl - C 를 누르면,
   // 주 쓰레드를 포함한 모든 쓰레드들이 정상 종료를 할 수 있도록 함
	signal(SIGINT, ServerManager::SignalFunction);

	char szBuffer[BUFSIZE] = { 0, };

	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		sprintf_s(szBuffer, "[TCP 클라이언트] 에러 발생 -- WSAStartup() :");
		ErrManager::ErrDisplay(szBuffer);

		return -1;
	}

	// 소켓 구조체 생성
	m_psSocket = new SSocket;

	// 1. IOCP 생성
	m_hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 0);
	assert(nullptr != m_hIOCP);
	
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// 2. WorkerThread() 들 동작
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// WorkerThread() 를 iThreadCount 만큼 생성
	// IOCP 쓰레드를 iThreadCount 만큼 생성
	m_IOCPThread = new IOCPThread(this, m_hIOCP);
	m_IOCPThread->Create(CREATE_SUSPENDED, 2);
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	printf_s("[TCP 클라이언트] 시작\n");

	return true;
}

void ServerManager::Release()
{
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// WorkerThread() 종료 처리 및 대기
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// WorkerThread() 들의 종료를 위한 최초 신호
	PostQueuedCompletionStatus(m_hIOCP, 0, 0, nullptr);

	// 쓰레드 종료
	m_IOCPThread->Destroy();

	// IOCP 종료
	CloseHandle(m_hIOCP);
	m_hIOCP = nullptr;
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// 윈속 종료
	WSACleanup();

	printf_s("[TCP 클라이언트] 종료\n");
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
		// IP 주소를 입력받는다.
		printf_s("IP 주소를 입력해주세요.\n");
		printf_s("자기 자신의 IP에 접속하려면 0을 입력해주세요.\n");
		std::string strBuffer;
		std::getline(std::cin, strBuffer);

		// 입력된 메시지가 없음
		if (strBuffer.empty())
		{
			printf_s("입력된 메시지가 없습니다.\n");
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
				sprintf_s(szBuffer, "[TCP 클라이언트] 에러 발생 -- WSASocket() :");
				ErrManager::ErrDisplay(szBuffer);
				break;
			}
		}

		printf_s("[TCP 클라이언트] 연결 시도\n");

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
					printf_s("[TCP 클라이언트] 서버 연결 재시도\n");
					continue;
				}

				closesocket(m_psSocket->socket);
				m_psSocket->socket = INVALID_SOCKET;
			}

			break;
		}

		// 통신을 진행해도 될지 체크
		if ((m_bExit) || (INVALID_SOCKET == m_psSocket->socket))
		{
			continue;
		}

		m_psSocket->bConnected = TRUE;

		printf_s("[TCP 클라이언트] 서버와 연결 완료\n");

		// 5. 소켓을 IOCP 에 키 값과 함께 등록
		if (CreateIoCompletionPort(reinterpret_cast<HANDLE>(m_psSocket->socket), m_hIOCP, reinterpret_cast<ULONG_PTR>(m_psSocket), 0) != m_hIOCP)
		{
			shutdown(m_psSocket->socket, SD_BOTH);
			closesocket(m_psSocket->socket);
			m_psSocket->socket = INVALID_SOCKET;
			m_psSocket->bConnected = FALSE;

			printf_s("[TCP 클라이언트] 서버와 연결 종료\n");
			continue;
		}

		// 6. WSARecv() 걸기
		if (!Receive())
		{
			shutdown(m_psSocket->socket, SD_BOTH);
			closesocket(m_psSocket->socket);
			m_psSocket->socket = INVALID_SOCKET;
			m_psSocket->bConnected = FALSE;

			printf_s("[TCP 클라이언트] 서버와 연결 종료\n");
			continue;
		}
		
		// 종료 대기
		while (!m_bExit)
		{
			// Ctrl - C 종료
			if (std::cin.eof())
			{
				Sleep(1000);
				break;
			}

			// 소켓이 연결되지 않았으면, 연결 시도
			if (!m_psSocket->bConnected)
			{
				m_psSocket->socket = INVALID_SOCKET;
				break;
			}
		}
	}

	return 0;
}
