#pragma once

class Button;
class ObjectManager;

enum class eSceneStateAll
{
	NONE,
	INTRO_TITLE,
	MAIN_GAME,
	OUTRO_RESULT,
};

/// ��� ���� ���� (�� ������ �߰��ٶ�)
enum class eNowSceneState
{
	NONE,
	FADE_OUT,
	GO_TO_NEXT_SCENE,
	GO_TO_PREV_SCENE,
};

/// ChangeSceneButton �迭�� ��������� �����ϱ� ���� enum
enum PrevNextButton
{
	PREV,
	NEXT
};

/// <summary>
/// �� �������̽�
/// 
/// �� ���� ���� �θ� Ŭ����
/// 2021. 02. 06. Hacgeum
/// (����)2021. 02. 18. ������
/// </summary>
class Scene
{
public:
	Scene();
	virtual ~Scene();

	// �ʱ�ȭ
	virtual void Initialize();
	virtual void Update(float dTime) abstract;
	virtual void Draw() abstract;
	virtual void ShowDebug();
	virtual void Release();

protected:
	ObjectManager* m_pObjectManager;
	eNowSceneState m_NowSceneState;		// ���� ���� ����
	
	bool m_IsGameEnd;

public:
	void SetIsGameEnd(bool val) { m_IsGameEnd = val; }
	void SetNowSceneState(eNowSceneState val) { m_NowSceneState = val; }
};
