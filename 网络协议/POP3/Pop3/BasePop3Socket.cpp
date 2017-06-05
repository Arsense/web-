// BasePop3Socket.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "Pop3.h"
#include "BasePop3Socket.h"
#include "Pop3Dlg.h"

class CPop3Dlg;

// CBasePop3Socket

CBasePop3Socket::CBasePop3Socket()
{
}

CBasePop3Socket::~CBasePop3Socket()
{
}


// CBasePop3Socket ��Ա����
void CBasePop3Socket::SetMsgWnd(CDialog * pWnd)  //set the pointer
{
	m_pWnd = pWnd;
}



void CBasePop3Socket::OnReceive(int nErrorCode)
{
	// TODO: �ڴ����ר�ô����/����û���
	if(nErrorCode==0)
		((CPop3Dlg*)m_pWnd)->Dispatch(RECEIVE);
	
}


void CBasePop3Socket::OnConnect(int nErrorCode)
{
	// TODO: �ڴ����ר�ô����/����û���
	if(nErrorCode==0)
		((CPop3Dlg*)m_pWnd)->Dispatch(CONNECT);
	CAsyncSocket::OnConnect(nErrorCode);
}


void CBasePop3Socket::OnAccept(int nErrorCode)
{
	// TODO: �ڴ����ר�ô����/����û���
	if(nErrorCode==0) //free of any error, dispatch message
		((CPop3Dlg*)m_pWnd)->Dispatch(ACCEPT);
	CAsyncSocket::OnAccept(nErrorCode);
}


void CBasePop3Socket::OnSend(int nErrorCode)
{
	// TODO: �ڴ����ר�ô����/����û���

	CAsyncSocket::OnSend(nErrorCode);
}
