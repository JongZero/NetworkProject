#pragma once

// ����� ��Ŷ ����ü�� 1 ����Ʈ ���� ����
#pragma pack( push, 1 )

struct SHeader
{
    unsigned short usSize;          // ��Ŷ�� ������
    unsigned short usType;          // ��Ŷ�� Ÿ��
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// [Ŭ���̾�Ʈ -> ����] ��Ŷ�� Ÿ�� ����
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
enum EC2S_TYPE
{
    EC2S_TYPE_MESSAGE = 0,          // ä�� �޼��� ���� ��û
    EC2S_TYPE_PLAYERNUM,            // �÷��̾��� ��ȣ
    EC2S_TYPE_GAMESTATE,            // ���� ����, ���� ���� �� ���ῡ ����
    EC2S_TYPE_TURN,                 // ���� ���� ������
    
	EC2S_TYPE_TURNEND,				// �� ���� ����
    EC2S_TYPE_KEYINPUT,             // Ŭ���̾�Ʈ���� �Է��� Ű (���濡�� �����־� �Ȱ��� ���� ����ϵ��� �Ѵ�.)
	EC2S_TYPE_SCORE,				// ���� ����

    EC2S_TYPE_MAX,
};

struct C2S_Message : public SHeader
{
    C2S_Message( void )
    {
        usSize = sizeof( *this ) - sizeof( usSize );
        usType = EC2S_TYPE_MESSAGE;
    }

    char            szMessage[128]; // �޽��� ����
};

struct C2S_PlayerNum : public SHeader
{
    C2S_PlayerNum(void)
	{
		usSize = sizeof(*this) - sizeof(usSize);
		usType = EC2S_TYPE_PLAYERNUM;
        usPlayerNum = -1;
	}

	unsigned short  usPlayerNum;     // �÷��̾� ��ȣ
};

struct C2S_Turn : public SHeader
{
    C2S_Turn(void)
	{
		usSize = sizeof(*this) - sizeof(usSize);
		usType = EC2S_TYPE_TURN;
        usTurn = -1;
	}

    unsigned short  usTurn;     // ���� ���� ������
	unsigned short  usAttack;	// ���� ���� ��������
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

	bool  bIsGameStart;     // Ŭ���̾�Ʈ ���� ���� ������ ���� (��� �÷��̾ �����ߴٰ� �˷��ش�.), ���� ���ῡ�� ����
	bool  bIsGameWin;		// ������ ������ ��� �� ��Ŷ�� ���� Ŭ���̾�Ʈ�� ������ �̱��.
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

	bool  bIsTurnEnd;     // ���� �����ٰ� �˷���
	bool  bIsAttack;	  // ����(���� ����)�� �������� ����(���� ���߱�)�� ��������
	char  inputKeys[6];	  // �Է¹��� Ű 6��
};

struct C2S_KeyInput : public SHeader
{
	C2S_KeyInput(void)
	{
		usSize = sizeof(*this) - sizeof(usSize);
		usType = EC2S_TYPE_KEYINPUT;
		inputKey = NULL;
	}

	char inputKey;     // Ŭ���̾�Ʈ���� ���� Ű
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

	unsigned short  usScore[2];     // ���� ����
};
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// [���� -> Ŭ���̾�Ʈ] ��Ŷ�� Ÿ�� ����
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
enum ES2C_TYPE
{
    ES2C_TYPE_MESSAGE = 0,          // ������ ä�� �޼��� ��ε� ĳ����
    ES2C_TYPE_PLAYERNUM,            // �÷��̾��� ��ȣ
    ES2C_TYPE_GAMESTATE,            // ���� ����, ���� ���� �� ���ῡ ����
    ES2C_TYPE_TURN,                 // ���� ���� ������

	ES2C_TYPE_TURNEND,				// �� ���� ����
	ES2C_TYPE_KEYINPUT,				// Ŭ���̾�Ʈ���� �Է��� Ű (���濡�� �����־� �Ȱ��� ���� ����ϵ��� �Ѵ�.)
	ES2C_TYPE_SCORE,				// ���� ����

	ES2C_TYPE_ROUND,				// ���� ����
	ES2C_TYPE_SERVER_DOWN,			// ������ ���� ��� �� ��Ŷ�� ���� Ŭ���̾�Ʈ�� �����Ų��.

    ES2C_TYPE_MAX,
};

struct S2C_Message : public SHeader
{
    S2C_Message( void )
    {
        usSize = sizeof( *this ) - sizeof( usSize );
        usType = ES2C_TYPE_MESSAGE;
    }

    SYSTEMTIME      sDate;          // ������ �޽����� ������ �ð�
    char            szIP[16];       // �޽����� ������ Ŭ���̾�Ʈ�� ������
    unsigned short  usPort;         // �޽����� ������ Ŭ���̾�Ʈ�� ��  Ʈ
    char            szMessage[128]; // �޽��� ����
};

struct S2C_PlayerNum : public SHeader
{
	S2C_PlayerNum(void)
	{
		usSize = sizeof(*this) - sizeof(usSize);
		usType = ES2C_TYPE_PLAYERNUM;
		usPlayerNum = -1;
	}

	unsigned short  usPlayerNum;     // �÷��̾� ��ȣ
};

struct S2C_Turn : public SHeader
{
	S2C_Turn(void)
	{
		usSize = sizeof(*this) - sizeof(usSize);
		usType = ES2C_TYPE_TURN;
        usTurn = -1;
	}

	unsigned short  usTurn;     // ���� ���� ������
	unsigned short  usAttack;	// ���� ���� ��������
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

	bool  bIsGameStart;     // Ŭ���̾�Ʈ ���� ���� ������ ���� (��� �÷��̾ �����ߴٰ� �˷��ش�.), ���� ���ῡ�� ����
	bool  bIsGameWin;		// ������ ������ ��� �� ��Ŷ�� ���� Ŭ���̾�Ʈ�� ������ �̱��.
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

	bool  bIsTurnEnd;     // ���� �����ٰ� �˷���
	bool  bIsAttack;	  // ����(���� ����)�� �������� ����(���� ���߱�)�� ��������
	char  inputKeys[6];	  // �Է¹��� Ű 4��
};

struct S2C_KeyInput : public SHeader
{
	S2C_KeyInput(void)
	{
		usSize = sizeof(*this) - sizeof(usSize);
		usType = ES2C_TYPE_KEYINPUT;
		inputKey = NULL;
	}

	char inputKey;     // Ŭ���̾�Ʈ���� ���� Ű
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

	unsigned short  usScore[2];     // ���� ����
};

struct S2C_Round : public SHeader
{
	S2C_Round(void)
	{
		usSize = sizeof(*this) - sizeof(usSize);
		usType = ES2C_TYPE_ROUND;
		usRound = 0;
	}

	unsigned short  usRound;     // ���� ����
};

struct S2C_ServerDown : public SHeader
{
	S2C_ServerDown(void)
	{
		usSize = sizeof(*this) - sizeof(usSize);
		usType = ES2C_TYPE_SERVER_DOWN;
		bIsServerDown = false;
	}

	bool  bIsServerDown;			// ������ ���� �Ǿ��� ��
};
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// ����� ��Ŷ ����ü�� 1 ����Ʈ ���� ��
#pragma pack( pop )