#include "pch.h"

#include "MainGameScene.h"
#include "ObjectManager.h"
#include "Object.h"
#include "ColliderBox.h"
#include "Button.h"


// static 멤버 변수 초기화
bool MainGameScene::IsOncePlay[11] = { false, };
bool MainGameScene::m_IsFirstPlay[13] = { false, };
int MainGameScene::Score[2] = { 0, };
int MainGameScene::PlayerNum = -1;
bool MainGameScene::IsGameWin = false;
bool MainGameScene::IsServerDown = false;
MainGameScene* MainGameScene::m_Instance = nullptr;

MainGameScene* MainGameScene::GetInstance()
{
	if (nullptr == m_Instance)
	{
		m_Instance = new MainGameScene;
	}

	return m_Instance;
}

void MainGameScene::DeleteInstance()
{
	if (m_Instance)
	{
		delete m_Instance;
		m_Instance = nullptr;
	}
}

MainGameScene::MainGameScene()
	: m_NowTurn(-1),
	m_IsGetPlayerNum(false),
	m_IsGameStart(false),
	m_EndMyTurn(false),
	m_NowState(-1),

	m_Background(nullptr),
	m_Loading(nullptr),
	m_Waiting(nullptr),
	m_WaitingBackground(nullptr),
	m_Warning(nullptr),

	m_DanceMan(nullptr),
	m_DanceGirl(nullptr),
	m_Player(nullptr),
	m_GuidePlayer(nullptr),
	m_RelativePlayer(nullptr),
	m_Me(nullptr),
	m_You(nullptr),

	m_Question(nullptr),
	m_Answer(nullptr),
	m_GameEnd(nullptr),
	m_GameEndBackground(nullptr),
	m_Arrow(nullptr),

	m_1Round(nullptr),
	m_2Round(nullptr),
	m_3Round(nullptr),
	m_4Round(nullptr),
	m_LastRound(nullptr),

	m_pPrevScene(nullptr),
	m_RemainingTime(13.5f),
	m_DelayTime(-10.0f),
	m_InputCount(6),
	m_RemainingRound(0),

	m_BGMName(L"BGM_hi_ingame50")
{

}

MainGameScene::~MainGameScene()
{
	for (int i = 0; i < 6; i++)
	{
		m_InputKeys[i] = NULL;
	}

	SoundManager::GetInstance()->Stop(0, m_BGMName.c_str());
}

