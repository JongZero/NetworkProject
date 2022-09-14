#pragma once

// �ֽ� VC++ �����Ϸ����� ��� �� ���� ����
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <WinSock2.h>
#include <string>
#include <vector>
#include <list>
#include <assert.h>
#include <process.h>
#include "Lock.h"
#include "../../Packet/Packet.h"

// ����2 ���̺귯��
#pragma comment( lib, "ws2_32" )

const unsigned short    BUFSIZE = 512;			// ��/���� ���� ũ��
const unsigned short    PORT = 9000;			// listen �� ����� ��Ʈ
const unsigned short    MAX_USER_COUNT = 2;		// ���������� ������ �ִ� ���ϼ�( 0 ���� �����ϸ� ���� ������ ���� �ʰ� �� )

struct SSocket
{
	SSocket(void)
	{
		socket = INVALID_SOCKET;
		usPort = 0;
		useCount = 0;
		checkClose = false;

		// ũ��Ƽ�� ���� �ʱ�ȭ
		InitializeCriticalSection(&CS_useCount);
	}

	~SSocket(void)
	{
		// ���� ����
		if (INVALID_SOCKET != socket)
		{
			closesocket(socket);
		}

		// ũ��Ƽ�� ���� ����
		DeleteCriticalSection(&CS_useCount);
	}

	SOCKET              socket;         // ����� ��  ��
	std::string         strIP;          // ������ ������
	unsigned short      usPort;         // ������ ��  Ʈ
	int                 useCount;       // ���� ��� ī��Ʈ ( Send, Recv ���� ȣ�� �� ����. �Ϸ� ó���� ���� )
	CRITICAL_SECTION    CS_useCount;    // ī��Ʈ�� ������ �� ����� ũ��Ƽ�� ����
	bool                checkClose;     // ������ �ݾƾ��ϴ��� ���� ( �Ϸ� ó�� �������� �˻��Ѵ�. )
};

struct SOverlapped
{
	SOverlapped(void)
	{
		ZeroMemory(&wsaOverlapped, sizeof(wsaOverlapped));

		socket = INVALID_SOCKET;
		ZeroMemory(szBuffer, sizeof(szBuffer));
		iDataSize = 0;
	}

	// Overlapped I/O �۾��� ����
	enum class EIOType
	{
		EIOType_Recv,
		EIOType_Send
	};

	WSAOVERLAPPED   wsaOverlapped;      // Overlapped I/O �� ���� ����ü
	EIOType         eIOType;            // ���߿� ó�� ����� �뺸 �޾�����, WSARecv() �� ���� ó��������, WSASend() �� ���� ó�������� �����ϱ� ���� �뵵

	SOCKET          socket;             // �� ���������� ��� ����
	char            szBuffer[BUFSIZE];  // ����( EIOType_Recv �� ���� ����, EIOType_Send �� �۽� ���� )
	int             iDataSize;          // �����ͷ�( EIOType_Recv �� ������ ó���ؾ��� �����ͷ�, EIOType_Send �� �۽� �����ͷ� )
};