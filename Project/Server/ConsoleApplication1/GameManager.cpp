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

	// 2���� -> �� ���� ����, ���� �Դٰ��� �� ����
	m_TotalCount = 10;

	// ũ��Ƽ�� ���� �ʱ�ȭ
	InitializeCriticalSection(&CS_AttackKeyVec);
	InitializeCriticalSection(&CS_DefenceKeyVec);
}

GameManager::~GameManager()
{
	// ũ��Ƽ�� ���� ����
	DeleteCriticalSection(&CS_AttackKeyVec);
	DeleteCriticalSection(&CS_DefenceKeyVec);
}

void GameManager::Begin()
{
	// ó���� �����ִٰ�
	// �÷��̾ ��� �����ϸ� ����
}

void GameManager::End()
{

}

unsigned int GameManager::Run(int dwIndex)
{
	m_IsGameStart = true;

	// ������ ����
	PickFirstTurn();

	// Ŭ���̾�Ʈ���� ���� ������ �˷��ش�.
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

		// ��� Ƚ���� ������ ������ ����ȴ�.
		if (m_TotalCount <= 0)
		{
			EndGame();
			break;
		}

		printf_s("[TCP ����] ���� ���ݱ� : �÷��̾� %d\n", m_NowAttack);

		/// ���� ����

		// ���� �ʱ�ȭ
		m_IsTurnEnd = false;

		// Ŭ���̾�Ʈ ������ ���� �������� �˷��ܰ� ���ÿ� �Է¹��� Ű�� �޾ƿ´�.
		// �� �������� ����Ѵ�.
		while (true)
		{
			if (m_IsTurnEnd)
				break;
		}

		// ���� ������.
		// ���� �ٲ۴�. �÷��̾�� �� ������ �����Ѵ�.

		Sleep(500);
		ChangeTurn();

		/// ���� ���߱�

		// ���� �ʱ�ȭ
		m_IsTurnEnd = false;

		// Ŭ���̾�Ʈ ������ ���� �������� �˷��ܰ� ���ÿ� �Է¹��� Ű�� �޾ƿ´�.
		// �� �������� ����Ѵ�.
		while (true)
		{
			if (m_IsTurnEnd)
				break;
		}

		// ���ھ ����Ѵ�.
		CheckScore();

		// ���ݱ��� �ٲ۴�.
		// �ٲ� ���ݱ� ������ Ŭ���̾�Ʈ�鿡�� �����ش�.
		Sleep(500);
		ChangeAttack();

		m_TotalCount--;
	}

	return 0;
}

void GameManager::PickFirstTurn()
{
	// random_device�� ���� ���� ���� ���� �ʱ�ȭ
	std::random_device rd;
	std::mt19937 gen(rd());

	std::uniform_int_distribution<int> dis(0, 1);
	int randNum = dis(gen);

	if (randNum == 0)
		m_NowTurn = PLAYER1;
	else
		m_NowTurn = PLAYER2;

	// ������ ���� ���� ���
	m_NowAttack = m_NowTurn;

	// ������ �������� �÷��̾�� �˷���
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

	// �ٲ� �� ������ Ŭ���̾�Ʈ�鿡�� �����ش�.
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

	// ���� ���ھ Ŭ���̾�Ʈ�� �����ش�.
	S2C_Score packet;
	packet.usScore[PLAYER1] = m_Score[PLAYER1];
	packet.usScore[PLAYER2] = m_Score[PLAYER2];

	m_ServerManager->BroadcastPacket(&packet);

	printf_s("[TCP ����] ���� ���ھ� �÷��̾� 0 : %d\n", m_Score[PLAYER1]);
	printf_s("[TCP ����] ���� ���ھ� �÷��̾� 1 : %d\n", m_Score[PLAYER2]);
}

void GameManager::EndGame()
{
	// ������ ����Ǿ����� Ŭ���̾�Ʈ���� ������.
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

