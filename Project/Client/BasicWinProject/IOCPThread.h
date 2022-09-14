#pragma once
#include "BaseThread.h"

class ServerManager;

/// <summary>
/// IOCP ������, Send, Recv ó�� ���
/// 2021. 05. 10
/// ������
/// </summary>
class IOCPThread : public BaseThread
{
public:
	IOCPThread(ServerManager* server, HANDLE hIOCP);
	virtual ~IOCPThread();

private:
	ServerManager* m_ServerManager;
	HANDLE                  m_hIOCP;

private:
	virtual void			Begin();
	virtual void			End();
	virtual unsigned int	Run(int dwIndex);

private:
	void ProcessRecv(DWORD dwNumberOfBytesTransferred, SOverlapped* psOverlapped, SSocket* psSocket);
	void ProcessSend(DWORD dwNumberOfBytesTransferred, SOverlapped* psOverlapped, SSocket* psSocket);
};