void MainGameScene::Initialize()
{
	Scene::Initialize();
	InputManager::GetInstance()->Reset();

	// 배경 이미지
	m_Background = m_pObjectManager->OnlyCreateObject_Image(JVector(0, -15), L"Background_Main", 1, UISortLayer::BACKGROUND);
	m_Loading = m_pObjectManager->OnlyCreateObject_Image(JVector(1230, 920), L"Loading", 2, UISortLayer::BACKGROUND);
	m_Waiting = m_pObjectManager->OnlyCreateObject_Image(JVector(-35, -55), L"Waiting", 1, UISortLayer::BACKGROUND);
	m_WaitingBackground = m_pObjectManager->OnlyCreateObject_Image(JVector(0, -15), L"WaitingBackground", 1, UISortLayer::BACKGROUND);

	m_Warning = m_pObjectManager->OnlyCreateObject_Image(JVector(0, -100), L"Warning", 1, UISortLayer::BACKGROUND);

	m_DanceMan = m_pObjectManager->OnlyCreateObject_Image(JVector(300, 100), L"Dance_Man", 5, UISortLayer::BUTTON);
	m_DanceGirl = m_pObjectManager->OnlyCreateObject_Image(JVector(1200, 100), L"Dance_Girl", 5, UISortLayer::BUTTON);

	m_Me = m_pObjectManager->OnlyCreateObject_Image(JVector(50, 5), L"Me", 1, UISortLayer::BUTTON);
	m_You = m_pObjectManager->OnlyCreateObject_Image(JVector(80, -15), L"You", 1, UISortLayer::BUTTON);
	m_GuidePlayer = m_pObjectManager->OnlyCreateObject_Image(JVector(10, -30), L"WaitPlayer", 1, UISortLayer::BUTTON);

	m_Question = m_pObjectManager->OnlyCreateObject_Image(JVector(25, -15), L"Question", 1, UISortLayer::BUTTON);
	m_Answer = m_pObjectManager->OnlyCreateObject_Image(JVector(50, -15), L"Answer", 1, UISortLayer::BUTTON);

	m_GameEnd = m_pObjectManager->OnlyCreateObject_Image(JVector(50, -100), L"GameEnd", 1, UISortLayer::BUTTON);
	m_GameEndBackground = m_pObjectManager->OnlyCreateObject_Image(JVector(0, -15), L"GameEndBackground", 1, UISortLayer::BUTTON);

	m_Arrow = m_pObjectManager->OnlyCreateObject_Image(JVector(-300, -300), L"Arrow", 4, UISortLayer::BUTTON);

	m_1Round = m_pObjectManager->OnlyCreateObject_Image(JVector(700, -10), L"1Round", 1, UISortLayer::BUTTON);
	m_2Round = m_pObjectManager->OnlyCreateObject_Image(JVector(700, -10), L"2Round", 1, UISortLayer::BUTTON);
	m_3Round = m_pObjectManager->OnlyCreateObject_Image(JVector(700, -10), L"3Round", 1, UISortLayer::BUTTON);
	m_4Round = m_pObjectManager->OnlyCreateObject_Image(JVector(700, -10), L"4Round", 1, UISortLayer::BUTTON);
	m_LastRound = m_pObjectManager->OnlyCreateObject_Image(JVector(650, -10), L"LastRound", 1, UISortLayer::BUTTON);

	// 서버와 연결한다.
	ServerManager::GetInstance()->Resume();

	// 서버에서 플레이어 Num을 받아올 때까지 대기
	while (true)
	{
		if (m_IsGetPlayerNum)
			break;
		// 연결 중임을 표시
		else
		{
			JJEngine::GetInstance()->BeginRender();
			m_Background->Draw();
			m_Loading->Draw();
			JJEngine::GetInstance()->DrawText(1920 / 2 - 100, 1080 / 4, 1920, 100, 40, ColorF(ColorF::Black), L"< 게임 방법 >");
			JJEngine::GetInstance()->DrawText(1920 / 2 - 130, 1080 / 4 + 60, 1920, 100, 40, ColorF(ColorF::Black), L" 총 5라운드에요! ");
			JJEngine::GetInstance()->DrawText(1920 / 2 - 400, 1080 / 4 + 120, 1920, 100, 40, ColorF(ColorF::Black), L" 상대방의 춤과 음을 잘 듣고 똑같이 따라하세요! ");
			JJEngine::GetInstance()->DrawText(1920 / 2 - 280, 1080 / 4 + 180, 1920, 100, 40, ColorF(ColorF::Black), L" 키 입력은 총 6번까지 가능해요! ");
			JJEngine::GetInstance()->DrawText(1920 / 2 - 340, 1080 / 4 + 240, 1920, 100, 40, ColorF(ColorF::Black), L" 한 문제 당 10점, 많이 맞춘 사람이 승리! ");

			JJEngine::GetInstance()->DrawText(1920 / 2 - 60, 1080 / 4 + 360, 1920, 100, 40, ColorF(ColorF::Black), L" [ 조작키 ] ");
			JJEngine::GetInstance()->DrawText(1920 / 2 - 60, 1080 / 4 + 420, 1920, 100, 40, ColorF(ColorF::Black), L" A, S, D, F ");
			JJEngine::GetInstance()->DrawText(1920 / 2 - 80, 1080 / 4 + 480, 1920, 100, 40, ColorF(ColorF::Black), L" 도 레 미 파 ");
			JJEngine::GetInstance()->EndRender();
		}
	}

	if (PlayerNum == 0)
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

	// 사운드
	SoundManager::GetInstance()->Play(0, m_BGMName.c_str());		// BGM사운드

	// 버튼
	//m_pPrevScene = m_pObjectManager->CreateObject_Button(JVector(893, 35), L"Dissecting_ReturnButton", 2, UISortLayer::BUTTON, eNowSceneState::GO_TO_PREV_SCENE);
}

