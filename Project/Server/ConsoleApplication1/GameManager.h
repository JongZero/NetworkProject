#pragma once
#include "BaseThread.h"
class ServerManager;

/// <summary>
/// 게임 로직을 돌리는 게임 매니저
/// 2021. 05. 14
/// 정종영
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

	void PickFirstTurn();	// 첫 턴을 결정(랜덤)
	void ChangeTurn();		// 턴 바꿈
	void ChangeAttack();	// 공격권 바꿈
	void CheckScore();		// 맞췄는지 못 맞췄는지 체크
	void EndGame();			// 게임을 끝낸다.

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