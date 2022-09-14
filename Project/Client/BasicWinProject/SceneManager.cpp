#include "pch.h"

#include "SceneManager.h"
#include "IntroTitleScene.h"
#include "MainGameScene.h"
#include "OutroResultScene.h"

// static ��� ���� �ʱ�ȭ
eSceneStateAll SceneManager::SceneState = eSceneStateAll::NONE;				// ���� ù ���� State�� Intro�� ����
Scene* SceneManager::pNowScene = nullptr;
bool SceneManager::IsSceneEnd = false;
float SceneManager::Opacity = 0;
bool SceneManager::IsFadeInEnd = false;
bool SceneManager::IsFadeOutEnd = false;
float SceneManager::FadeInFadeOutTime = 0;
ID2D1Bitmap* SceneManager::BlackScreen = nullptr;

SceneManager::SceneManager()
{

}

SceneManager::~SceneManager()
{

}

void SceneManager::Initialize()
{
	// FadeInFadeOut�� ��ũ�� ����
	BlackScreen = ResourceManager::GetInstance()->GetMyImage(L"Black");
}

void SceneManager::Update(float dTime)
{
	if (pNowScene)
	{
		pNowScene->Update(dTime);
	}
}

void SceneManager::Render()
{
	if (pNowScene)
	{
		pNowScene->Draw();
	}
}

void SceneManager::ChangeScene(eSceneStateAll nextScene)
{
	// �� ��ȯ
	if (SceneState != nextScene)
	{
		IsFadeOutEnd = false;

		// ���� �� ����(Safe Delete)
		if (pNowScene)
		{
			pNowScene->Release();
			delete pNowScene;

			// ���� �� nullptr�� �ʱ�ȭ
			pNowScene = nullptr;
		}

		switch (nextScene)
		{
			case eSceneStateAll::INTRO_TITLE:
			{
				// ��Ʈ�� Ÿ��Ʋ ����
				pNowScene = new IntroTitleScene();
				pNowScene->Initialize();
				SceneState = nextScene;
			}
			break;

			case eSceneStateAll::MAIN_GAME:
			{
				// ���� �繫�� ����
				pNowScene = MainGameScene::GetInstance();
				pNowScene->Initialize();
				SceneState = nextScene;
			}
			break;

			case eSceneStateAll::OUTRO_RESULT:
			{
				// �ƿ�Ʈ�� ��� ����
				pNowScene = new OutroResultScene();
				pNowScene->Initialize();
				SceneState = nextScene;
			}
			break;
		}
	}
}

void SceneManager::Release()
{
	if (pNowScene)
	{
		pNowScene->Release();
	}
}

void SceneManager::FadeIn(float dTime)
{
	FadeInFadeOutTime += dTime * 30;

	if (FadeInFadeOutTime >= 1)
	{
		FadeInFadeOutTime = 0;

		if (Opacity < 1.0f && IsFadeInEnd == false)
			Opacity += 0.05f;
		else
			IsFadeInEnd = true;
	}
}

void SceneManager::FadeOut(float dTime)
{
	FadeInFadeOutTime += dTime * 30;

	if (FadeInFadeOutTime >= 1)
	{
		FadeInFadeOutTime = 0;

		if (IsFadeInEnd)
			Opacity -= 0.05f;

		if (Opacity <= 0)
		{
			Opacity = 0;
			IsFadeOutEnd = true;
			IsFadeInEnd = false;
		}
	}
}

void SceneManager::DrawBlackScreen()
{
	JJEngine::GetInstance()->DrawSprite(BlackScreen, 0.0f, 0.0f, Opacity);
}
