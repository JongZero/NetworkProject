#pragma once
#include "BaseThread.h"
class ServerManager;

/// <summary>
/// ���� ������ ������ ���� �Ŵ���
/// 2021. 05. 14
/// ������
/// </summary>
class GameManager : public BaseThread
{
public:
	GameManager(ServerManager* serverManager);
	virtual ~GameManager();

private:
	ServerManager*			m_ServerManager;

	bool m_IsGameStart;

	enum PLAYER
	{
		PLAYER1,
		PLAYER2
	};

	SOCKET m_Player[2];
	int m_PlayerCount;
	int m_NowTurn;
	int m_NowAttack;
	int m_Score[2];

	bool m_IsTurnEnd;

	int m_TotalCount;
	int m_NowRound;

	std::vector<char> m_AttackKeyVec;
	std::vector<char> m_DefenceKeyVec;
	CRITICAL_SECTION    CS_AttackKeyVec;
	CRITICAL_SECTION    CS_DefenceKeyVec;

private:
	virtual void			Begin();
	virtual void			End();
	virtual unsigned int	Run(int dwIndex);

	void PickFirstTurn();	// ù ���� ����(����)
	void ChangeTurn();		// �� �ٲ�
	void ChangeAttack();	// ���ݱ� �ٲ�
	void CheckScore();		// ������� �� ������� üũ
	void EndGame();			// ������ ������.

public:
	SOCKET GetRelativePlayer(SOCKET socket);
	int GetPlayerCount() { return m_PlayerCount; }
	void SetIsTurnEnd(bool val) { m_IsTurnEnd = val; }
	void SetNowTurn(int val) { m_NowTurn = val; }

public:
	void EnterPlayer(SOCKET player) { m_Player[m_PlayerCount] = player; m_PlayerCount++; }
	void LeavePlayer(SOCKET player);
	void AddAttackKey(char c);
	void AttackKeyReset();
	void AddDefenceKey(char c); 
	void DefenceKeyReset();
	bool IsGameStart() { return m_IsGameStart; }
};