
// Pop3Dlg.h : ͷ�ļ�
//

#pragma once
#include "afxcmn.h"
#include "Pop3Manage.h"
#include "resource.h"


// CPop3Dlg �Ի���

	
class CPop3Dlg : public CDialogEx
{
// ����
public:
	CPop3Dlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_POP3_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��

	
// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CString m_Password;
	CString m_UserName;
	CString m_ServerAddress;
	BOOL m_bDelete;
	CString m_InFormation;
	CRichEditCtrl m_RichEditCtrlInformation;
	CPop3Manage Pop3;
	void CPop3Dlg::Dispatch(LONG param);
	
	afx_msg void OnBnClickedConnect();
	afx_msg void OnBnClickedView();
};
