#pragma once
#include "Object.h"

/// <summary>
/// ���콺�� ��ȣ�ۿ��ϴ� ��ư
/// 
/// 2021.02.18 ������
/// </summary>
class Button : public Object
{
private:
	eNowSceneState m_StateForClicked;	// �� ������Ʈ�� ������ ��, �� ������Ʈ�� �ִ� ������ ����Ǿ�� �ϴ� ����

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