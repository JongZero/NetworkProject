#include "pch.h"
#include "IOCPThread.h"
#include "ServerManager.h"

IOCPThread::IOCPThread(ServerManager* server, HANDLE hIOCP)
	: m_ServerManager(server), m_hIOCP(hIOCP)
{

}

IOCPThread::~IOCPThread()
{

}

void IOCPThread::Begin(void)
{
	size_t nCount = Resume();
	assert(m_ThreadVec.size() == nCount);
}

void IOCPThread::End(void)
{
	// 종료 시 필요한 코드 넣기
	// 지금은 아직 없다.
}

unsigned int IOCPThread::Run(int dwIndex)
{
	assert(m_ServerManager != nullptr);
	assert(m_hIOCP != nullptr);

	// 쓰레드 종료 체크
	while (!m_bClose)
	{
		DWORD        dwNumberOfBytesTransferred = 0;
		SSocket* psSocket = nullptr;
		SOverlapped* psOverlapped = nullptr;

		// GetQueuedCompletionStatus() - GQCS 라고 부름
		// WSARead(), WSAWrite() 등의 Overlapped I/O 관련 처리 결과를 받아오는 함수
		// PostQueuedCompletionStatus() 를 통해서도 GQCS 를 리턴시킬 수 있다.( 일반적으로 쓰레드 종료 처리 )
		BOOL bSuccessed = GetQueuedCompletionStatus(m_hIOCP,												// IOCP 핸들
			&dwNumberOfBytesTransferred,						    // I/O 에 사용된 데이터의 크기
			reinterpret_cast<PULONG_PTR>(&psSocket),           // 소켓의 IOCP 등록시 넘겨준 키 값
																   // ( connect() 이 후, CreateIoCompletionPort() 시 )
			reinterpret_cast<LPOVERLAPPED*>(&psOverlapped),   // WSARead(), WSAWrite() 등에 사용된 WSAOVERLAPPED
			INFINITE);										    // 신호가 발생될 때까지 무제한 대기

		// 키가 nullptr 일 경우 쓰레드 종료를 의미
		if (nullptr == psSocket)
		{
			// 다른 WorkerThread() 들의 종료를 위해서
			PostQueuedCompletionStatus(m_hIOCP, 0, 0, nullptr);
			break;
		}

		assert(nullptr != psOverlapped);

		// 오버랩드 결과 체크
		if (!bSuccessed)
		{
			if (SOCKET_ERROR != shutdown(psOverlapped->socket, SD_BOTH))
			{
				closesocket(psSocket->socket);
				psSocket->bConnected = FALSE;

				printf_s("[TCP 클라이언트] 서버와 연결이 끊겼습니다.\n");

				// 서버가 종료되었으니 게임을 종료시킨다.
				MainGameScene::GetInstance()->SetDelayTime(2.5f);
				MainGameScene::GetInstance()->SetIsServerDown(true);
			}

			delete psOverlapped;
			continue;
		}

		// 연결 종료
		if (0 == dwNumberOfBytesTransferred)
		{
			if (SOCKET_ERROR != shutdown(psOverlapped->socket, SD_BOTH))
			{
				closesocket(psSocket->socket);
				psSocket->bConnected = FALSE;

				printf_s("[TCP 클라이언트] 서버와 연결이 끊겼습니다.\n");

				// 서버가 종료되었으니 게임을 종료시킨다.
				MainGameScene::GetInstance()->SetDelayTime(2.5f);
				MainGameScene::GetInstance()->SetIsServerDown(true);
			}

			delete psOverlapped;
			continue;
		}

		// Overlapped I/O 처리
		switch (psOverlapped->eIOType)
		{
			case SOverlapped::EIOType::EIOType_Recv: ProcessRecv(dwNumberOfBytesTransferred, psOverlapped, psSocket); break; // WSARecv() 의 Overlapped I/O 완료에 대한 처리
			case SOverlapped::EIOType::EIOType_Send: ProcessSend(dwNumberOfBytesTransferred, psOverlapped, psSocket); break; // WSASend() 의 Overlapped I/O 완료에 대한 처리
		}
	}

	return 0;
}

