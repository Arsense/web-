

#pragma once


// CLogin �Ի���

class CLogin : public CDialog
{
	DECLARE_DYNAMIC(CLogin)

public:
	CLogin(CWnd* pParent = NULL);   // ��׼���캯��
	CString m_strIP;


// �Ի�������
	enum { IDD = IDD_DIALOG_LOGIN };

	CIPAddressCtrl	m_ControlIP;
	CString	m_strName;
	
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
};