void MainGameScene::Update(float dTime)
{
	m_Arrow->UpdateAnimation(dTime * 0.8f);

	// 딜레이 타임
	m_DelayTime -= dTime;
	if (m_DelayTime > 0)
		return;

	if (m_IsGameEnd || IsServerDown)
		m_NowSceneState = eNowSceneState::GO_TO_NEXT_SCENE;

	switch (m_NowSceneState)
	{
		case eNowSceneState::NONE:
			break;

		case eNowSceneState::GO_TO_NEXT_SCENE:
		{
			SceneManager::ChangeScene(eSceneStateAll::OUTRO_RESULT);
			return;
		}
		break;
	}

	m_pObjectManager->UpdateAll(dTime);

	m_pObjectManager->CheckButtonClicked(this);

	if (m_NowAttackPlayer == PlayerNum)
		m_NowAttackDefenceState = STATE_ATTACK;
	else
		m_NowAttackDefenceState = STATE_DEFENCE;

	if (m_NowTurn == PlayerNum)
		m_NowState = STATE_MY_TURN;
	else
		m_NowState = STATE_RELATIVE_PLAYER_TURN;

	// 내 턴일 경우만 키 인풋을 받음
	if (m_NowState == STATE_MY_TURN)
	{
		InputKey();

		// 내 턴일 경우 제한 시간 줄어든다.
		m_RemainingTime -= dTime;
	}
	else
	{
		m_RemainingTime = 10.0f;
	}

	// 제한 시간이 다 되거나, 키보드를 6번 입력하면 턴이 끝난다.
	if (m_RemainingTime <= 0.0f || m_InputCount <= 0)
	{
		m_RemainingTime = 10.0f;
		m_InputCount = 6;
		EndTurn();
		m_EndMyTurn = true;
	}

	if (m_NowState == STATE_MY_TURN)
	{
		m_Arrow->Transform()->SetPos(440, -20);

		if (PlayerNum == 0)
		{
			m_Arrow->Transform()->SetPos(430, -20);
		}
	}
	else
	{
		m_Arrow->Transform()->SetPos(1400, -20);
	}

	// 이전 씬으로 가는 버튼 마우스 상호작용 애니메이션
// 	if (m_pPrevScene->GetColliderBox()->IsCollided)
// 	{
// 		if (m_IsFirstPlay[0] == false)
// 		{
// 			SoundManager::GetInstance()->Play(1, L"Button_Over_Sample_01");
// 			m_IsFirstPlay[0] = true;
// 		}
// 		m_pPrevScene->ChangeFrame(1);
// 	}
// 	else
// 	{
// 		m_IsFirstPlay[0] = false;
// 		m_pPrevScene->ChangeFrame(0);
// 	}

	POINT _mousePos = InputManager::GetInstance()->GetMousePos();
}

void MainGameScene::EndTurn()
{
	C2S_TurnEnd packet;
	packet.bIsTurnEnd = true;

	for (int i = 0; i < 6; i++)
	{
		packet.inputKeys[i] = m_InputKeys[i];
		m_InputKeys[i] = NULL;
	}

	if (m_NowAttackDefenceState == STATE_ATTACK)
		packet.bIsAttack = true;
	else
		packet.bIsAttack = false;

	ServerManager::GetInstance()->SendPacket(&packet);
}

void MainGameScene::Draw()
{
	m_Background->Draw();
	m_Player->Draw();
	m_Me->Draw();

	if (m_IsGameStart == true)
	{
		m_RelativePlayer->Draw();
		m_You->Draw();
		m_Arrow->Draw();

		// 남은 라운드 표시
		switch (m_RemainingRound)
		{
			case 5:
				m_1Round->Draw();
				break;
			case 4:
				m_2Round->Draw();
				break;
			case 3:
				m_3Round->Draw();
				break;
			case 2:
				m_4Round->Draw();
				break;
			case 1:
				m_LastRound->Draw();
				break;
		}

		//if (m_RemainingRound <= 1)
		//	JJEngine::GetInstance()->DrawText(1920 / 2.5 + 40, 0, 40, L"마지막 라운드", m_RemainingRound);
		//else
		//	JJEngine::GetInstance()->DrawText(1920 / 2.5 + 40, 0, 40, L"남은 라운드 : %d", m_RemainingRound);
	}
	else
	{
		m_WaitingBackground->Draw(0.8f);
		m_GuidePlayer->Draw();
		m_Waiting->Draw();
	}

	// 게임 종료
	if (m_IsGameEnd || IsServerDown)
	{
		//JJEngine::GetInstance()->DrawText(650, 160, 100, L"게임끝!!!!!!!", m_DelayTime);
		m_GameEndBackground->Draw(0.8f);
		m_GameEnd->Draw();
	}
	else
	{
		// 딜레이 시간 동안 누구 턴인지, 누가 공격인지 보여준다.
		if (m_DelayTime > 0)
		{

		}
		else if (m_NowTurn == PlayerNum)
		{
			JJEngine::GetInstance()->DrawText(1920 / 2.5 + 40, 1080 - 300, 40, L"남은 시간 : %.1f", m_RemainingTime);
			JJEngine::GetInstance()->DrawText(1920 / 2.5, 1080 - 250, 40, L"남은 입력 횟수 : %d", m_InputCount);
			if (m_NowAttackDefenceState == STATE_ATTACK)
			{
				//JJEngine::GetInstance()->DrawText(650, 160, 100, L"문제를 내세요!");
				m_Question->Draw();
			}
			else
			{
				//JJEngine::GetInstance()->DrawText(650, 160, 100, L"문제를 맞추세요!");
				m_Answer->Draw();
			}
		}
		else
		{
			if (m_NowAttackDefenceState == STATE_DEFENCE)
			{
				m_Warning->Draw();
			}
		}
	}

	// 임시로 딜레이 출력
	//JJEngine::GetInstance()->DrawText(0, 160, L"딜레이 (0되면 풀림) : %f", m_DelayTime);

	// 스코어 표시
	ShowScore();

	//ShowDebug();
}

