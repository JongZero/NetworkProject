#pragma once
#include "Scene.h"

class Object;
class Button;
class Player;

/// <summary>
/// 메인게임 씬
/// 2021. 02. 08. Hacgeum
/// </summary>
class MainGameScene : public Scene
{
private:
	MainGameScene();
	virtual ~MainGameScene();

private:
	static MainGameScene* m_Instance;

private:
	enum PLAYER
	{
		PLAYER1,
		PLAYER2
	};

	int m_NowTurn;		// 현재 누구 턴인지
	int m_NowAttackPlayer;	// 현재 공격권을 가진 플레이어인지

	bool m_IsGetPlayerNum;
	bool m_IsGameStart;	// 상대가 들어와서 게임이 시작해야되는지
	bool m_EndMyTurn;	// 내 턴이 종료

	enum GAME_STATE
	{
		STATE_MY_TURN,
		STATE_RELATIVE_PLAYER_TURN,
	};

	enum ATTACK_DEFENCE_STATE
	{
		STATE_ATTACK,
		STATE_DEFENCE,
	};

	int m_NowState;
	int m_NowAttackDefenceState;

	char m_InputKeys[6];		// 입력받은 키

	float m_RemainingTime;		// 남은 시간
	float m_DelayTime;			// 턴이 넘어갈 때 마다 약간의 딜레이

	int m_InputCount;			// 최대로 입력받을 숫자

	int m_RemainingRound;		// 남은 라운드
	wstring m_BGMName;

private:
	Object* m_Background;
	Object* m_Loading;
	Object* m_Waiting;
	Object* m_WaitingBackground;
	Object* m_Warning;

	Object* m_DanceMan;					// 춤추는 남자
	Object* m_DanceGirl;				// 춤추는 여자
	Object* m_Player;
	Object* m_GuidePlayer;
	Object* m_RelativePlayer;
	Object* m_Me;
	Object* m_You;

	Object* m_Question;
	Object* m_Answer;
	Object* m_GameEnd;
	Object* m_GameEndBackground;
	Object* m_Arrow;

	Object* m_1Round;
	Object* m_2Round;
	Object* m_3Round;
	Object* m_4Round;
	Object* m_LastRound;

	Button* m_pPrevScene;

public:
	static int PlayerNum;	// 내가 몇 번 플레이어인지
	static int Score[2];		// 나와 상대의 점수
	static bool IsGameWin;	// 중간에 한 명이 나가서 이겼나?
	static bool IsServerDown;	// 서버가 멈췄는가?

	void SetPlayerNum(int val) { PlayerNum = val; m_IsGetPlayerNum = true; }
	void SetNowTurn(int val) { m_NowTurn = val; }

	int GetScore(int playerNum) const { return Score[playerNum]; }
	void SetScore(int score0, int score1) { Score[0] = score0; Score[1] = score1; }

	void SetIsGameStart(bool val) { m_IsGameStart = val; }
	void SetNowAttackPlayer(int val) { m_NowAttackPlayer = val; }

	void SetDelayTime(float time) { m_DelayTime = time; }
	bool GetIsGameWin() const { return IsGameWin; }
	void SetIsGameWin(bool val) { IsGameWin = val; }

	void SetEndMyTurn(bool val) { m_EndMyTurn = val; }

	void SetRemainingRound(int val) { m_RemainingRound = val; }
	void SetIsServerDown(bool val) { IsServerDown = val; }

public:
	static bool IsOncePlay[11];							// 텍스트
	static bool m_IsFirstPlay[13];						// 사운드

public:
	static MainGameScene* GetInstance();
	void DeleteInstance();

public:
	// Scene을(를) 통해 상속됨
	virtual void Initialize() override;
	virtual void Update(float dTime) override;
	virtual void Draw() override;
	virtual void ShowDebug() override;
	virtual void Release() override;

	void InputKey();
	void ResetRelativePlayerFrame();
	void RelativePlayerAction(char c);

private:
	void AddKey(char c);
	void ShowScore();
	void EndTurn();
};