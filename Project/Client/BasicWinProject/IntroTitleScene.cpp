#include <stdlib.h>
#include "pch.h"
#include "IntroTitleScene.h"
#include "Object.h"
#include "ObjectManager.h"
#include "Button.h"
#include "ColliderBox.h"

bool IntroTitleScene::m_IsFirstPlay[2] = { false, };

IntroTitleScene::IntroTitleScene()
	:m_pBackGround(nullptr),
	m_pTitleBar(nullptr),
	m_pStartButton(nullptr),
	m_pExitButton(nullptr),
	m_pNowSceneState(eSceneStateAll::INTRO_TITLE),
	m_BGMName(L"BGM_hi_title")
{

}

IntroTitleScene::~IntroTitleScene()
{

}

void IntroTitleScene::Initialize()
{
	Scene::Initialize();
	InputManager::GetInstance()->Reset();

	// 백그라운드이미지
	m_pObjectManager->CreateObject_Image(JVector(0, -15), L"Background_Intro", 1, UISortLayer::BACKGROUND);

	// 타이틀바
	m_pTitleBar = m_pObjectManager->CreateObject_Image(JVector(520, -20), L"NewTitle_ani", 6, UISortLayer::BUTTON);

	// 게임시작버튼
	m_pStartButton = m_pObjectManager->CreateObject_Button(JVector(730, 620), L"NewTitle_StartButton", 2, UISortLayer::BUTTON, eNowSceneState::GO_TO_NEXT_SCENE);
	// 게임종료버튼
	m_pExitButton = m_pObjectManager->CreateObject_Button(JVector(760, 800), L"NewTitle_ExitButton", 2, UISortLayer::BUTTON, eNowSceneState::GO_TO_PREV_SCENE);

	// 음악 끄기
	//JVector pos, std::wstring name, JVector colliderSize, UISortLayer layer, eNowSceneState state
	m_pBGMOffButton = m_pObjectManager->CreateObject_Button(JVector(1920 - 200, 1080 - 135), L"BGM OFF", JVector(200, 100), UISortLayer::BUTTON, eNowSceneState::NONE);

	SoundManager::GetInstance()->Play(0, m_BGMName.c_str());		// BGM사운드
	SoundManager::GetInstance()->VolumeDown(0);		// BGM사운드
}

void IntroTitleScene::Update(float dTime)
{
	// 마우스가 올라가있냐 안올라가있냐?
	m_pObjectManager->UpdateAll(dTime);

	m_pObjectManager->CheckButtonClicked(this);

	m_pTitleBar->UpdateAnimation(dTime);

	/// 시작버튼에 올려두었을 때
	if (m_pStartButton->GetColliderBox()->IsCollided)
	{
		m_pStartButton->ChangeFrame(1);

		if (m_IsFirstPlay[0] == false)
		{
			SoundManager::GetInstance()->Play(1, L"Button_Over_Sample_01");
			m_IsFirstPlay[0] = true;
		}
	}
	else
	{
		m_pStartButton->ChangeFrame(0);
		m_IsFirstPlay[0] = false;
	}

	// 생성
	/// 종료버튼에 올려두었을 때 
	if (m_pExitButton->GetColliderBox()->IsCollided)
	{
		m_pExitButton->ChangeFrame(1);

		if (m_IsFirstPlay[1] == false)
		{
			SoundManager::GetInstance()->Play(1, L"Button_Over_Sample_01");
			m_IsFirstPlay[1] = true;
		}
	}
	else
	{
		m_pExitButton->ChangeFrame(0);
		m_IsFirstPlay[1] = false;
	}

	if (m_pBGMOffButton->GetColliderBox()->IsClicked)
	{
		SoundManager::GetInstance()->Release();
	}

	switch (m_NowSceneState)
	{
		case eNowSceneState::NONE:
			break;

		case eNowSceneState::GO_TO_NEXT_SCENE:
		{
			SoundManager::GetInstance()->Play(1, L"Button_Up_Normal");	// 버튼업 했을때 실행된다.
			SceneManager::ChangeScene(eSceneStateAll::MAIN_GAME);
		}
		return;

		case eNowSceneState::GO_TO_PREV_SCENE:
			SoundManager::GetInstance()->Play(1, L"Button_Up_Normal");	// 버튼업 했을때 실행된다.
			exit(0);
			return;
	}
}

void IntroTitleScene::Draw()
{
	m_pObjectManager->DrawAll();

	//SceneManager::DrawBlackScreen();

	// 페이드 아웃이 끝났을 경우만 씬 업데이트 (오작동 방지)
	if (!SceneManager::IsFadeOutEnd)
		return;
}


void IntroTitleScene::ShowDebug()
{
	Scene::ShowDebug();

	JJEngine::GetInstance()->DrawText(0, 40, L"Intro_Title 씬");
}

void IntroTitleScene::Release()
{
	Scene::Release();

	SoundManager::GetInstance()->Stop(0, m_BGMName.c_str());
}