void IOCPThread::ProcessRecv(DWORD dwNumberOfBytesTransferred, SOverlapped* psOverlapped, SSocket* psSocket)
{
	printf_s("[TCP 클라이언트] 패킷 수신 완료 <- %d 바이트\n", dwNumberOfBytesTransferred);

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// 패킷 처리
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// 수신한 데이터들 크기를 누적 시켜준다.
	psOverlapped->iDataSize += dwNumberOfBytesTransferred;

	// 처리할 데이터가 있으면 처리
	while (psOverlapped->iDataSize > 0)
	{
		// header 크기는 2 바이트( 고정 )
		static const unsigned short cusHeaderSize = 2;

		// header 를 다 받지 못했다. 이어서 recv()
		if (cusHeaderSize > psOverlapped->iDataSize)
		{
			break;
		}

		// body 의 크기는 N 바이트( 가변 ), 패킷에 담겨있음
		unsigned short usBodySize = *reinterpret_cast<unsigned short*>(psOverlapped->szBuffer);
		unsigned short usPacketSize = cusHeaderSize + usBodySize;

		// 하나의 패킷을 다 받지 못했다. 이어서 recv()
		if (usPacketSize > psOverlapped->iDataSize)
		{
			break;
		}

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// 완성된 패킷을 처리
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// 클라이언트로부터 수신한 패킷
		const SHeader* const cpcHeader = reinterpret_cast<const SHeader* const>(psOverlapped->szBuffer);

		switch (cpcHeader->usType)
		{
			case ES2C_TYPE_MESSAGE:         // 채팅 메세지
			{
				const S2C_Message* const pcscPacket = static_cast<const S2C_Message* const>(cpcHeader);

				// 초록색
				SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 2);

				printf_s("[TCP 클라이언트] [%04d-%02d-%02d %02d:%02d:%02d] [%15s:%5d] 님의 말 : %s\n", pcscPacket->sDate.wYear, pcscPacket->sDate.wMonth, pcscPacket->sDate.wDay,
					pcscPacket->sDate.wHour, pcscPacket->sDate.wMinute, pcscPacket->sDate.wSecond,
					pcscPacket->szIP, pcscPacket->usPort,
					pcscPacket->szMessage);

				// 흰색
				SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
			}
			break;
			case ES2C_TYPE_PLAYERNUM:		// 플레이어 번호
			{
				const S2C_PlayerNum* const pcscPacket = static_cast<const S2C_PlayerNum* const>(cpcHeader);
				MainGameScene::GetInstance()->SetPlayerNum(pcscPacket->usPlayerNum);

				printf_s("[TCP 클라이언트] 나의 플레이어 번호 : %d\n", pcscPacket->usPlayerNum);
			}
			break;
			case ES2C_TYPE_GAMESTATE:
			{
				const S2C_GameState* const pcscPacket = static_cast<const S2C_GameState* const>(cpcHeader);

				//MainGameScene::GetInstance()->ResetRelativePlayerFrame();

				if (pcscPacket->bIsGameStart)
				{
					MainGameScene::GetInstance()->SetIsGameStart(pcscPacket->bIsGameStart);
					MainGameScene::GetInstance()->SetDelayTime(1.8f);
					printf_s("[TCP 클라이언트] 상대플레이어가 접속했습니다. 게임을 시작합니다.\n");
				}
				else
				{
					MainGameScene::GetInstance()->SetDelayTime(2.5f);

					// 중간에 상대방이 나감
					if (pcscPacket->bIsGameWin)
					{
						if (MainGameScene::GetInstance()->GetIsGameWin() == false)
							MainGameScene::GetInstance()->SetIsGameWin(true);

						MainGameScene::GetInstance()->SetIsGameEnd(true);
					}
					// 정상 종료
					else
					{
						MainGameScene::GetInstance()->SetIsGameEnd(true);
						printf_s("[TCP 클라이언트] 턴이 모두 끝났습니다. 게임을 종료합니다.\n");
					}
				}
			}
			break;
			case ES2C_TYPE_TURN:
			{
				const S2C_Turn* const pcscPacket = static_cast<const S2C_Turn* const>(cpcHeader);
				MainGameScene::GetInstance()->SetNowTurn(pcscPacket->usTurn);
				MainGameScene::GetInstance()->SetNowAttackPlayer(pcscPacket->usAttack);
				MainGameScene::GetInstance()->SetDelayTime(1.8f);
				MainGameScene::GetInstance()->ResetRelativePlayerFrame();
				MainGameScene::GetInstance()->SetEndMyTurn(false);

				printf_s("[TCP 클라이언트] 현재 공격권 : 플레이어 %d\n", pcscPacket->usAttack);
				printf_s("[TCP 클라이언트] 지금은 플레이어 %d의 턴입니다.\n", pcscPacket->usTurn);
			}
			break;
			case ES2C_TYPE_KEYINPUT:
			{
				const S2C_KeyInput* const pcscPacket = static_cast<const S2C_KeyInput* const>(cpcHeader);

				MainGameScene::GetInstance()->RelativePlayerAction(pcscPacket->inputKey);

				printf_s("[TCP 클라이언트] 상대방이 키를 입력했습니다.\n");
			}
			break;
			case ES2C_TYPE_SCORE:
			{
				// 점수를 서버로부터 받아와 표시한다.
				const S2C_Score* const pcscPacket = static_cast<const S2C_Score* const>(cpcHeader);

				MainGameScene::GetInstance()->SetScore(pcscPacket->usScore[0], pcscPacket->usScore[1]);
			}
			break;
			case ES2C_TYPE_ROUND:
			{
				// 현재 라운드 표시
				const S2C_Round* const pcscPacket = static_cast<const S2C_Round* const>(cpcHeader);

				MainGameScene::GetInstance()->SetRemainingRound(pcscPacket->usRound);
			}
			break;
			case ES2C_TYPE_SERVER_DOWN:
			{
				const S2C_ServerDown* const pcscPacket = static_cast<const S2C_ServerDown* const>(cpcHeader);

				// 서버가 종료되었으니 게임을 종료시킨다.
				MainGameScene::GetInstance()->SetDelayTime(2.5f);
				//MainGameScene::GetInstance()->SetIsServerDown(true);
			}
			break;
			case ES2C_TYPE_MAX:
				break;
		}
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		// 데이터들을 이번에 처리한만큼 당긴다.
		memcpy_s(psOverlapped->szBuffer, psOverlapped->iDataSize,
			psOverlapped->szBuffer + usPacketSize, psOverlapped->iDataSize - usPacketSize);

		// 처리한 패킷 크기만큼 처리할량 감소
		psOverlapped->iDataSize -= usPacketSize;
	}
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// 수신 걸기( 이번에 사용한 오버랩드를 다시 사용 )
	if (!m_ServerManager->Receive(psOverlapped))
	{
		if (SOCKET_ERROR != shutdown(psOverlapped->socket, SD_BOTH))
		{
			closesocket(psSocket->socket);
			psSocket->bConnected = FALSE;

			printf_s("[TCP 클라이언트] 서버와 연결이 끊겼습니다.\n");

			// 서버가 종료되었으니 게임을 종료시킨다.
			MainGameScene::GetInstance()->SetDelayTime(2.5f);
			MainGameScene::GetInstance()->SetIsServerDown(true);
		}

		delete psOverlapped;
		return;
	}
}

void IOCPThread::ProcessSend(DWORD dwNumberOfBytesTransferred, SOverlapped* psOverlapped, SSocket* psSocket)
{
	printf_s("[TCP 클라이언트] 패킷 송신 완료 -> %d 바이트\n", dwNumberOfBytesTransferred);

	delete psOverlapped;
}