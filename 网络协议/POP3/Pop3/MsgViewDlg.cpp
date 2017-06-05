// MsgViewDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "Pop3.h"
#include "MsgViewDlg.h"
#include "afxdialogex.h"


// CMsgViewDlg �Ի���

IMPLEMENT_DYNAMIC(CMsgViewDlg, CDialog)

CMsgViewDlg::CMsgViewDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMsgViewDlg::IDD, pParent)
	, text(_T(""))
{

}

CMsgViewDlg::~CMsgViewDlg()
{
}

void CMsgViewDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_MSGTEXT, text);
}


BEGIN_MESSAGE_MAP(CMsgViewDlg, CDialog)
	ON_BN_CLICKED(IDC_SAVE, &CMsgViewDlg::OnBnClickedSave)
END_MESSAGE_MAP()


// CMsgViewDlg ��Ϣ�������


void CMsgViewDlg::OnBnClickedSave()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	CFileDialog dlg(FALSE);
	if(dlg.DoModal()==IDOK)
	{
		CFile file(dlg.GetPathName(),CFile::modeCreate | CFile::modeWrite | CFile::typeBinary);
		char* v1 = NULL;
		memcpy(v1,text,sizeof(text));
		file.Write((LPCSTR)v1,text.GetLength());
		file.Close();
	}
}
