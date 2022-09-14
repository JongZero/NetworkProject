#include "pch.h"

#include "OutroResultScene.h"
#include "ObjectManager.h"
#include "IntroTitleScene.h"
#include "MainGameScene.h"
#include "Button.h"
#include "ColliderBox.h"

bool OutroResultScene::m_IsFirstPlay[2] = { false, };

OutroResultScene::OutroResultScene()
	:m_Background(nullptr), m_BGMName(L"BGM_hi_title"), m_pExitButton(nullptr)
{

}

OutroResultScene::~OutroResultScene()
{

}

void OutroResultScene::Initialize()
{
	Scene::Initialize();

	InputManager::GetInstance()->Reset();

	// ���������ư
	m_pExitButton = m_pObjectManager->CreateObject_Button(JVector(760, 800), L"NewTitle_ExitButton", 2, UISortLayer::BUTTON, eNowSceneState::GO_TO_NEXT_SCENE);

	// �ƿ�Ʈ�� ���
	m_Background = m_pObjectManager->CreateObject_Image(JVector(0, -15), L"Background_Main", 1, UISortLayer::BACKGROUND);

	if (MainGameScene::GetInstance()->Score[0] > MainGameScene::GetInstance()->Score[1])
	{
		m_WinPlayer = 0;
	}
	else if (MainGameScene::GetInstance()->Score[0] == MainGameScene::GetInstance()->Score[1])
	{
		m_WinPlayer = -1;
	}
	else
		m_WinPlayer = 1;

	if (MainGameScene::GetInstance()->IsGameWin)
		m_WinPlayer = MainGameScene::GetInstance()->PlayerNum;

	m_DanceMan = m_pObjectManager->CreateObject_Image(JVector(300, 100), L"Dance_Man", 5, UISortLayer::BUTTON);
	m_DanceGirl = m_pObjectManager->CreateObject_Image(JVector(1200, 100), L"Dance_Girl", 5, UISortLayer::BUTTON);

	m_Me = m_pObjectManager->CreateObject_Image(JVector(50, 5), L"Me", 1, UISortLayer::BUTTON);
	m_You = m_pObjectManager->CreateObject_Image(JVector(80, -15), L"You", 1, UISortLayer::BUTTON);

	if (MainGameScene::GetInstance()->PlayerNum == 0)
	{
		m_Player = m_DanceMan;
		m_RelativePlayer = m_DanceGirl;

		m_Player->Transform()->SetPos(300, 100);
		m_RelativePlayer->Transform()->SetPos(1220, 100);
	}
	else
	{
		m_Player = m_DanceGirl;
		m_RelativePlayer = m_DanceMan;

		m_Player->Transform()->SetPos(250, 100);
		m_RelativePlayer->Transform()->SetPos(1260, 100);
	}

	// BGM
	SoundManager::GetInstance()->Play(0, m_BGMName.c_str());
	SoundManager::GetInstance()->VolumeDown(0);		// BGM����
}

void OutroResultScene::Update(float dTime)
{
	m_pObjectManager->UpdateAll(dTime);

	if (m_WinPlayer == -1)
	{
		m_Player->UpdateAnimation(dTime);
		m_RelativePlayer->UpdateAnimation(dTime);
	}
	else if (m_WinPlayer == MainGameScene::GetInstance()->PlayerNum || MainGameScene::GetInstance()->IsGameWin)
		m_Player->UpdateAnimation(dTime);
	else
		m_RelativePlayer->UpdateAnimation(dTime);

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

	if (m_pExitButton->GetColliderBox()->IsClicked)
	{
		m_NowSceneState = eNowSceneState::GO_TO_NEXT_SCENE;
	}

	if (m_NowSceneState == eNowSceneState::GO_TO_NEXT_SCENE)
	{
		SoundManager::GetInstance()->Play(1, L"Button_Up_Normal");	// ��ư�� ������ ����ȴ�.
		exit(0);
	}
}

void OutroResultScene::Draw()
{
	m_pObjectManager->DrawAll();

	if (m_WinPlayer != -1)
	{
		if (MainGameScene::GetInstance()->IsGameWin)
		{
			JJEngine::GetInstance()->DrawText(1920 / 2 - 210, 1080 / 4 + 50, 1920, 100, 40, ColorF(ColorF::Black), L" ��밡 ���������ϴ�. ");
			JJEngine::GetInstance()->DrawText(1920 / 2 - 95, 1080 / 4 + 120, 1920, 100, 80, ColorF(ColorF::Black), L" �¸�! ", m_WinPlayer);
		}
		else
		{
			if (m_WinPlayer == MainGameScene::GetInstance()->PlayerNum)
				JJEngine::GetInstance()->DrawText(1920 / 2 - 95, 1080 / 4 + 120, 1920, 100, 80, ColorF(ColorF::Black), L" �¸�! ", m_WinPlayer);
			else
				JJEngine::GetInstance()->DrawText(1920 / 2 - 95, 1080 / 4 + 120, 1920, 100, 80, ColorF(ColorF::Black), L" �й�! ", m_WinPlayer);
		}
	}
	else
		JJEngine::GetInstance()->DrawText(1920 / 2 - 95, 1080 / 4 + 120, 1920, 100, 40, ColorF(ColorF::Black), L" ���º�! ");

	ShowScore();
	//ShowDebug();
}

void OutroResultScene::ShowDebug()
{
	JJEngine::GetInstance()->DrawText(0, 40, L"Outro_Result ��");
}

void OutroResultScene::Release()
{
	m_pObjectManager->Release();

	Scene::Release();
}

void OutroResultScene::ShowScore()
{
	JJEngine::GetInstance()->DrawText(430, 800, 110, ColorF(ColorF::Chocolate), L"%d", MainGameScene::GetInstance()->Score[MainGameScene::GetInstance()->PlayerNum]);

	// ��� �÷��̾��� ��ȣ
	int relativePlayerNum = -1;
	if (MainGameScene::GetInstance()->PlayerNum == 0)
		relativePlayerNum = 1;
	else
		relativePlayerNum = 0;

	JJEngine::GetInstance()->DrawText(1400, 800, 110, ColorF(ColorF::LimeGreen), L"%d", MainGameScene::GetInstance()->Score[relativePlayerNum]);
}
