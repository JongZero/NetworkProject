#pragma once
#include "ServerDefine.h"
#include "BaseThread.h"

class IOCPThread;
class GameManager;

/// <summary>
/// 클라를 서버와 연결시킴
/// 2021. 05. 10
/// 정종영
/// </summary>
class ServerManager : public BaseThread
{
private:
	ServerManager();
	virtual ~ServerManager();

private:
	static ServerManager* m_Instance;

private:
	SSocket*				m_psSocket;

	HANDLE                  m_hIOCP;			// Input/Output Completion Port( 입출력 완료 포트, 내부적으로 Queue 를 생성 ) 핸들
	static BOOL             m_bExit;			// 주 쓰레드 종료 플래그

	IOCPThread* m_IOCPThread;		// 워커 쓰레드

public:
	static ServerManager* GetInstance();
	void DeleteInstance();

private:
	virtual void			Begin();
	virtual void			End();
	virtual unsigned int	Run(int dwIndex);

private:
	static void SignalFunction(int iSignalNumber);

public:
	bool Initialize();
	void Release();

	BOOL SendPacket(SHeader* psPacket);
	BOOL Receive(SOverlapped* psOverlapped = nullptr);	// 클라이언트로부터 패킷 수신 오버랩드 걸기
};
