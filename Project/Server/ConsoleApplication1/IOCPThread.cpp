#include "IOCPThread.h"
#include "ServerManager.h"
#include "GameManager.h"

IOCPThread::IOCPThread(ServerManager* server, GameManager* game, HANDLE hIOCP)
	: m_ServerManager(server), m_GameManager(game), m_hIOCP(hIOCP)
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
		SSocket*	 psSocket = nullptr;
		SOverlapped* psOverlapped = nullptr;

		// GetQueuedCompletionStatus() - GQCS ��� �θ�
		// WSARead(), WSAWrite() ���� Overlapped I/O ���� ó�� ����� �޾ƿ��� �Լ�
		// PostQueuedCompletionStatus() �� ���ؼ��� GQCS �� ���Ͻ�ų �� �ִ�.( �Ϲ������� ������ ���� ó�� )
		BOOL bSuccessed = GetQueuedCompletionStatus(m_hIOCP,												// IOCP �ڵ�
			&dwNumberOfBytesTransferred,						    // I/O �� ���� �������� ũ��
			reinterpret_cast<PULONG_PTR>(&psSocket),           // ������ IOCP ��Ͻ� �Ѱ��� Ű ��
																   // ( WSAAccept() �� ��, CreateIoCompletionPort() �� )
			reinterpret_cast<LPOVERLAPPED*>(&psOverlapped),   // WSARead(), WSAWrite() � ���� WSAOVERLAPPED
			INFINITE);										    // ��ȣ�� �߻��� ������ ������ ���

		// Ű�� nullptr �� ��� ������ ���Ḧ �ǹ�
		if (nullptr == psSocket)
		{
			// �ٸ� WorkerThread() ���� ���Ḧ ���ؼ�
			PostQueuedCompletionStatus(m_hIOCP, 0, 0, nullptr);
			m_bClose = true;
			break;
		}

		assert(nullptr != psOverlapped);

		// �������� ��� üũ
		if (!bSuccessed)
		{
			// Ŭ���̾�Ʈ ����
			m_ServerManager->RemoveSocket(psOverlapped->socket);
			delete psOverlapped;
			continue;
		}

		// ���� ����
		if (0 == dwNumberOfBytesTransferred)
		{
			// Ŭ���̾�Ʈ ����
			m_ServerManager->RemoveSocket(psOverlapped->socket);
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
	printf_s("[TCP ����] [%15s:%5d] ��Ŷ ���� �Ϸ� <- %d ����Ʈ\n", psSocket->strIP.c_str(), psSocket->usPort, dwNumberOfBytesTransferred);

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// ��Ŷ ó��
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
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

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// �ϼ��� ��Ŷ�� ó��
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Ŭ���̾�Ʈ�κ��� ������ ��Ŷ
		const SHeader* const cpcHeader = reinterpret_cast<const SHeader* const>(psOverlapped->szBuffer);

		// �߸��� ��Ŷ
		if (EC2S_TYPE_MAX <= cpcHeader->usType)
		{
			// Ŭ���̾�Ʈ ����
			m_ServerManager->RemoveSocket(psOverlapped->socket);
			delete psOverlapped;
			return;
		}

		switch (cpcHeader->usType)
		{
		case EC2S_TYPE_MESSAGE:         // ä�� �޼���
		{
			const C2S_Message* const cpcPacket = static_cast<const C2S_Message* const>(cpcHeader);

			// S2C_Message ��Ŷ �ۼ�
			S2C_Message sPacket;
			GetLocalTime(&sPacket.sDate);                         // ��  ��
			strcpy_s(sPacket.szIP, psSocket->strIP.c_str());      // ������
			sPacket.usPort = psSocket->usPort;                    // ��  Ʈ
			strcpy_s(sPacket.szMessage, cpcPacket->szMessage);    // �޽���

			// ��ε� ĳ����
			m_ServerManager->BroadcastPacket(&sPacket);
		}
		break;
		
		case EC2S_TYPE_KEYINPUT:
		{
			const C2S_KeyInput* const cpcPacket = static_cast<const C2S_KeyInput* const>(cpcHeader);
		
			S2C_KeyInput packet;
			packet.inputKey = cpcPacket->inputKey;

			// ��� �÷��̾�� Input Key�� ������ (�Ҹ�, ��� ������ ����)
			m_ServerManager->SendPacketToRelativePlayer(psSocket, &packet);
		}
		break;

		case EC2S_TYPE_TURNEND:
		{
			const C2S_TurnEnd* const cpcPacket = static_cast<const C2S_TurnEnd* const>(cpcHeader);

			// �����̿����� ���񿴴���
			if (cpcPacket->bIsAttack)
			{
				m_GameManager->AttackKeyReset();

				for (int i = 0; i < 6; i++)
				{
					m_GameManager->AddAttackKey(cpcPacket->inputKeys[i]);
				}

				printf_s("[TCP ����] Attack Keys : %s\n", cpcPacket->inputKeys);
			}
			else
			{
				m_GameManager->DefenceKeyReset();

				for (int i = 0; i < 6; i++)
				{
					m_GameManager->AddDefenceKey(cpcPacket->inputKeys[i]);
				}

				printf_s("[TCP ����] Defence Keys : %s\n", cpcPacket->inputKeys);
			}

			m_GameManager->SetIsTurnEnd(cpcPacket->bIsTurnEnd);

		}
		break;
		}
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		// �����͵��� �̹��� ó���Ѹ�ŭ ����.
		memcpy_s(psOverlapped->szBuffer, psOverlapped->iDataSize,
			psOverlapped->szBuffer + usPacketSize, psOverlapped->iDataSize - usPacketSize);

		// ó���� ��Ŷ ũ�⸸ŭ ó���ҷ� ����
		psOverlapped->iDataSize -= usPacketSize;
	}
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	{   // �Ӱ� ����
		Lock _lock(&psSocket->CS_useCount);

		// ó���Ǿ��ٸ� ī��Ʈ ����
		psSocket->useCount--;
	}

	// �����ؾ��ϴ� �������� Ȯ��
	if (psSocket->useCount == 0 && psSocket->checkClose)
	{
		delete psSocket;
		delete psOverlapped;
		return;
	}

	// ���� �ɱ�( �̹��� ����� �������带 �ٽ� ��� )
	if (!m_ServerManager->Receive(psSocket, psOverlapped))
	{
		// Ŭ���̾�Ʈ ����
		m_ServerManager->RemoveSocket(psOverlapped->socket);
		delete psOverlapped;
		return;
	}
}

void IOCPThread::ProcessSend(DWORD dwNumberOfBytesTransferred, SOverlapped* psOverlapped, SSocket* psSocket)
{
	printf_s("[TCP ����] [%15s:%5d] ��Ŷ �۽� �Ϸ� -> %d ����Ʈ\n", psSocket->strIP.c_str(), psSocket->usPort, dwNumberOfBytesTransferred);

	{   // �Ӱ� ����
		Lock _lock(&psSocket->CS_useCount);

		// ó���Ǿ��ٸ� ī��Ʈ ����
		psSocket->useCount--;
	}

	// �����ؾ��ϴ� �������� Ȯ��
	if (psSocket->useCount == 0 && psSocket->checkClose)
	{
		delete psSocket;
	}

	delete psOverlapped;
}
