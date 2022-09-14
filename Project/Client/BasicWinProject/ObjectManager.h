#pragma once
#include "Object.h"

class Object;
class GameObject;
class Button;
class TextBox;

/// <summary>
/// �̱��� ������ �̿��� ���ӿ�����Ʈ ���� �Ŵ���
/// 
/// 2021. 02. 09. Hacgeum
/// </summary>
class ObjectManager
{
public:
	ObjectManager();
	~ObjectManager();

private:
	list<Object*> m_ObjectList;
	list<Object*> m_NotDrawUpdateObjectList;
	/// 1. �̰� Scene�������� �޶�� �Ѵ�.
	/// 2. �̰��� Sort�� ����� �ȴ�. ���İ���(depth)�� ���� ó������ �������� ���� �ְ�,
	/// ���� �߰��� �ٲ� ���� �ִ�.

public:
	void Initialize();
	void Release();

	Object* OnlyCreateObject_Image(JVector pos, std::wstring name, int frame, UISortLayer layer);
	Button* OnlyCreateObject_Button(JVector pos, std::wstring name, JVector colliderSize, UISortLayer layer, eNowSceneState state);
	Button* OnlyCreateObject_Button(JVector pos, std::wstring name, int frame, UISortLayer layer);
	Button* OnlyCreateObject_Button(JVector pos, std::wstring name, int frame, UISortLayer layer, eNowSceneState state);

	// ������Ʈ�� �����ϰ� ����Ʈ�� �ִ´�.
	Object* CreateObject_Image(JVector pos, std::wstring name, int frame, UISortLayer layer);
	Button* CreateObject_Button(JVector pos, std::wstring name, JVector colliderSize, UISortLayer layer, eNowSceneState state);
	Button* CreateObject_Button(JVector pos, std::wstring name, int frame, UISortLayer layer);
	Button* CreateObject_Button(JVector pos, std::wstring name, int frame, UISortLayer layer, eNowSceneState state);

	void UpdateAll(float dTime);
	void DrawAll();

public:
	void CheckButtonClicked(Scene* nowScene);
};