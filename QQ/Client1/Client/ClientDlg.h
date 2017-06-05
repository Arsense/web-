
// ClientDlg.h : ͷ�ļ�
//

#pragma once
#include "afxwin.h"
#include "afxcmn.h"
#include"MyEdit.h"
#include"MyTracker.h"
// CClientDlg �Ի���
class CClientDlg : public CDialog
{
// ����
public:

				int m_xScreen;
				int m_yScreen;

		     CMyTracker			 m_rectTracker;     //��Ƥ����
		     HCURSOR				 m_hCursor;              //���
		     BOOL                      m_bDraw;                   //�Ƿ�Ϊ��ȡ״̬
		     BOOL                      m_bFirstDraw;              //�Ƿ�Ϊ�״ν�ȡ
		     BOOL                      m_bQuit;                   //�Ƿ�Ϊ�˳�
		     BOOL                      m_bShowMsg;                //��ʾ��ȡ���δ�С��Ϣ
		 	CPoint			          	m_startPt;				//��ȡ�������Ͻ�
			CBitmap *                m_pBitmap;            //����λͼ
			CRgn m_rgn;						//������������


			void DrawTip();                            //��ʾ������ʾ��Ϣ
			HBITMAP CopyScreenToBitmap(LPRECT lpRect,BOOL bSave=FALSE);   //�������浽λͼ
			void DrawMessage(CRect &inRect,CDC * pDC);       //��ʾ��ȡ������Ϣ


	CString m_strOtherIP;
		DWORD   m_dwFileSize;
		CString m_strFileName;
		CString m_strOtherName;
	DWORD   m_dwOtherFileSize;
		CString CClientDlg::KindChange(DWORD dwFileSize);
	CString m_strOtherFileName;
	CClientDlg(CWnd* pParent = NULL);	// ��׼���캯��
	SOCKET  m_ClientSocket;
	SOCKADDR_IN  m_ServerAddr;
		void CClientDlg::AnalyseString(char* szBuffer);
	VOID CClientDlg::SendMsg(int iType,CString strName,CString strMsg);
	VOID CClientDlg::OnRecive(WPARAM wParam);
	void CClientDlg::HideItem();
	void CClientDlg::Shaking();
		CString  m_strName;
			CString  m_Timer;
// �Ի�������
	enum { IDD = IDD_CLIENT_DIALOG };
//	CComboBox	m_ComBoUser;
//	CListBox	m_ListUser;
//	CString		m_EditShow;
	//CString	m_EditSend;
	//BOOL	m_bOnly;
	//BOOL	m_bMe;
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
	afx_msg LRESULT OnSocketRead(WPARAM wParam,LPARAM lParam);
	DECLARE_MESSAGE_MAP()
public:
	CComboBox m_ComBoUser;
	afx_msg void OnBnClickedButtonRecv();
	afx_msg void OnButtonShake();
	afx_msg void OnButtonTrans();
	CListBox m_ListUser;
//	CEdit m_EditShow;
	CString m_EditShow;
	afx_msg void OnCbnSelchangeComboUser();
	BOOL m_bOnly;
	BOOL m_bMe;
	afx_msg void OnEnChangeEditSend();
	CString m_EditSend;
	afx_msg void OnButtonSend();
	CButton m_ButtonCancle;
	CProgressCtrl m_Progress;
	CButton m_ButtonRecv;
	afx_msg void OnButtonRecv();
	CMyEdit m_tipEdit;
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
};
