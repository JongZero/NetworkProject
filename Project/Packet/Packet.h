#pragma once

// 사용할 패킷 구조체들 1 바이트 정렬 시작
#pragma pack( push, 1 )

struct SHeader
{
    unsigned short usSize;          // 패킷의 사이즈
    unsigned short usType;          // 패킷의 타입
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// [클라이언트 -> 서버] 패킷들 타입 정의
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
enum EC2S_TYPE
{
    EC2S_TYPE_MESSAGE = 0,          // 채팅 메세지 전송 요청
    EC2S_TYPE_PLAYERNUM,            // 플레이어의 번호
    EC2S_TYPE_GAMESTATE,            // 게임 상태, 게임 시작 및 종료에 쓰임
    EC2S_TYPE_TURN,                 // 현재 누구 턴인지
    
	EC2S_TYPE_TURNEND,				// 턴 종료 여부
    EC2S_TYPE_KEYINPUT,             // 클라이언트에서 입력한 키 (상대방에게 보내주어 똑같은 음을 재생하도록 한다.)
	EC2S_TYPE_SCORE,				// 현재 점수

    EC2S_TYPE_MAX,
};

struct C2S_Message : public SHeader
{
    C2S_Message( void )
    {
        usSize = sizeof( *this ) - sizeof( usSize );
        usType = EC2S_TYPE_MESSAGE;
    }

    char            szMessage[128]; // 메시지 내용
};

struct C2S_PlayerNum : public SHeader
{
    C2S_PlayerNum(void)
	{
		usSize = sizeof(*this) - sizeof(usSize);
		usType = EC2S_TYPE_PLAYERNUM;
        usPlayerNum = -1;
	}

	unsigned short  usPlayerNum;     // 플레이어 번호
};

struct C2S_Turn : public SHeader
{
    C2S_Turn(void)
	{
		usSize = sizeof(*this) - sizeof(usSize);
		usType = EC2S_TYPE_TURN;
        usTurn = -1;
	}

    unsigned short  usTurn;     // 현재 누구 턴인지
	unsigned short  usAttack;	// 현재 누가 공격인지
};

struct C2S_GameState : public SHeader
{
    C2S_GameState(void)
	{
		usSize = sizeof(*this) - sizeof(usSize);
		usType = EC2S_TYPE_GAMESTATE;
        bIsGameStart = false;
		bIsGameWin = false;
	}

	bool  bIsGameStart;     // 클라이언트 측의 게임 시작을 위해 (상대 플레이어가 접속했다고 알려준다.), 게임 종료에도 쓰임
	bool  bIsGameWin;		// 상대방이 나갔을 경우 이 패킷을 받은 클라이언트는 무조건 이긴다.
};

struct C2S_TurnEnd : public SHeader
{
	C2S_TurnEnd(void)
	{
		usSize = sizeof(*this) - sizeof(usSize);
		usType = EC2S_TYPE_TURNEND;
		bIsTurnEnd = false;
		bIsAttack = true;
	}

	bool  bIsTurnEnd;     // 턴이 끝났다고 알려줌
	bool  bIsAttack;	  // 공격(문제 내기)이 끝난건지 수비(문제 맞추기)가 끝난건지
	char  inputKeys[6];	  // 입력받은 키 6개
};

struct C2S_KeyInput : public SHeader
{
	C2S_KeyInput(void)
	{
		usSize = sizeof(*this) - sizeof(usSize);
		usType = EC2S_TYPE_KEYINPUT;
		inputKey = NULL;
	}

	char inputKey;     // 클라이언트에서 누른 키
};

struct C2S_Score : public SHeader
{
	C2S_Score(void)
	{
		usSize = sizeof(*this) - sizeof(usSize);
		usType = EC2S_TYPE_SCORE;
		usScore[0] = 0;
		usScore[1] = 0;
	}

	unsigned short  usScore[2];     // 현재 점수
};
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// [서버 -> 클라이언트] 패킷들 타입 정의
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
enum ES2C_TYPE
{
    ES2C_TYPE_MESSAGE = 0,          // 유저의 채팅 메세지 브로드 캐스팅
    ES2C_TYPE_PLAYERNUM,            // 플레이어의 번호
    ES2C_TYPE_GAMESTATE,            // 게임 상태, 게임 시작 및 종료에 쓰임
    ES2C_TYPE_TURN,                 // 현재 누구 턴인지