void MainGameScene::ShowDebug()
{
	Scene::ShowDebug();

	JJEngine::GetInstance()->DrawText(0, 40, L"Main_Game 씬");
}

void MainGameScene::Release()
{
	Scene::Release();
}

void MainGameScene::InputKey()
{
	/// 춤추는 남자/여자
	if (GetAsyncKeyState('A') & 0x8000 && m_InputCount > 0 && !m_EndMyTurn)
	{
		m_Player->ChangeFrame(1);

		if (m_IsFirstPlay[0] == false)
		{
			SoundManager::GetInstance()->Play(1, L"do");

			// 키 입력정보를 서버에 보낸다.
			C2S_KeyInput packet;
			packet.inputKey = 'A';

			ServerManager::GetInstance()->SendPacket(&packet);

			// 입력한 키 추가
			AddKey('A');

			m_IsFirstPlay[0] = true;

			m_InputCount--;
		}
	}
	else if (GetAsyncKeyState('S') & 0x8000 && m_InputCount > 0 && !m_EndMyTurn)
	{
		m_Player->ChangeFrame(2);

		if (m_IsFirstPlay[0] == false)
		{
			SoundManager::GetInstance()->Play(1, L"re");

			// 키 입력정보를 서버에 보낸다.
			C2S_KeyInput packet;
			packet.inputKey = 'S';

			ServerManager::GetInstance()->SendPacket(&packet);

			// 입력한 키 추가
			AddKey('S');

			m_IsFirstPlay[0] = true;

			m_InputCount--;
		}
	}
	else if (GetAsyncKeyState('D') & 0x8000 && m_InputCount > 0 && !m_EndMyTurn)
	{
		m_Player->ChangeFrame(3);

		if (m_IsFirstPlay[0] == false)
		{
			SoundManager::GetInstance()->Play(1, L"mi");

			// 키 입력정보를 서버에 보낸다.
			C2S_KeyInput packet;
			packet.inputKey = 'D';

			ServerManager::GetInstance()->SendPacket(&packet);

			// 입력한 키 추가
			AddKey('D');

			m_IsFirstPlay[0] = true;

			m_InputCount--;
		}
	}
	else if (GetAsyncKeyState('F') & 0x8000 && m_InputCount > 0 && !m_EndMyTurn)
	{
		m_Player->ChangeFrame(4);

		if (m_IsFirstPlay[0] == false)
		{
			SoundManager::GetInstance()->Play(1, L"pa");

			// 키 입력정보를 서버에 보낸다.
			C2S_KeyInput packet;
			packet.inputKey = 'F';

			ServerManager::GetInstance()->SendPacket(&packet);

			// 입력한 키 추가
			AddKey('F');

			m_IsFirstPlay[0] = true;

			m_InputCount--;
		}
	}
	else
	{
		m_Player->ChangeFrame(0);
		m_IsFirstPlay[0] = false;
	}
}

void MainGameScene::ResetRelativePlayerFrame()
{
	if (m_RelativePlayer != nullptr)
	{
		m_RelativePlayer->ChangeFrame(0);
	}
}

void MainGameScene::RelativePlayerAction(char c)
{
	/// 자신의 턴이 시작할 때 ChangeFrame(0)으로 돌려놔야될듯
	/// 춤추는 남자/여자
	if (c == 'A')
	{
		m_RelativePlayer->ChangeFrame(1);

		SoundManager::GetInstance()->Play(1, L"do");
	}
	else if (c == 'S')
	{
		m_RelativePlayer->ChangeFrame(2);

		SoundManager::GetInstance()->Play(1, L"re");
	}
	else if (c == 'D')
	{
		m_RelativePlayer->ChangeFrame(3);

		SoundManager::GetInstance()->Play(1, L"mi");
	}
	else if (c == 'F')
	{
		m_RelativePlayer->ChangeFrame(4);

		SoundManager::GetInstance()->Play(1, L"pa");
	}
	else
	{
		m_RelativePlayer->ChangeFrame(0);
	}
}

void MainGameScene::AddKey(char c)
{
	for (int i = 0; i < 6; i++)
	{
		if (m_InputKeys[i] == NULL)
		{
			m_InputKeys[i] = c;
			break;
		}
	}
}

void MainGameScene::ShowScore()
{
	JJEngine::GetInstance()->DrawText(430, 800, 110, ColorF(ColorF::Chocolate), L"%d", Score[PlayerNum]);

	// 상대 플레이어의 번호
	int relativePlayerNum = -1;
	if (PlayerNum == PLAYER1)
		relativePlayerNum = PLAYER2;
	else
		relativePlayerNum = PLAYER1;

	JJEngine::GetInstance()->DrawText(1400, 800, 110, ColorF(ColorF::LimeGreen), L"%d", Score[relativePlayerNum]);
}
