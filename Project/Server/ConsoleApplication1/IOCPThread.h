#pragma once
#include "BaseThread.h"

class ServerManager;
class GameManager;

/// <summary>
/// IOCP ������, Send, Recv ó�� ���
/// 2021. 05. 10
/// ������
/// </summary>
class IOCPThread : public BaseThread
{
public:
	IOCPThread(ServerManager* server, GameManager* game, HANDLE hIOCP);
	virtual ~IOCPThread();

private:
	ServerManager*			m_ServerManager;
	GameManager*			m_GameManager;
	HANDLE                  m_hIOCP;

private:
	virtual void			Begin();
	virtual void			End();
	virtual unsigned int	Run(int dwIndex);

private:
	void ProcessRecv(DWORD dwNumberOfBytesTransferred, SOverlapped* psOverlapped, SSocket* psSocket);
	void ProcessSend(DWORD dwNumberOfBytesTransferred, SOverlapped* psOverlapped, SSocket* psSocket);
};

