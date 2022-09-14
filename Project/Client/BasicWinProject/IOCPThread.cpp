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
	// ���� �� �ʿ��� �ڵ� �ֱ�
	// ������ ���� ����.
}

unsigned int IOCPThread::Run(int dwIndex)
{
	assert(m_ServerManager != nullptr);
	assert(m_hIOCP != nullptr);

	// ������ ���� üũ
	while (!m_bClose)
	{
		DWORD        dwNumberOfBytesTransferred = 0;
		SSocket* psSocket = nullptr;
		SOverlapped* psOverlapped = nullptr;

		// GetQueuedCompletionStatus() - GQCS ��� �θ�
		// WSARead(), WSAWrite() ���� Overlapped I/O ���� ó�� ����� �޾ƿ��� �Լ�
		// PostQueuedCompletionStatus() �� ���ؼ��� GQCS �� ���Ͻ�ų �� �ִ�.( �Ϲ������� ������ ���� ó�� )
		BOOL bSuccessed = GetQueuedCompletionStatus(m_hIOCP,												// IOCP �ڵ�
			&dwNumberOfBytesTransferred,						    // I/O �� ���� �������� ũ��
			reinterpret_cast<PULONG_PTR>(&psSocket),           // ������ IOCP ��Ͻ� �Ѱ��� Ű ��
																   // ( connect() �� ��, CreateIoCompletionPort() �� )
			reinterpret_cast<LPOVERLAPPED*>(&psOverlapped),   // WSARead(), WSAWrite() � ���� WSAOVERLAPPED
			INFINITE);										    // ��ȣ�� �߻��� ������ ������ ���

		// Ű�� nullptr �� ��� ������ ���Ḧ �ǹ�
		if (nullptr == psSocket)
		{
			// �ٸ� WorkerThread() ���� ���Ḧ ���ؼ�
			PostQueuedCompletionStatus(m_hIOCP, 0, 0, nullptr);
			break;
		}

		assert(nullptr != psOverlapped);

		// �������� ��� üũ
		if (!bSuccessed)
		{
			if (SOCKET_ERROR != shutdown(psOverlapped->socket, SD_BOTH))
			{
				closesocket(psSocket->socket);
				psSocket->bConnected = FALSE;

				printf_s("[TCP Ŭ���̾�Ʈ] ������ ������ ������ϴ�.\n");

				// ������ ����Ǿ����� ������ �����Ų��.
				MainGameScene::GetInstance()->SetDelayTime(2.5f);
				MainGameScene::GetInstance()->SetIsServerDown(true);
			}

			delete psOverlapped;
			continue;
		}

		// ���� ����
		if (0 == dwNumberOfBytesTransferred)
		{
			if (SOCKET_ERROR != shutdown(psOverlapped->socket, SD_BOTH))
			{
				closesocket(psSocket->socket);
				psSocket->bConnected = FALSE;

				printf_s("[TCP Ŭ���̾�Ʈ] ������ ������ ������ϴ�.\n");

				// ������ ����Ǿ����� ������ �����Ų��.
				MainGameScene::GetInstance()->SetDelayTime(2.5f);
				MainGameScene::GetInstance()->SetIsServerDown(true);
			}

			delete psOverlapped;
			continue;
		}

		// Overlapped I/O ó��
		switch (psOverlapped->eIOType)
		{
			case SOverlapped::EIOType::EIOType_Recv: ProcessRecv(dwNumberOfBytesTransferred, psOverlapped, psSocket); break; // WSARecv() �� Overlapped I/O �Ϸῡ ���� ó��
			case SOverlapped::EIOType::EIOType_Send: ProcessSend(dwNumberOfBytesTransferred, psOverlapped, psSocket); break; // WSASend() �� Overlapped I/O �Ϸῡ ���� ó��
		}
	}

	return 0;
}

