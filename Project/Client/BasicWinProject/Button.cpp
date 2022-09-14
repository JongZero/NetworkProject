#include "pch.h"

#include "Button.h"
#include "ColliderBox.h"

Button::Button(UISortLayer layer, eNowSceneState state)
	:Object(layer), m_StateForClicked(state)
{
	
}

Button::Button(UISortLayer layer)
	:Object(layer), m_StateForClicked(eNowSceneState::NONE)
{

}

Button::~Button()
{

}

void Button::Initialize(JVector pos, PCWSTR objectName)
{
	Object::Initialize(pos, objectName);
}

void Button::Release()
{
	Object::Release();
}

void Button::Update(float dTime)
{
	// 부모에 있는 중점 업데이트 사용
	Object::Update(dTime);
}

void Button::Draw(float opacity/*=1.0f*/)
{
	if (m_SpriteVec.size() <= 0)
	{

	}
	else
		JJEngine::GetInstance()->DrawSprite(m_SpriteVec[m_SpriteIndex], m_Transform->Pos.x, m_Transform->Pos.y, opacity);

	ShowDebug();
}

void Button::ShowDebug()
{
	Object::ShowDebug();
}