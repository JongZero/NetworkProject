#pragma once
#include "Scene.h"

class ObjectManager;
class Object;

/// <summary>
/// �ƿ�Ʈ�� ��
/// 
/// �ΰ��� ���� ã�� ���ŷ� ���� ����� ������
/// 2021. 02. 08. Hacgeum
/// </summary>
class OutroResultScene : public Scene
{
public:
	OutroResultScene();
	virtual ~OutroResultScene();

private:
	// ���ȭ��
	Object* m_Background;
	Button* m_pExitButton;
	wstring m_BGMName;

	Object* m_DanceMan;					// ���ߴ� ����
	Object* m_DanceGirl;				// ���ߴ� ����

	Object* m_Player;
	Object* m_RelativePlayer;

	Object* m_Me;
	Object* m_You;

	int m_WinPlayer;

public:
	static bool m_IsFirstPlay[2];

public:
	// Scene��(��) ���� ��ӵ�
	virtual void Initialize() override;
	virtual void Update(float dTime) override;
	virtual void Draw() override;
	virtual void ShowDebug() override;
	virtual void Release() override;

private:
	void ShowScore();
};