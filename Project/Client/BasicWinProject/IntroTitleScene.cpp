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

	// ��׶����̹���
	m_pObjectManager->CreateObject_Image(JVector(0, -15), L"Background_Intro", 1, UISortLayer::BACKGROUND);

	// Ÿ��Ʋ��
	m_pTitleBar = m_pObjectManager->CreateObject_Image(JVector(520, -20), L"NewTitle_ani", 6, UISortLayer::BUTTON);

	// ���ӽ��۹�ư
	m_pStartButton = m_pObjectManager->CreateObject_Button(JVector(730, 620), L"NewTitle_StartButton", 2, UISortLayer::BUTTON, eNowSceneState::GO_TO_NEXT_SCENE);
	// ���������ư
	m_pExitButton = m_pObjectManager->CreateObject_Button(JVector(760, 800), L"NewTitle_ExitButton", 2, UISortLayer::BUTTON, eNowSceneState::GO_TO_PREV_SCENE);

	// ���� ����
	//JVector pos, std::wstring name, JVector colliderSize, UISortLayer layer, eNowSceneState state
	m_pBGMOffButton = m_pObjectManager->CreateObject_Button(JVector(1920 - 200, 1080 - 135), L"BGM OFF", JVector(200, 100), UISortLayer::BUTTON, eNowSceneState::NONE);

	SoundManager::GetInstance()->Play(0, m_BGMName.c_str());		// BGM����
	SoundManager::GetInstance()->VolumeDown(0);		// BGM����
}

void IntroTitleScene::Update(float dTime)
{
	// ���콺�� �ö��ֳ� �ȿö��ֳ�?
	m_pObjectManager->UpdateAll(dTime);

	m_pObjectManager->CheckButtonClicked(this);

	m_pTitleBar->UpdateAnimation(dTime);

	/// ���۹�ư�� �÷��ξ��� ��
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

	// ����
	/// �����ư�� �÷��ξ��� �� 
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
			SoundManager::GetInstance()->Play(1, L"Button_Up_Normal");	// ��ư�� ������ ����ȴ�.
			SceneManager::ChangeScene(eSceneStateAll::MAIN_GAME);
		}
		return;

		case eNowSceneState::GO_TO_PREV_SCENE:
			SoundManager::GetInstance()->Play(1, L"Button_Up_Normal");	// ��ư�� ������ ����ȴ�.
			exit(0);
			return;
	}
}

void IntroTitleScene::Draw()
{
	m_pObjectManager->DrawAll();

	//SceneManager::DrawBlackScreen();

	// ���̵� �ƿ��� ������ ��츸 �� ������Ʈ (���۵� ����)
	if (!SceneManager::IsFadeOutEnd)
		return;
}


void IntroTitleScene::ShowDebug()
{
	Scene::ShowDebug();

	JJEngine::GetInstance()->DrawText(0, 40, L"Intro_Title ��");
}

void IntroTitleScene::Release()
{
	Scene::Release();

	SoundManager::GetInstance()->Stop(0, m_BGMName.c_str());
}