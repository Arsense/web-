#pragma once


// CMsgViewDlg �Ի���

class CMsgViewDlg : public CDialog
{
	DECLARE_DYNAMIC(CMsgViewDlg)

public:
	CMsgViewDlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CMsgViewDlg();

// �Ի�������
	enum { IDD = IDD_MSGVIEW };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
public:
	CString text;
	afx_msg void OnBnClickedSave();
};
