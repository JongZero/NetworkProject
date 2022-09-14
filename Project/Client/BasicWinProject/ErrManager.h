#pragma once

/// <summary>
/// 에러 담당 매니저
/// </summary>
class ErrManager
{
public:
	ErrManager() {};
	~ErrManager() {};

	static void ErrDisplay(const char* const cpcMSG);
};

