#pragma once
#include "Object.h"

/// <summary>
/// 마우스와 상호작용하는 버튼
/// 
/// 2021.02.18 정종영
/// </summary>
class Button : public Object
{
private:
	eNowSceneState m_StateForClicked;	// 이 오브젝트가 눌렸을 때, 이 오브젝트가 있는 씬에서 변경되어야 하는 상태

public:
	eNowSceneState GetStateForClicked() const { return m_StateForClicked; }
	void SetStateForClicked(eNowSceneState val) { m_StateForClicked = val; }

public:
	Button(UISortLayer layer);
	Button(UISortLayer layer, eNowSceneState state);
	virtual ~Button();

public:
	virtual void Initialize(JVector pos, PCWSTR objectName);
	virtual void Release();

	virtual void Update(float dTime);
	virtual void Draw(float opacity = 1.0f);
	virtual void ShowDebug();
};