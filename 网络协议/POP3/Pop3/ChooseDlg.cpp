// ChooseDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "Pop3.h"
#include "ChooseDlg.h"
#include "afxdialogex.h"
#include "Pop3Dlg.h"
#include "MsgViewDlg.h"


// CChooseDlg �Ի���

IMPLEMENT_DYNAMIC(CChooseDlg, CDialogEx)

CChooseDlg::CChooseDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CChooseDlg::IDD, pParent)
{

}

CChooseDlg::~CChooseDlg()
{
}

void CChooseDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_MSGLIST, m_List);
}


BEGIN_MESSAGE_MAP(CChooseDlg, CDialogEx)
	ON_BN_CLICKED(IDCANCEL, &CChooseDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDOK, &CChooseDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CChooseDlg ��Ϣ�������


BOOL CChooseDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  �ڴ���Ӷ���ĳ�ʼ��
	CPop3Dlg* par;
	par=(CPop3Dlg*)GetParent(); //��ø�����
	for(int i=0;i<par->Pop3.GetRetrMsgNum();i++)
	{
		AddToList(par->Pop3.GetMsgSubject(i));
	}
	m_List.SetCurSel(0);

	return TRUE;  // return TRUE unless you set the focus to a control
	// �쳣: OCX ����ҳӦ���� FALSE
}
void CChooseDlg::AddToList(CString s)
{
	
	m_List.AddString((LPCTSTR)s);
}


void CChooseDlg::OnBnClickedCancel()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	
	CDialogEx::OnCancel();
}



void CChooseDlg::OnBnClickedOk()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	CPop3Dlg* par;
	par=(CPop3Dlg*)GetParent();
	CMsgViewDlg dlg; 
	//����ż��Ļ��������Լ��ż�����
	dlg.text=par->Pop3.GetMsgStuff(m_List.GetCurSel());
	dlg.text+=par->Pop3.GetMsgBody(0);
	dlg.DoModal();
	CDialogEx::OnOK();
}
