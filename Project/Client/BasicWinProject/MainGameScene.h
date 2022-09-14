#pragma once
#include "Scene.h"

class Object;
class Button;
class Player;

/// <summary>
/// ���ΰ��� ��
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

	int m_NowTurn;		// ���� ���� ������
	int m_NowAttackPlayer;	// ���� ���ݱ��� ���� �÷��̾�����

	bool m_IsGetPlayerNum;
	bool m_IsGameStart;	// ��밡 ���ͼ� ������ �����ؾߵǴ���
	bool m_EndMyTurn;	// �� ���� ����

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

	char m_InputKeys[6];		// �Է¹��� Ű

	float m_RemainingTime;		// ���� �ð�
	float m_DelayTime;			// ���� �Ѿ �� ���� �ణ�� ������

	int m_InputCount;			// �ִ�� �Է¹��� ����

	int m_RemainingRound;		// ���� ����
	wstring m_BGMName;

private:
	Object* m_Background;
	Object* m_Loading;
	Object* m_Waiting;
	Object* m_WaitingBackground;
	Object* m_Warning;

	Object* m_DanceMan;					// ���ߴ� ����
	Object* m_DanceGirl;				// ���ߴ� ����
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
	static int PlayerNum;	// ���� �� �� �÷��̾�����
	static int Score[2];		// ���� ����� ����
	static bool IsGameWin;	// �߰��� �� ���� ������ �̰峪?
	static bool IsServerDown;	// ������ ����°�?

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
	static bool IsOncePlay[11];							// �ؽ�Ʈ
	static bool m_IsFirstPlay[13];						// ����

public:
	static MainGameScene* GetInstance();
	void DeleteInstance();

public:
	// Scene��(��) ���� ��ӵ�
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