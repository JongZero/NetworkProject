#include "pch.h"

#include "SceneManager.h"
#include "IntroTitleScene.h"
#include "MainGameScene.h"
#include "OutroResultScene.h"

// static 멤버 변수 초기화
eSceneStateAll SceneManager::SceneState = eSceneStateAll::NONE;				// 게임 첫 시작 State는 Intro로 설정
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
	// FadeInFadeOut용 스크린 생성
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
	// 씬 전환
	if (SceneState != nextScene)
	{
		IsFadeOutEnd = false;

		// 기존 씬 삭제(Safe Delete)
		if (pNowScene)
		{
			pNowScene->Release();
			delete pNowScene;

			// 삭제 후 nullptr로 초기화
			pNowScene = nullptr;
		}

		switch (nextScene)
		{
			case eSceneStateAll::INTRO_TITLE:
			{
				// 인트로 타이틀 실행
				pNowScene = new IntroTitleScene();
				pNowScene->Initialize();
				SceneState = nextScene;
			}
			break;

			case eSceneStateAll::MAIN_GAME:
			{
				// 메인 사무실 실행
				pNowScene = MainGameScene::GetInstance();
				pNowScene->Initialize();
				SceneState = nextScene;
			}
			break;

			case eSceneStateAll::OUTRO_RESULT:
			{
				// 아웃트로 결과 실행
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
