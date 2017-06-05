
// QQServerDlg.h : ͷ�ļ�
//

#pragma once
#include "resource.h"
#include<afxwin.h>
#include<winsock2.h>
#pragma comment(lib,"ws2_32.lib")


#define MAX 20
// CQQServerDlg �Ի���
class CQQServerDlg : public CDialogEx
{
// ����
public:
	CQQServerDlg(CWnd* pParent = NULL);	// ��׼���캯��
	VOID CQQServerDlg::InitSocket();
	VOID CQQServerDlg::GetHostInfor();
	VOID CQQServerDlg::InitNotify();
// �Ի�������
	enum { IDD = IDD_QQSERVER_DIALOG };
	CStatic	m_strPeople;
	CStatic	m_strHost;
	CString	m_strShow;
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��
		SOCKET   m_sListen;
		SOCKET   m_sClient[MAX];
		sockaddr_in  m_ServerAddr;
		ULONG    m_People;


// ʵ��
protected:
	HICON m_hIcon;

	 NOTIFYICONDATAA m_nid;    //ϵͳ����

	

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
};
