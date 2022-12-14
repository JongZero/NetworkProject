// pch.h: 미리 컴파일된 헤더 파일입니다.
// 아래 나열된 파일은 한 번만 컴파일되었으며, 향후 빌드에 대한 빌드 성능을 향상합니다.
// 코드 컴파일 및 여러 코드 검색 기능을 포함하여 IntelliSense 성능에도 영향을 미칩니다.
// 그러나 여기에 나열된 파일은 빌드 간 업데이트되는 경우 모두 다시 컴파일됩니다.
// 여기에 자주 업데이트할 파일을 추가하지 마세요. 그러면 성능이 저하됩니다.

#ifndef PCH_H
#define PCH_H

// 최신 VC++ 컴파일러에서 경고 및 오류 방지
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#define _AFXDLL
#include <afx.h>
#include <afxtempl.h>
#include <iostream>

// 여기에 미리 컴파일하려는 헤더 추가
#include "framework.h"
#include "resource.h"

#include "fmod.h"
#pragma comment(lib, "fmod_vc.lib")
#pragma comment(lib, "fmodL_vc.lib")

#include <list>
#include <map>
#include <vector>

#include "JJEngine_D2D.h"
#include "JTimer.h"
#include "JSprite.h"
#include "JMotion.h"
#include "JScript.h"
#include "JTransform.h"
#include "InputManager.h"
#include "SoundManager.h"
#include "SceneManager.h"
#include "ResourceManager.h"
#include "MainGameScene.h"
#include "ServerManager.h"

#include "Debug.h"

using namespace std;
#endif //PCH_H