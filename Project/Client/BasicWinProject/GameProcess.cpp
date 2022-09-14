#include "pch.h"
#include "GameProcess.h"
#include "JTimer.h"
#include "ServerManager.h"

GameProcess::GameProcess()
	:m_hwnd(nullptr),
	m_FPS(0.0f), m_DTime(0.0f),
	m_pTimer(nullptr),
	m_pSceneManager(nullptr)
{

}

GameProcess::~GameProcess()
{

}

bool GameProcess::Initialize(HINSTANCE hInstance)
{
	/// ������ ����Ѵ�.
	WNDCLASSEXW wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = GameProcess::WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = NULL;
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = _T("�� ���� �غ��� �����~");
	wcex.hIconSm = NULL;

	RegisterClassExW(&wcex);

	// ���ø����̼� �ʱ�ȭ�� �����մϴ�:
	m_hwnd = CreateWindowW(_T("�� ���� �غ��� �����~"), _T("�� ���� �غ��� �����~"), WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, 1920, 1080, nullptr, nullptr, hInstance, nullptr);

	if (!m_hwnd)
	{
		return false;
	}

	ShowWindow(m_hwnd, SW_SHOWNORMAL);
	UpdateWindow(m_hwnd);

	// �Ŵ��� Ŭ���� ����
	CreateManager();

	// ���ҽ��� �ε�
	LoadResources();

	// ���� ����
	CreateScene();

	// ������ �����ϱ����� �Ŵ��� ����
	ServerManager::GetInstance()->Create();

	m_pTimer->Initialize();

	return true;
}

void GameProcess::Release()
{
	JJEngine::GetInstance()->Release();

	if (m_pTimer != nullptr)
	{
		m_pTimer->Release();
		delete m_pTimer;
		m_pTimer = nullptr;
	}

	InputManager::GetInstance()->DeleteInstance();
	SoundManager::GetInstance()->Release();

	if (m_pSceneManager != nullptr)
	{
		m_pSceneManager->Release();
		delete m_pSceneManager;
		m_pSceneManager = nullptr;
	}

	ServerManager::GetInstance()->Destroy();
}

void GameProcess::MessageLoop()
{
	MSG msg;

	// �⺻ �޽��� �����Դϴ�
	while (true)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);

			if (msg.message == WM_QUIT)
			{
				break;
			}
		}
		else
		{
			UpdateAll();
			RenderAll();
		}
	}
}

void GameProcess::UpdateAll()
{
	m_FPS = m_pTimer->GetFPS();
	m_DTime = m_pTimer->GetDeltaTime();

	m_pTimer->Update();

	InputManager::GetInstance()->Update();
	SoundManager::GetInstance()->Update();

	m_pSceneManager->Update(m_DTime);
}

void GameProcess::RenderAll()
{
	JJEngine::GetInstance()->BeginRender();
	m_pSceneManager->Render();
	//ShowDebug();

	JJEngine::GetInstance()->EndRender();
}

void GameProcess::CreateManager()
{
	// ���� ����
	JJEngine::GetInstance()->Initialize(m_hwnd);

	// Ÿ�̸� ����
	m_pTimer = new JTimer();
	m_pTimer->Initialize();

	// InputManager �ʱ�ȭ
	InputManager::GetInstance()->Initialize(m_hwnd);

	// SoundManager �ʱ�ȭ
	SoundManager::GetInstance()->Initialize(5);
}

void GameProcess::LoadResources()
{
	// ���ҽ� �̹������� �ε��Ѵ�.
	ResourceManager::GetInstance()->LoadImageFile(_T("../../../Resources/Image"));

	// ���� ���ҽ����� �ε��Ѵ�.
	SoundManager::GetInstance()->LoadSoundFile(_T("../../../Resources/Sound"));
}

void GameProcess::CreateScene()
{
	// SceneManager ����
	m_pSceneManager = new SceneManager;
	m_pSceneManager->Initialize();

	// �� �Ŵ����� ���¸� INTRO�� �ʱ�ȭ
	m_pSceneManager->ChangeScene(eSceneStateAll::INTRO_TITLE);
}

void GameProcess::ShowDebug()
{
	JJEngine::GetInstance()->DrawText(0, 0, L"FPS : %.0f", m_FPS);
	JJEngine::GetInstance()->DrawText(0, 20, L"DeltaTime : %f", m_DTime);
	JJEngine::GetInstance()->DrawText(300, 0, L"MousePosX : %d", InputManager::GetInstance()->GetMousePos().x);
	JJEngine::GetInstance()->DrawText(300, 20, L"MousePosY : %d", InputManager::GetInstance()->GetMousePos().y);
}

LRESULT CALLBACK GameProcess::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}