void IOCPThread::ProcessRecv(DWORD dwNumberOfBytesTransferred, SOverlapped* psOverlapped, SSocket* psSocket)
{
	printf_s("[TCP Ŭ���̾�Ʈ] ��Ŷ ���� �Ϸ� <- %d ����Ʈ\n", dwNumberOfBytesTransferred);

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// ��Ŷ ó��
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// ������ �����͵� ũ�⸦ ���� �����ش�.
	psOverlapped->iDataSize += dwNumberOfBytesTransferred;

	// ó���� �����Ͱ� ������ ó��
	while (psOverlapped->iDataSize > 0)
	{
		// header ũ��� 2 ����Ʈ( ���� )
		static const unsigned short cusHeaderSize = 2;

		// header �� �� ���� ���ߴ�. �̾ recv()
		if (cusHeaderSize > psOverlapped->iDataSize)
		{
			break;
		}

		// body �� ũ��� N ����Ʈ( ���� ), ��Ŷ�� �������
		unsigned short usBodySize = *reinterpret_cast<unsigned short*>(psOverlapped->szBuffer);
		unsigned short usPacketSize = cusHeaderSize + usBodySize;

		// �ϳ��� ��Ŷ�� �� ���� ���ߴ�. �̾ recv()
		if (usPacketSize > psOverlapped->iDataSize)
		{
			break;
		}

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// �ϼ��� ��Ŷ�� ó��
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Ŭ���̾�Ʈ�κ��� ������ ��Ŷ
		const SHeader* const cpcHeader = reinterpret_cast<const SHeader* const>(psOverlapped->szBuffer);

		switch (cpcHeader->usType)
		{
			case ES2C_TYPE_MESSAGE:         // ä�� �޼���
			{
				const S2C_Message* const pcscPacket = static_cast<const S2C_Message* const>(cpcHeader);

				// �ʷϻ�
				SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 2);

				printf_s("[TCP Ŭ���̾�Ʈ] [%04d-%02d-%02d %02d:%02d:%02d] [%15s:%5d] ���� �� : %s\n", pcscPacket->sDate.wYear, pcscPacket->sDate.wMonth, pcscPacket->sDate.wDay,
					pcscPacket->sDate.wHour, pcscPacket->sDate.wMinute, pcscPacket->sDate.wSecond,
					pcscPacket->szIP, pcscPacket->usPort,
					pcscPacket->szMessage);

				// ���
				SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
			}
			break;
			case ES2C_TYPE_PLAYERNUM:		// �÷��̾� ��ȣ
			{
				const S2C_PlayerNum* const pcscPacket = static_cast<const S2C_PlayerNum* const>(cpcHeader);
				MainGameScene::GetInstance()->SetPlayerNum(pcscPacket->usPlayerNum);

				printf_s("[TCP Ŭ���̾�Ʈ] ���� �÷��̾� ��ȣ : %d\n", pcscPacket->usPlayerNum);
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
					printf_s("[TCP Ŭ���̾�Ʈ] ����÷��̾ �����߽��ϴ�. ������ �����մϴ�.\n");
				}
				else
				{
					MainGameScene::GetInstance()->SetDelayTime(2.5f);

					// �߰��� ������ ����
					if (pcscPacket->bIsGameWin)
					{
						if (MainGameScene::GetInstance()->GetIsGameWin() == false)
							MainGameScene::GetInstance()->SetIsGameWin(true);

						MainGameScene::GetInstance()->SetIsGameEnd(true);
					}
					// ���� ����
					else
					{
						MainGameScene::GetInstance()->SetIsGameEnd(true);
						printf_s("[TCP Ŭ���̾�Ʈ] ���� ��� �������ϴ�. ������ �����մϴ�.\n");
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

				printf_s("[TCP Ŭ���̾�Ʈ] ���� ���ݱ� : �÷��̾� %d\n", pcscPacket->usAttack);
				printf_s("[TCP Ŭ���̾�Ʈ] ������ �÷��̾� %d�� ���Դϴ�.\n", pcscPacket->usTurn);
			}
			break;
			case ES2C_TYPE_KEYINPUT:
			{
				const S2C_KeyInput* const pcscPacket = static_cast<const S2C_KeyInput* const>(cpcHeader);

				MainGameScene::GetInstance()->RelativePlayerAction(pcscPacket->inputKey);

				printf_s("[TCP Ŭ���̾�Ʈ] ������ Ű�� �Է��߽��ϴ�.\n");
			}
			break;
			case ES2C_TYPE_SCORE:
			{
				// ������ �����κ��� �޾ƿ� ǥ���Ѵ�.
				const S2C_Score* const pcscPacket = static_cast<const S2C_Score* const>(cpcHeader);

				MainGameScene::GetInstance()->SetScore(pcscPacket->usScore[0], pcscPacket->usScore[1]);
			}
			break;
			case ES2C_TYPE_ROUND:
			{
				// ���� ���� ǥ��
				const S2C_Round* const pcscPacket = static_cast<const S2C_Round* const>(cpcHeader);

				MainGameScene::GetInstance()->SetRemainingRound(pcscPacket->usRound);
			}
			break;
			case ES2C_TYPE_SERVER_DOWN:
			{
				const S2C_ServerDown* const pcscPacket = static_cast<const S2C_ServerDown* const>(cpcHeader);

				// ������ ����Ǿ����� ������ �����Ų��.
				MainGameScene::GetInstance()->SetDelayTime(2.5f);
				//MainGameScene::GetInstance()->SetIsServerDown(true);
			}
			break;
			case ES2C_TYPE_MAX:
				break;
		}
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		// �����͵��� �̹��� ó���Ѹ�ŭ ����.
		memcpy_s(psOverlapped->szBuffer, psOverlapped->iDataSize,
			psOverlapped->szBuffer + usPacketSize, psOverlapped->iDataSize - usPacketSize);

		// ó���� ��Ŷ ũ�⸸ŭ ó���ҷ� ����
		psOverlapped->iDataSize -= usPacketSize;
	}
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// ���� �ɱ�( �̹��� ����� �������带 �ٽ� ��� )
	if (!m_ServerManager->Receive(psOverlapped))
	{
		if (SOCKET_ERROR != shutdown(psOverlapped->socket, SD_BOTH))
		{
			closesocket(psSocket->socket);
			psSocket->bConnected = FALSE;

			printf_s("[TCP Ŭ���̾�Ʈ] ������ ������ ������ϴ�.\n");

			// ������ ����Ǿ����� ������ �����Ų��.
			MainGameScene::GetInstance()->SetDelayTime(2.5f);
			MainGameScene::GetInstance()->SetIsServerDown(true);
		}

		delete psOverlapped;
		return;
	}
}

void IOCPThread::ProcessSend(DWORD dwNumberOfBytesTransferred, SOverlapped* psOverlapped, SSocket* psSocket)
{
	printf_s("[TCP Ŭ���̾�Ʈ] ��Ŷ �۽� �Ϸ� -> %d ����Ʈ\n", dwNumberOfBytesTransferred);

	delete psOverlapped;
}