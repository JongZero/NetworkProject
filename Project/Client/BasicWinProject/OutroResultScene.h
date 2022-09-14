#pragma once
#include "Scene.h"

class ObjectManager;
class Object;

/// <summary>
/// 아웃트로 씬
/// 
/// 부검을 통해 찾은 증거로 재판 결과를 보여줌
/// 2021. 02. 08. Hacgeum
/// </summary>
class OutroResultScene : public Scene
{
public:
	OutroResultScene();
	virtual ~OutroResultScene();

private:
	// 배경화면
	Object* m_Background;
	Button* m_pExitButton;
	wstring m_BGMName;

	Object* m_DanceMan;					// 춤추는 남자
	Object* m_DanceGirl;				// 춤추는 여자

	Object* m_Player;
	Object* m_RelativePlayer;

	Object* m_Me;
	Object* m_You;

	int m_WinPlayer;

public:
	static bool m_IsFirstPlay[2];

public:
	// Scene을(를) 통해 상속됨
	virtual void Initialize() override;
	virtual void Update(float dTime) override;
	virtual void Draw() override;
	virtual void ShowDebug() override;
	virtual void Release() override;

private:
	void ShowScore();
};