#pragma once
#include "afxwin.h"


// CChooseDlg �Ի���

class CChooseDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CChooseDlg)

public:
	CChooseDlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CChooseDlg();
	void CChooseDlg::AddToList(CString s);

// �Ի�������
	enum { IDD = IDD_MSGCHOOSE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	CComboBox m_List;
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedOk();
};
