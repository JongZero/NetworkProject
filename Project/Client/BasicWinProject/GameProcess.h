#pragma once

class JTimer;

class GameProcess
{
private:
	HWND m_hwnd;

public:
	GameProcess();
	~GameProcess();

public:
	bool Initialize(HINSTANCE hInstance);
	void Release();

	void MessageLoop();
	void UpdateAll();
	void RenderAll();

	void CreateManager();
	void LoadResources();
	void CreateScene();
	void ShowDebug();

private:
	float m_FPS;
	float m_DTime;
	JTimer* m_pTimer;

	SceneManager* m_pSceneManager;

	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
};