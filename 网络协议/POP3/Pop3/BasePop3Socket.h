#pragma once
// CBasePop3Socket ����Ŀ��


#define ACCEPT 0 
#define CLOSE 1
#define CONNECT 2
#define RECEIVE 3
#define SEND 4
class CBasePop3Socket : public CAsyncSocket
{
public:
	CBasePop3Socket();
	virtual ~CBasePop3Socket();
	//�趨������Ϣ����
	void SetMsgWnd(CDialog * pWnd);
	CDialog * m_pWnd; //ָ�򸸴���

	


	
	virtual void OnReceive(int nErrorCode);
	virtual void OnConnect(int nErrorCode);
	virtual void OnAccept(int nErrorCode);
	virtual void OnSend(int nErrorCode);
};


