
// Pop3Dlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "Pop3.h"
#include "Pop3Dlg.h"
#include "afxdialogex.h"
#include "ChooseDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// �Ի�������
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CPop3Dlg �Ի���




CPop3Dlg::CPop3Dlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CPop3Dlg::IDD, pParent)
	, m_Password(_T("ml1075256556"))
	, m_UserName(_T("17802926324"))
	, m_ServerAddress(_T("pop.163.com"))
	, m_bDelete(FALSE)
	, m_InFormation(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CPop3Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_PASS, m_Password);
	DDX_Text(pDX, IDC_USER, m_UserName);
	DDX_Text(pDX, IDC_SERVER, m_ServerAddress);
	DDX_Check(pDX, IDC_DELETE, m_bDelete);
	DDX_Text(pDX, IDC_INFO, m_InFormation);
	DDX_Control(pDX, IDC_INFO, m_RichEditCtrlInformation);
}

BEGIN_MESSAGE_MAP(CPop3Dlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_CONNECT, &CPop3Dlg::OnBnClickedConnect)
	ON_BN_CLICKED(IDC_VIEW, &CPop3Dlg::OnBnClickedView)
END_MESSAGE_MAP()


// CPop3Dlg ��Ϣ�������

BOOL CPop3Dlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ��������...���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// ���ô˶Ի����ͼ�ꡣ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO: �ڴ���Ӷ���ĳ�ʼ������
	CHARFORMAT cf;                      //�ַ���ʽ��Ľṹ��richedit��ʹ��
	cf.cbSize=sizeof(cf);
	cf.dwMask=CFM_COLOR | CFM_FACE;    //���������������ɫ
	cf.dwEffects=0;
	cf.crTextColor=RGB(255,0,0);
	WCHAR* v1 = L"Verdana";
	memcpy(cf.szFaceName,v1,sizeof(v1));
	CRichEditCtrl* re=(CRichEditCtrl*)GetDlgItem(IDC_INFO);
	re->SetDefaultCharFormat(cf);

	

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

void CPop3Dlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CPop3Dlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CPop3Dlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CPop3Dlg::OnBnClickedConnect()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������

	Pop3.SetMsgWnd(this); //�趨�����Ա��ڽ�����Ϣ
	UpdateData(TRUE);
	m_InFormation = "";
	Pop3.SetProp(m_UserName,m_Password); //�趨�û���������
	Pop3.DelAfterRead(m_bDelete);
	Pop3.Create();
	
	Pop3.Connect(((LPCTSTR)m_ServerAddress),110); //���ӷ�����
	UpdateData(FALSE);
}

void CPop3Dlg::Dispatch(LONG param)
{
	CString cStr;
	switch(param)
	{
	case CONNECT:
		m_InFormation = "Connect to" + m_ServerAddress;
		m_InFormation += "\r\n";
		GetDlgItem(IDC_CONN)->EnableWindow(FALSE);
		GetDlgItem(IDC_DISC)->EnableWindow(TRUE);
		GetDlgItem(IDC_VIEW)->EnableWindow(TRUE);
		break;
	case RECEIVE:
		Pop3.GetLastMsg(cStr);
		m_InFormation+=cStr;
		break; 
	case CLOSE: //�ر�����
		m_InFormation+=Pop3.GetError();
		m_InFormation+="Connection closed\r\n";
		GetDlgItem(IDC_CONN)->EnableWindow(TRUE);
		GetDlgItem(IDC_DISC)->EnableWindow(FALSE);
		break;
	case GETNUMMSGS: //�ʼ�������
		{
			cStr.Format("There are %d messages\r\n",Pop3.GetNumMsg());
			m_InFormation+=cStr;
			break;
		}
	case GETSIZEMSGS:               //�ʼ��Ĵ�С
		{
			cStr.Format("Size is: %d\r\n",Pop3.GetSizeMsg());
			m_InFormation+=cStr;
			break;
		}
	case ENDRETR: //we have received all of messages
		{
			cStr.Format("Received %d messages\r\n",Pop3.GetRetrMsgNum());
			m_InFormation+=cStr;
			if(Pop3.GetRetrMsgNum()>0)
			{
				GetDlgItem(IDC_VIEW)->EnableWindow(TRUE);
			}
			break;
		}
	}
	//SetDlgItemTextA(IDC_INFO,m_InFormation);

	UpdateData(FALSE);
	
}


void CPop3Dlg::OnBnClickedView()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	CChooseDlg dlg;
	dlg.DoModal();
}
