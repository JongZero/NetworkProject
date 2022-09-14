#pragma once

#include <WinSock2.h>
#include <string>
#include <vector>
#include <list>
#include <assert.h>
#include <process.h>
#include "Lock.h"
#include "../../Packet/Packet.h"

// 윈속2 라이브러리
#pragma comment( lib, "ws2_32" )

const unsigned short    BUFSIZE = 512;			// 송/수신 버퍼 크기
const unsigned short    PORT = 9000;			// connect 에 사용할 포트
const std::string       IP = "127.0.0.1";  // 서버측 아이피

struct SSocket
{
	SSocket(void)
	{
		socket = INVALID_SOCKET;
		bConnected = FALSE;
	}

	~SSocket(void)
	{
		// 소켓 정리
		if (INVALID_SOCKET != socket)
		{
			shutdown(socket, SD_BOTH);
			closesocket(socket);
		}
	}

	SOCKET  socket;     // 소켓
    BOOL    bConnected; // 연결 여부
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

	// Overlapped I/O 작업의 종류
	enum class EIOType
	{
		EIOType_Recv,
		EIOType_Send
	};

	WSAOVERLAPPED   wsaOverlapped;      // Overlapped I/O 에 사용될 구조체
	EIOType         eIOType;            // 나중에 처리 결과를 통보 받았을때, WSARecv() 에 대한 처리였는지, WSASend() 에 대한 처리였는지 구분하기 위한 용도

	SOCKET          socket;             // 이 오버랩드의 대상 소켓
	char            szBuffer[BUFSIZE];  // 버퍼( EIOType_Recv 시 수신 버퍼, EIOType_Send 시 송신 버퍼 )
	int             iDataSize;          // 데이터량( EIOType_Recv 시 누적된 처리해야할 데이터량, EIOType_Send 시 송신 데이터량 )
};