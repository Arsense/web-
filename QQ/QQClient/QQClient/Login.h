#pragma once
#include "afxcmn.h"
#include"resource.h"

// CLogin �Ի���

class CLogin : public CDialog
{
	DECLARE_DYNAMIC(CLogin)

public:
	CLogin(CWnd* pParent = NULL);   // ��׼���캯��
	CString m_strIP;
	virtual ~CLogin();

// �Ի�������
	enum { IDD = IDD_DIALOG_LOGIN };
	CIPAddressCtrl	m_ControlIP;   //127.0.0.1
	CString	 m_strName;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
	virtual void OnOK();
	BOOL CLogin::OnInitDialog() ;
	DECLARE_MESSAGE_MAP()
public:	
};
