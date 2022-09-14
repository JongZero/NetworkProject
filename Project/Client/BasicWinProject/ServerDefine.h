#pragma once

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
const unsigned short    PORT = 9000;			// connect �� ����� ��Ʈ
const std::string       IP = "127.0.0.1";  // ������ ������

struct SSocket
{
	SSocket(void)
	{
		socket = INVALID_SOCKET;
		bConnected = FALSE;
	}

	~SSocket(void)
	{
		// ���� ����
		if (INVALID_SOCKET != socket)
		{
			shutdown(socket, SD_BOTH);
			closesocket(socket);
		}
	}

	SOCKET  socket;     // ����
    BOOL    bConnected; // ���� ����
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