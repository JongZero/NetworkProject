#include "GameManager.h"
#include "ServerManager.h"
#include <random>

GameManager::GameManager(ServerManager* serverManager)
	: m_ServerManager(serverManager), m_NowTurn(0), m_PlayerCount(0),
	m_IsGameStart(false), m_NowAttack(-1), m_IsTurnEnd(false)
{
	m_Player[0] = INVALID_SOCKET;
	m_Player[1] = INVALID_SOCKET;

	m_Score[PLAYER1] = 0;
	m_Score[PLAYER2] = 0;

	// 2번이 -> 한 번씩 공격, 수비 왔다갔다 한 것임
	m_TotalCount = 10;

	// 크리티컬 섹션 초기화
	InitializeCriticalSection(&CS_AttackKeyVec);
	InitializeCriticalSection(&CS_DefenceKeyVec);
}

GameManager::~GameManager()
{
	// 크리티컬 섹션 종료
	DeleteCriticalSection(&CS_AttackKeyVec);
	DeleteCriticalSection(&CS_DefenceKeyVec);
}

void GameManager::Begin()
{
	// 처음엔 멈춰있다가
	// 플레이어가 모두 접속하면 시작
}

void GameManager::End()
{

}

unsigned int GameManager::Run(int dwIndex)
{
	m_IsGameStart = true;

	// 선공권 결정
	PickFirstTurn();

	// 클라이언트에게 게임 시작을 알려준다.
	S2C_GameState gs;
	gs.bIsGameStart = true;
	m_ServerManager->BroadcastPacket(&gs);
	
	while (!m_bClose)
	{
		if (m_TotalCount % 2 == 0)
		{
			S2C_Round packet;
			packet.usRound = m_TotalCount / 2;
			m_ServerManager->BroadcastPacket(&packet);
		}

		// 모든 횟수가 끝나면 게임이 종료된다.
		if (m_TotalCount <= 0)
		{
			EndGame();
			break;
		}

		printf_s("[TCP 서버] 현재 공격권 : 플레이어 %d\n", m_NowAttack);

		/// 문제 내기

		// 변수 초기화
		m_IsTurnEnd = false;

		// 클라이언트 측에서 턴이 끝났음을 알려줌과 동시에 입력받은 키를 받아온다.
		// 그 전까지는 대기한다.
		while (true)
		{
			if (m_IsTurnEnd)
				break;
		}

		// 턴이 끝났다.
		// 턴을 바꾼다. 플레이어에게 턴 정보를 전송한다.

		Sleep(500);
		ChangeTurn();

		/// 문제 맞추기

		// 변수 초기화
		m_IsTurnEnd = false;

		// 클라이언트 측에서 턴이 끝났음을 알려줌과 동시에 입력받은 키를 받아온다.
		// 그 전까지는 대기한다.
		while (true)
		{
			if (m_IsTurnEnd)
				break;
		}

		// 스코어를 계산한다.
		CheckScore();

		// 공격권을 바꾼다.
		// 바꾼 공격권 정보를 클라이언트들에게 보내준다.
		Sleep(500);
		ChangeAttack();

		m_TotalCount--;
	}

	return 0;
}

void GameManager::PickFirstTurn()
{
	// random_device를 통한 난수 생성 엔진 초기화
	std::random_device rd;
	std::mt19937 gen(rd());

	std::uniform_int_distribution<int> dis(0, 1);
	int randNum = dis(gen);

	if (randNum == 0)
		m_NowTurn = PLAYER1;
	else
		m_NowTurn = PLAYER2;

	// 선공은 현재 턴인 사람
	m_NowAttack = m_NowTurn;

	// 정해진 선공권을 플레이어에게 알려줌
	S2C_Turn turn;
	turn.usTurn = m_NowTurn;
	turn.usAttack = m_NowAttack;

	m_ServerManager->BroadcastPacket(&turn);
}

void GameManager::ChangeTurn()
{
	if (m_NowTurn == PLAYER1)
		m_NowTurn = PLAYER2;
	else
		m_NowTurn = PLAYER1;

	// 바꾼 턴 정보를 클라이언트들에게 보내준다.
	S2C_Turn turn;
	turn.usTurn = m_NowTurn;
	turn.usAttack = m_NowAttack;

	m_ServerManager->BroadcastPacket(&turn);
}

void GameManager::ChangeAttack()
{
	if (m_NowAttack == PLAYER1)
		m_NowAttack = PLAYER2;
	else
		m_NowAttack = PLAYER1;

	S2C_Turn turn;
	turn.usTurn = m_NowTurn;
	turn.usAttack = m_NowAttack;

	m_ServerManager->BroadcastPacket(&turn);
}

void GameManager::CheckScore()
{
	int score = 0;

	{	// Lock
		Lock _lock(&CS_AttackKeyVec);
		Lock _lock2(&CS_DefenceKeyVec);

		for (int i = 0; i < 6; i++)
		{
			if (m_AttackKeyVec[i] == m_DefenceKeyVec[i] && m_AttackKeyVec[i] != NULL)
				score += 10;
		}
	}	// Unlock
	
	int defencePlayer = -1;

	if (m_NowAttack == PLAYER1)
		defencePlayer = PLAYER2;
	else
		defencePlayer = PLAYER1;

	m_Score[defencePlayer] += score;

	// 계산된 스코어를 클라이언트에 보내준다.
	S2C_Score packet;
	packet.usScore[PLAYER1] = m_Score[PLAYER1];
	packet.usScore[PLAYER2] = m_Score[PLAYER2];

	m_ServerManager->BroadcastPacket(&packet);

	printf_s("[TCP 서버] 현재 스코어 플레이어 0 : %d\n", m_Score[PLAYER1]);
	printf_s("[TCP 서버] 현재 스코어 플레이어 1 : %d\n", m_Score[PLAYER2]);
}

void GameManager::EndGame()
{
	// 게임이 종료되었음을 클라이언트에게 보낸다.
	S2C_GameState endPacket;

	endPacket.bIsGameStart = false;
	m_ServerManager->BroadcastPacket(&endPacket);
}

SOCKET GameManager::GetRelativePlayer(SOCKET socket)
{
	if (socket == m_Player[0])
	{
		return m_Player[1];
	}
	else
	{
		return m_Player[0];
	}
}

void GameManager::LeavePlayer(SOCKET player)
{
	if (player == m_Player[0])
	{
		m_Player[0] = INVALID_SOCKET;
		m_PlayerCount = 0;
	}
	else
	{
		m_Player[1] = INVALID_SOCKET;
		m_PlayerCount = 1;
	}
}

void GameManager::AddAttackKey(char c)
{
	{	
		Lock _lock(&CS_AttackKeyVec);
		m_AttackKeyVec.push_back(c); 
	}
}

void GameManager::AttackKeyReset()
{
	{ 
		Lock _lock(&CS_AttackKeyVec);
		m_AttackKeyVec.clear(); 
	}
}

void GameManager::AddDefenceKey(char c)
{
	{ 
		Lock _lock(&CS_DefenceKeyVec);
		m_DefenceKeyVec.push_back(c); 
	}
}

void GameManager::DefenceKeyReset()
{
	{ 
		Lock _lock(&CS_DefenceKeyVec);
		m_DefenceKeyVec.clear();
	}
}

