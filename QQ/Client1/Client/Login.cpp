// Login.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "Client.h"
#include "Login.h"
#include "afxdialogex.h"


// CLogin �Ի���



IMPLEMENT_DYNAMIC(CLogin, CDialog)

CLogin::CLogin(CWnd* pParent /*=NULL*/)
	: CDialog(CLogin::IDD, pParent)
{
	m_strName = _T("");
}



void CLogin::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_IPADDRESS, m_ControlIP);
	DDX_Text(pDX, IDC_EDIT, m_strName);
}


BEGIN_MESSAGE_MAP(CLogin, CDialog)
END_MESSAGE_MAP()


// CLogin ��Ϣ�������

BOOL CLogin::OnInitDialog() 
{
	CDialog::OnInitDialog();

	WSADATA wsaData;
	WSAStartup(MAKEWORD(2,2),&wsaData);



	ULONG   uIp   =   inet_addr("127.0.0.1");
	m_ControlIP.SetAddress(htonl(uIp));  //IP �ؼ���������



	BYTE f1,f2,f3,f4;
	m_ControlIP.GetAddress(f1,f2,f3,f4);
	m_strIP.Format("%d.%d.%d.%d",f1,f2,f3,f4);

	UpdateData(false);



	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}