	ES2C_TYPE_TURNEND,				// 턴 종료 여부
	ES2C_TYPE_KEYINPUT,				// 클라이언트에서 입력한 키 (상대방에게 보내주어 똑같은 음을 재생하도록 한다.)
	ES2C_TYPE_SCORE,				// 현재 점수

	ES2C_TYPE_ROUND,				// 남은 라운드
	ES2C_TYPE_SERVER_DOWN,			// 서버가 꺼질 경우 이 패킷을 보내 클라이언트를 종료시킨다.

    ES2C_TYPE_MAX,
};

struct S2C_Message : public SHeader
{
    S2C_Message( void )
    {
        usSize = sizeof( *this ) - sizeof( usSize );
        usType = ES2C_TYPE_MESSAGE;
    }

    SYSTEMTIME      sDate;          // 서버가 메시지를 수신한 시각
    char            szIP[16];       // 메시지를 전송한 클라이언트의 아이피
    unsigned short  usPort;         // 메시지를 전송한 클라이언트의 포  트
    char            szMessage[128]; // 메시지 내용
};

struct S2C_PlayerNum : public SHeader
{
	S2C_PlayerNum(void)
	{
		usSize = sizeof(*this) - sizeof(usSize);
		usType = ES2C_TYPE_PLAYERNUM;
		usPlayerNum = -1;
	}

	unsigned short  usPlayerNum;     // 플레이어 번호
};

struct S2C_Turn : public SHeader
{
	S2C_Turn(void)
	{
		usSize = sizeof(*this) - sizeof(usSize);
		usType = ES2C_TYPE_TURN;
        usTurn = -1;
	}

	unsigned short  usTurn;     // 현재 누구 턴인지
	unsigned short  usAttack;	// 현재 누가 공격인지
};

struct S2C_GameState : public SHeader
{
	S2C_GameState(void)
	{
		usSize = sizeof(*this) - sizeof(usSize);
		usType = ES2C_TYPE_GAMESTATE;
        bIsGameStart = false;
		bIsGameWin = false;
	}

	bool  bIsGameStart;     // 클라이언트 측의 게임 시작을 위해 (상대 플레이어가 접속했다고 알려준다.), 게임 종료에도 쓰임
	bool  bIsGameWin;		// 상대방이 나갔을 경우 이 패킷을 받은 클라이언트는 무조건 이긴다.
};

struct S2C_TurnEnd : public SHeader
{
	S2C_TurnEnd(void)
	{
		usSize = sizeof(*this) - sizeof(usSize);
		usType = ES2C_TYPE_TURNEND;
		bIsTurnEnd = false;
		bIsAttack = true;
	}

	bool  bIsTurnEnd;     // 턴이 끝났다고 알려줌
	bool  bIsAttack;	  // 공격(문제 내기)이 끝난건지 수비(문제 맞추기)가 끝난건지
	char  inputKeys[6];	  // 입력받은 키 4개
};

struct S2C_KeyInput : public SHeader
{
	S2C_KeyInput(void)
	{
		usSize = sizeof(*this) - sizeof(usSize);
		usType = ES2C_TYPE_KEYINPUT;
		inputKey = NULL;
	}

	char inputKey;     // 클라이언트에서 누른 키
};

struct S2C_Score : public SHeader
{
	S2C_Score(void)
	{
		usSize = sizeof(*this) - sizeof(usSize);
		usType = ES2C_TYPE_SCORE;
		usScore[0] = 0;
		usScore[1] = 0;
	}

	unsigned short  usScore[2];     // 현재 점수
};

struct S2C_Round : public SHeader
{
	S2C_Round(void)
	{
		usSize = sizeof(*this) - sizeof(usSize);
		usType = ES2C_TYPE_ROUND;
		usRound = 0;
	}

	unsigned short  usRound;     // 현재 라운드
};

struct S2C_ServerDown : public SHeader
{
	S2C_ServerDown(void)
	{
		usSize = sizeof(*this) - sizeof(usSize);
		usType = ES2C_TYPE_SERVER_DOWN;
		bIsServerDown = false;
	}

	bool  bIsServerDown;			// 서버가 종료 되었을 때
};
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// 사용할 패킷 구조체들 1 바이트 정렬 끝
#pragma pack( pop )