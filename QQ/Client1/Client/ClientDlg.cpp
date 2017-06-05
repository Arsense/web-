
// ClientDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "Client.h"
#include "ClientDlg.h"
#include "afxdialogex.h"
#include "Login.h"

#include <Mmsystem.h>

#pragma comment(lib,"Winmm.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define FILEMAX  17520
// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

#define  WM_SOCKET_READ  WM_USER+2
static DWORD WINAPI RecvFileThread(LPVOID lPParam);
static DWORD WINAPI SendFileThread(LPVOID lPParam);
typedef struct _SENDINFOR_    
{
	int  iType;       //0
	char szName[20];  //123
	char szMsg[400];  //���� 
}SENDINFOR;

class CAboutDlg : public CDialog
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

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CClientDlg �Ի���




CClientDlg::CClientDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CClientDlg::IDD, pParent)
	, m_EditShow(_T(""))
	, m_bOnly(FALSE)
	, m_bMe(FALSE)
	, m_EditSend(_T(""))
{
	//m_EditShow = _T("");
	//m_EditSend = _T("");

	//m_bMe = FALSE;
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	//m_EditShow = _T("");
		m_dwFileSize = 0;
		//*******************************************************************************
    //��ʼ����Ƥ����,������resizeMiddle ����
	m_rectTracker.m_nStyle=CMyTracker::resizeMiddle|CMyTracker::solidLine;  
	m_rectTracker.m_rect.SetRect(-1,-2,-3,-4);
	//���þ�����ɫ
	m_rectTracker.SetRectColor(RGB(10,100,130));
	//���þ��ε���ʱ���,Ĭ�ϵ�̫С��,Ū�˸�����
	m_rectTracker.SetResizeCursor(IDC_CURSOR6,IDC_CURSOR5,IDC_CURSOR2,IDC_CURSOR3,IDC_CURSOR4);

    
    
	//һϵ�е����ݳ�ʼ��
	m_bDraw=FALSE;
	m_bFirstDraw=FALSE;
	m_bQuit=FALSE;
	m_bShowMsg=FALSE;
    m_startPt=0;
    
	//��ȡ��Ļ�ֱ���
	m_xScreen = GetSystemMetrics(SM_CXSCREEN);//����ʾ�������Ļ��棬������Ϊ��λ�Ŀ�ȡ�����ͨ�����û�õ�ֵ��ͬ GetDeviceCaps���£�GetDeviceCaps��hdcPrimaryMonitor��HORZRES
	m_yScreen = GetSystemMetrics(SM_CYSCREEN);//����ʾ�������Ļ��棬������Ϊ��λ�ĸ߶ȡ�����ͨ�����û�õ�ֵ��ͬ GetDeviceCaps���£�GetDeviceCaps��hdcPrimaryMonitor��VERTRES�� ��

	//��ȡ��Ļ��λͼ��
	CRect rect(0, 0,m_xScreen,m_yScreen);
	m_pBitmap=CBitmap::FromHandle(CopyScreenToBitmap(&rect));
    
	//��ʼ��ˢ�´������� m_rgn
    m_rgn.CreateRectRgn(0,0,50,50);
	m_hCursor=AfxGetApp()->LoadCursor(IDC_CURSOR1);  
//*******************************************************************************



}

void CClientDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_USER, m_ComBoUser);
	DDX_Control(pDX, IDC_LIST_USER, m_ListUser);
	DDX_Text(pDX, IDC_EDIT_SHOW, m_EditShow);
	DDX_Check(pDX, IDC_CHECK_ONLY, m_bOnly);
	DDX_Check(pDX, IDC_CHECK_ME, m_bMe);
	DDX_Text(pDX, IDC_EDIT_SEND, m_EditSend);
	DDX_Control(pDX, IDC_BUTTON_END, m_ButtonCancle);
	DDX_Control(pDX, IDC_PROGRESS, m_Progress);
	DDX_Control(pDX, IDC_BUTTON_RECV, m_ButtonRecv);
	DDX_Control(pDX, IDC_CATCHSCREEN, m_tipEdit);
}

BEGIN_MESSAGE_MAP(CClientDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()

	ON_MESSAGE(WM_SOCKET_READ,OnSocketRead)
	ON_BN_CLICKED(IDC_BUTTON_SHAKE, &CClientDlg::OnButtonShake)
	ON_BN_CLICKED(IDC_BUTTON_TRANS, &CClientDlg::OnButtonTrans)
	ON_CBN_SELCHANGE(IDC_COMBO_USER, &CClientDlg::OnCbnSelchangeComboUser)
	ON_EN_CHANGE(IDC_EDIT_SEND, &CClientDlg::OnEnChangeEditSend)
	ON_BN_CLICKED(IDC_BUTTON_SEND, &CClientDlg::OnButtonSend)
	ON_BN_CLICKED(IDC_BUTTON_RECV, &CClientDlg::OnButtonRecv)
	ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()


// CClientDlg ��Ϣ�������

BOOL CClientDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	//**************************************************************************
	//�ѶԻ������ó�ȫ�����㴰��
	SetWindowPos(&wndTopMost,0,0,m_xScreen,m_yScreen,SWP_SHOWWINDOW);
    
	//�ƶ�������ʾ����
	CRect rect;
	m_tipEdit.GetWindowRect(&rect);
    m_tipEdit.MoveWindow(10,10,rect.Width(),rect.Height());

	//��ʾ������ʾ��������
	DrawTip();
	
	//���񰴼���Ϣ����,���Ի���ľ�����ݵ�CCatchScreenApp��
	((CClientApp *)AfxGetApp())->m_hwndDlg=m_hWnd;
//**************************************************************************
	CLogin  LoginDlg;           //������½�Ի���

	if (LoginDlg.DoModal()==IDOK)     
	{

		//LoginDlg ���ServerIP ����

		CString strName;
		CString strIP;



		strName = LoginDlg.m_strName;        //�ѶԻ����ϵ����ָ�ֵ��strName

		ULONG ulServerIP;                //127.0.0.1
		ulServerIP = inet_addr(LoginDlg.m_strIP);//�ַ���IPת32λIP



		m_ClientSocket = socket(PF_INET,SOCK_STREAM,0);    //ͨ���׽���
		if(m_ClientSocket==INVALID_SOCKET)          //û�д����ɹ�
		{
			AfxMessageBox("Create SOCKET Failed");
			WSACleanup(); 
			closesocket(m_ClientSocket);
			CClientDlg::OnCancel();
		}


		//��ʼ��������Ϣ
		m_ServerAddr.sin_family = AF_INET;
		m_ServerAddr.sin_addr.s_addr = ulServerIP;
		m_ServerAddr.sin_port = htons(9527);//short �����ֽ���


		//����
		int bOk =
			connect(m_ClientSocket,(sockaddr*)&m_ServerAddr,sizeof(m_ServerAddr));      

		if(bOk==SOCKET_ERROR)           //����ʧ��
		{
			MessageBox("���ӹ��̷�������\n��ȷ��IP������ȷ����",NULL,MB_OK);
			CDialog::OnCancel();
		}
		else
		{
			SendMsg(0,strName,"");   //0 ���ݵ�����  ����������͵�һ����Ϣ ����
		}
		WSAAsyncSelect(m_ClientSocket,m_hWnd,WM_SOCKET_READ,FD_READ|FD_CLOSE);
		//ע��һ���Զ�����Ϣ����ӷ������������ݰ�����ͷ������ر�


		PlaySound(MAKEINTRESOURCE(IDR_WAVE),                //��������
			AfxGetResourceHandle(),SND_ASYNC|SND_RESOURCE|SND_NODEFAULT); //AfxGetResourceHandle���ڻ�ȡ��ǰ��Դģ����
		SetWindowText(strName);
	
	}

	else
	{
		CDialog::OnCancel();
	}

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
		//ASSERT(bNameValid);
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

	m_ComBoUser.AddString("���");        //��������ʾ���
	m_ComBoUser.SetWindowText("���");   //������ʽ�˵���ˢ�����


	CButton* Temp; 
	HBITMAP  hBitmap;
	Temp =(CButton *)GetDlgItem(IDC_BUTTON_TRANS);
	hBitmap =::LoadBitmap(::AfxGetInstanceHandle(),MAKEINTRESOURCE(IDB_BITMAP_SEND));
	Temp->SetBitmap(hBitmap);


	Temp =(CButton *)GetDlgItem(IDC_BUTTON_SHAKE);
	hBitmap =::LoadBitmap(::AfxGetInstanceHandle(),MAKEINTRESOURCE(IDB_BITMAP_LIGHT));
	Temp->SetBitmap(hBitmap);


	// TODO: �ڴ���Ӷ���ĳ�ʼ������

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

void CClientDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CClientDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

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
		//**************************************************************************
		CPaintDC dc(this);
         
		//��ʾ��ȡ���δ�С��Ϣ
		if(m_bShowMsg&&m_bFirstDraw)
		{
			//�õ���ǰ���δ�С
			CRect rect;
			m_rectTracker.GetTrueRect(&rect);
			//����CPaintDC ��Ϊ�˲����˴����ϻ���Ϣ
			DrawMessage(rect,&dc);
		}

		//������Ƥ�����
		if(m_bFirstDraw)
		{
			m_rectTracker.Draw(&dc);
		}

//*************************************************************************
		CDialog::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CClientDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


VOID CClientDlg::SendMsg(int iType,CString strName,CString strMsg)
{
	SENDINFOR SendInfor = {0};
	SendInfor.iType = iType;


	memcpy(SendInfor.szName,strName.LockBuffer(),strName.GetLength()+1);
	memcpy(SendInfor.szMsg,strMsg.LockBuffer(),strMsg.GetLength()+1);

	int bOk = send(m_ClientSocket,(char*)&SendInfor,sizeof(SENDINFOR),0);
	if(bOk==SOCKET_ERROR)
	{
		MessageBox("���͹����з���һ������",NULL,MB_OK);
		return;
	}
}

LRESULT CClientDlg::OnSocketRead(WPARAM wParam,LPARAM lParam)
{
	int iEvent = WSAGETSELECTEVENT(lParam);

	switch (iEvent)
	{
	case FD_CLOSE:
		{
			break;
		}
	case FD_READ:
		{
			OnRecive(wParam); 
			break;
		}
	default:break;
	}

	return 0;
}

VOID CClientDlg::OnRecive(WPARAM wParam)
{
	SENDINFOR  Data = {0};

	int iRet = recv(m_ClientSocket,(char*)&Data,sizeof(Data),0);

	//0        ����     
	//��Ϣ���� �û����� ��Ϣ����

	//0        ����     []����������

	//�������ݰ�
	switch(Data.iType)
	{

	case 20:
		{
			m_EditShow+="\r\n";
			m_EditShow+=Data.szMsg;

			UpdateData(FALSE);

			break;
		}
	case 6:
		{
			m_EditShow+="\r\n";
			m_EditShow+=Data.szMsg;

			UpdateData(FALSE);


			//11
			//����һ���������ݵ��߳�

			CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)SendFileThread,this,0,NULL);


			break;
		}
	case 7:
		{

			m_EditShow+="\r\n";
			m_EditShow+=Data.szMsg;
			UpdateData(FALSE);

			HideItem();
			break;
		}
	case  5:    //11  5    ->12
		{
			AnalyseString(Data.szMsg);    //5  12 192.168.0.100|11|1.txt|5|

			CString strFileSize;
			strFileSize = KindChange(m_dwOtherFileSize);

			strFileSize="("+strFileSize+")";


			m_EditShow+="\r\n\r\n";
			m_EditShow+=m_strOtherName;
			m_EditShow+="������㷢���ļ� ";
			m_EditShow+=m_strOtherFileName;
			m_EditShow+=strFileSize;


			//���¿ؼ�

			GetDlgItem(IDC_PROGRESS)->ShowWindow(SW_SHOW);

			m_ButtonCancle.ShowWindow(SW_SHOW);
			m_ButtonCancle.SetWindowText("�ܾ�");
			m_ButtonRecv.ShowWindow(SW_SHOW);
			GetDlgItem(IDC_BUTTON_TRANS)->ShowWindow(SW_HIDE);



			GetDlgItem(IDC_STATIC_RECV)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_STATIC_RECV)->SetWindowText("�����ļ���");
			GetDlgItem(IDC_STATIC_FILENAME)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_STATIC_FILENAME)->SetWindowText(m_strOtherFileName);
			GetDlgItem(IDC_STATIC_FILESIZE)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_STATIC_FILESIZE)->SetWindowText(strFileSize);

			GetDlgItem(IDC_STATIC)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_STATIC)->SetWindowText("0%");

			GetDlgItem(IDC_STATIC_SPEED)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_STATIC_SPEED)->SetWindowText("�ٶ�: 0");
			if(this->IsWindowVisible()!=1)
			{
				::SetForegroundWindow(m_hWnd);
				::ShowWindow(m_hWnd,SW_SHOW);				
			}



			UpdateData(FALSE);

			break;
		}
	case -1:
		{

			m_EditShow+="\r\n";
			m_EditShow+=Data.szMsg;	



			//���¿ؼ���Ϣ
			int Index = m_ListUser.FindString(0,Data.szName);

			if (Index<0)
			{
				break;
			}
			m_ListUser.DeleteString(Index);


			Index = m_ComBoUser.FindString(0,Data.szName);

			if (Index<0)
			{
				break;
			}
			m_ComBoUser.DeleteString(Index);



			UpdateData(FALSE);



			break;
		}
	case 120:
	case 110:
		{

			m_EditShow+="\r\n";
			m_EditShow+=Data.szMsg;	
			UpdateData(FALSE);

			Shaking();

			break;
		}
	case 1:   //���˵
	case 2:   //˽��
		{

			UpdateData(TRUE);
			if (m_bMe==TRUE)
			{
				if (Data.szName[0]!=0&&strcmp(Data.szName,m_strName.LockBuffer())==0)
				{


					m_EditShow+="\r\n";
					m_EditShow+=Data.szMsg;

				}
				UpdateData(FALSE);
				break;
			}


			m_EditShow+="\r\n";
			m_EditShow+=Data.szMsg;


			UpdateData(FALSE);

			break;
		}
	case 0:   //New People
		{

			m_ListUser.AddString(Data.szName);
			m_ComBoUser.AddString(Data.szName);


			if (Data.szMsg[0]!=NULL)       //��ǰ���ݰ������û������µ��û�
			{
				m_EditShow+=Data.szMsg;    //EditBox �����Ϣ

				UpdateData(FALSE);
			}



			break;
		}
	}




	PlaySound(MAKEINTRESOURCE(IDR_WAVE),
		AfxGetResourceHandle(),SND_ASYNC|SND_RESOURCE|SND_NODEFAULT);
	SetWindowText(m_strName);





	HWND hWnd = ::GetForegroundWindow();

	if (hWnd!=this->m_hWnd)
	{
		this->FlashWindow(1);
	}

}




void CClientDlg::OnButtonShake()
{

	UpdateData(TRUE);
	CString  Data;
	CString Temp;
	m_ComBoUser.GetWindowText(Temp);
	if(Temp==m_strName)
	{
		MessageBox("���ܸ��Լ�����Ϣ");
		return;
	}

	else
	{
		if (Temp=="���")
		{


			GetCurrentTime();
			Data+=m_Timer.LockBuffer();
			Data+=m_strName;
			Data+="�Դ�ҷ�����һ����������";	

			SendMsg(110,"",Data);


			m_EditShow+="\r\n";
			m_EditShow+=m_Timer.LockBuffer();
			m_EditShow+="��Դ�ҷ�����һ����������";


			m_EditSend = "";

			UpdateData(FALSE);


		}
		else
		{

			GetCurrentTime();
			Data+=m_Timer.LockBuffer();
			Data+=m_strName;
			Data+="��";
			Data+=Temp;
			Data+="������һ����������";	

			SendMsg(120,Temp,Data);


			m_EditShow+="\r\n";
			m_EditShow+=m_Timer.LockBuffer();
			m_EditShow+="���";
			m_EditShow+=Temp;
			m_EditShow+="������һ����������";


			m_EditSend = "";

			UpdateData(FALSE);


		}
	}


	Shaking();


}


void CClientDlg::OnButtonTrans()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
}


void CClientDlg::OnCbnSelchangeComboUser()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
}


void CClientDlg::OnEnChangeEditSend()
{
	// TODO:  ����ÿؼ��� RICHEDIT �ؼ���������
	// ���ʹ�֪ͨ��������д CDialog::OnInitDialog()
	// ���������� CRichEditCtrl().SetEventMask()��
	// ͬʱ�� ENM_CHANGE ��־�������㵽�����С�

	// TODO:  �ڴ���ӿؼ�֪ͨ����������
}


void CClientDlg::OnButtonSend()
{
	CString  Data;
	UpdateData(true);
	if(m_EditSend=="")
	{
		MessageBox("��Ϣ����Ϊ��");
		return;
	}

	CString Temp;
	m_ComBoUser.GetWindowText(Temp);
	if(Temp==m_strName)
	{
		MessageBox("���ܸ��Լ�����Ϣ");
		return;
	}
	else
	{

		if (Temp=="���")
		{
			//�������ݰ�
			GetCurrentTime();
			Data+=m_Timer.LockBuffer();
			Data+=m_strName;
			Data+="�Դ��˵:";
			Data+=m_EditSend;


			SendMsg(1,"",Data);

			m_EditShow+="\r\n";
			m_EditShow+=m_Timer.LockBuffer();
			m_EditShow+="��Դ��˵:";
			m_EditShow+=m_EditSend;

			UpdateData(FALSE);
		}

		else
		{


			if (m_bOnly==TRUE)   //˽��
			{


				GetCurrentTime();
				Data+=m_Timer.LockBuffer();
				Data+=m_strName;
				Data+="����˵:";
				Data+=m_EditSend;

				SendMsg(2,Temp,Data);


				m_EditShow+="\r\n";
				m_EditShow+=m_Timer.LockBuffer();
				m_EditShow+="���";
				m_EditShow+=Temp;
				m_EditShow+="˵:";
				m_EditShow+=m_EditSend;


				UpdateData(FALSE);

			}
			else     
			{
				GetCurrentTime();
				Data+=m_Timer.LockBuffer();
				Data+=m_strName;
				Data+="��";
				Data+=Temp;
				Data+="˵:";
				Data+=m_EditSend;


				SendMsg(1,Temp,Data);

				m_EditShow+="\r\n";
				m_EditShow+=m_Timer.LockBuffer();
				m_EditShow+="���";
				m_EditShow+=Temp;
				m_EditShow+="˵:";
				m_EditShow+=m_EditSend;

				UpdateData(FALSE);
			}
		}


	}

	m_EditSend = "";

	UpdateData(FALSE);
}
void CClientDlg::Shaking()
{
	int ty=3;
	CRect   m_rect; 
	this->ShowWindow(SW_SHOW);
	::SetForegroundWindow(m_hWnd);
	::GetWindowRect(m_hWnd,&m_rect);  
	int recordy=m_rect.left;
	int recordx=m_rect.top;
	for(int i=0;i<3;i++)
	{
		m_rect.left=recordy;
		m_rect.top=recordx;
		m_rect.top = m_rect.top + ty;  
		m_rect.left = m_rect.left - ty;
		::SetWindowPos(m_hWnd,NULL,m_rect.left,m_rect.top,0,0,SWP_NOSIZE);Sleep(20);
		m_rect.top = m_rect.top -ty;
		::SetWindowPos( m_hWnd,NULL,m_rect.left,m_rect.top,0,0,SWP_NOSIZE);Sleep(20);
		m_rect.top = m_rect.top -ty;
		::SetWindowPos( m_hWnd,NULL,m_rect.left,m_rect.top,0,0,SWP_NOSIZE);Sleep(20);
		m_rect.left=m_rect.left+ty;
		::SetWindowPos( m_hWnd,NULL,m_rect.left,m_rect.top,0,0,SWP_NOSIZE);Sleep(20);
		m_rect.left=m_rect.left+ty;
		::SetWindowPos( m_hWnd,NULL,m_rect.left,m_rect.top,0,0,SWP_NOSIZE);Sleep(20);
		m_rect.top = m_rect.top + ty;  
		::SetWindowPos( m_hWnd,NULL,m_rect.left,m_rect.top,0,0,SWP_NOSIZE);Sleep(20);
		m_rect.top=m_rect.top+ty;
		::SetWindowPos( m_hWnd,NULL,m_rect.left,m_rect.top,0,0,SWP_NOSIZE);Sleep(20);
		m_rect.top=m_rect.top+ty;
		::SetWindowPos( m_hWnd,NULL,m_rect.left,m_rect.top,0,0,SWP_NOSIZE);Sleep(20);
		::SetWindowPos( m_hWnd,NULL,recordy,recordx,0,0,SWP_NOSIZE);Sleep(3);
	}
}
void CClientDlg::HideItem()
{
	GetDlgItem(IDC_PROGRESS)->ShowWindow(SW_HIDE);
	m_ButtonCancle.ShowWindow(SW_HIDE);
	GetDlgItem(IDC_STATIC_RECV)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_STATIC_FILENAME)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_STATIC_FILESIZE)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_STATIC)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_BUTTON_TRANS)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_BUTTON_RECV)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_STATIC_SPEED)->ShowWindow(SW_HIDE);
}
static DWORD WINAPI SendFileThread(LPVOID lPParam)  //11
{

	CClientDlg* ClientDlg = (CClientDlg*)lPParam;

	//Ҫ���ļ����ݷ�������  fopen  fread 
	CFile File;   
	if(!File.Open(ClientDlg->m_strFileName,CFile::modeRead)) 
	{ 
		AfxMessageBox("���ļ�����!");     //���͸�����һ���ͻ���
		ClientDlg->GetDlgItem(IDC_BUTTON_TRANS)->ShowWindow(SW_SHOW);
		ClientDlg->HideItem();
		return 0; 
	} 

	else
	{

		//�ȴ�һ������
		CSocket sListen;
		sListen.Create(9999);
		sListen.Listen(1);
		CSocket sClient;
		sListen.Accept(sClient);  //Block cin 


		ClientDlg->m_Progress.SetRange(0,100);


		CString strName;
		ClientDlg->m_ComBoUser.GetWindowText(strName);
		ClientDlg->SendMsg(20,strName,"��ʼ��������.......");


		//���ļ�  �����ļ�����
		DWORD ByteTotal = 0;
		DWORD dwTemp  = 0;
		DWORD dwTemp1 = 0;
		CString strTemp;
		CString strTemp1;
		char Buffer[FILEMAX] = {0}; 
		while (1)
		{
			DWORD dwLen = 0;
			dwLen = File.Read(Buffer,FILEMAX);  

			if(dwLen==0)
			{
				break;
			}

			dwLen = sClient.Send(Buffer,dwLen);

			if(dwLen==SOCKET_ERROR)
			{ 
				AfxMessageBox("����ʧ�ܣ�"); 
				break;
			}

			ByteTotal+=dwLen;
			dwTemp = int(ByteTotal*100.0/ClientDlg->m_dwFileSize);
			if(dwTemp1!=dwTemp)
			{
				dwTemp1=dwTemp;
				ClientDlg->m_Progress.SetPos(dwTemp);
			}
			strTemp.Format("%d",dwTemp);
			if(strTemp1!=strTemp)
			{
				strTemp1=strTemp;
				strTemp=strTemp+"%";
				ClientDlg->GetDlgItem(IDC_STATIC)->SetWindowText(strTemp); 
			}

		}


		sClient.Close();
		sListen.Close();
		File.Close();


		ClientDlg->m_Progress.SetPos(0);
		ClientDlg->HideItem();


		if(ByteTotal==ClientDlg->m_dwFileSize)
		{			
			ClientDlg->SendMsg(20,strName,"�ļ��������!");
		}
	}

	return 0;
}

static DWORD WINAPI RecvFileThread(LPVOID lPParam) //12
{
	//����CClientDlg �ĳ�Ա
	CClientDlg* ClientDlg = (CClientDlg*)lPParam;


	//���ΪDlg
	CFileDialog Dlg(FALSE,NULL,NULL,
		OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT,"�����ļ� (*.*)|*.*||");

	//	CFileDialog Dlg(TRUE,NULL,NULL,OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT,  
	//		"�����ļ� (*.*)|*.*||"); 

	strcpy(Dlg.m_ofn.lpstrFile, 
		ClientDlg->m_strOtherFileName.GetBuffer(ClientDlg->m_strOtherFileName.GetLength()));

	if (Dlg.DoModal()!=IDOK)
	{
		return -1;
	}

	else
	{
		ClientDlg->GetDlgItem(IDC_BUTTON_RECV)->ShowWindow(SW_HIDE);//���հ�ť 
		ClientDlg->m_ButtonCancle.SetWindowText("��ֹ");


		CSocket FileSocket;   
		if(FileSocket.Create(9999)==SOCKET_ERROR)
		{
			FileSocket.Create();
		}


		ClientDlg->GetDlgItem(IDC_STATIC_FILENAME)->SetWindowText(Dlg.GetFileName());

		ClientDlg->m_Progress.SetRange(0,100);



		//11-->12   12    6  11  �Է��ѽ������󣬽�������.......
		ClientDlg->SendMsg(6,ClientDlg->m_strOtherName,"�Է��ѽ������󣬽�������.......");


		//����ͨ��Socket
		while(FileSocket.Connect(ClientDlg->m_strOtherIP,9999)==0)
		{
			Sleep(50);   
		}



		CFile File; 
		if(!File.Open(Dlg.GetPathName(),CFile::modeCreate|CFile::modeWrite))
		{ 
			AfxMessageBox("���ļ�����"); 
			return 0; 
		} 	

		ClientDlg->SendMsg(20,ClientDlg->m_strOtherName,"��ʼ��������.......");


		DWORD ByteTotal = 0;
		DWORD dwTemp  = 0;
		DWORD dwTemp1 = 0;
		CString strTemp;
		CString strTemp1;
		//�����ļ�  �������ļ�
		char Buffer[FILEMAX] = {0}; 
		while (1)
		{

			DWORD dwLen = 0;
			dwLen = FileSocket.Receive(Buffer,FILEMAX);			
			if(dwLen==SOCKET_ERROR)
			{ 
				AfxMessageBox("����ʧ��"); 
				break;
			}

			if(dwLen==0)
			{
				break;
			}

			File.Write(Buffer,dwLen);

			ByteTotal+=dwLen;
			dwTemp = int(ByteTotal*100.0/ClientDlg->m_dwOtherFileSize);
			if(dwTemp1!=dwTemp)
			{
				dwTemp1=dwTemp;
				ClientDlg->m_Progress.SetPos(dwTemp);
			}
			strTemp.Format("%d",dwTemp);
			if(strTemp1!=strTemp)
			{
				strTemp1=strTemp;
				strTemp=strTemp+"%";
				ClientDlg->GetDlgItem(IDC_STATIC)->SetWindowText(strTemp); 
			}

		}

		FileSocket.Close();	
		File.Close();
		ClientDlg->m_Progress.SetPos(0);
		ClientDlg->HideItem();

		if(ByteTotal==ClientDlg->m_dwFileSize)
		{
			ClientDlg->SendMsg(20,ClientDlg->m_strOtherName,"�ļ��������!");
		}

	}

	return 0;

}
void CClientDlg::AnalyseString(char* szBuffer)
{

	char* szTemp = NULL;
	char* szStart = szBuffer;
	int j = 0;
	while (1)
	{


		szTemp = strstr(szStart,"|");  //192.168.0.100\011\01.txt\05\0 

		if (szTemp==NULL)
		{
			break;
		}

		*szTemp = '\0';


		if (j==0)
		{
			m_strOtherIP = szStart;
		}

		else if (j==1)
		{
			m_strOtherName = szStart;

		}

		else if (j==2)
		{
			m_strOtherFileName = szStart;

		}

		else if (j==3)
		{

			m_dwOtherFileSize = atoi(szStart);   //char int

		}


		szTemp++;
		szStart = szTemp;
		j++;
	}
}

CString CClientDlg::KindChange(DWORD dwFileSize)  
{
	float Size;
	CString strFileSize;
	if(dwFileSize<1024)
	{
		strFileSize.Format("%d�ֽ�",dwFileSize);
	}
	else if(dwFileSize<(1024*1024))
	{
		Size = dwFileSize*1.0/1024;
		strFileSize.Format("%.1fKB",Size);
	}
	else
	{
		Size=dwFileSize*1.0/(1024*1024);
		strFileSize.Format("%.1fMB",Size);
	}
	return strFileSize;
}



void CClientDlg::OnButtonRecv()
{
	//Server
	//���������߳�    P2P   12 connect 11  Accept
	CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)RecvFileThread,this,0,NULL);
}
//*********************���ӵĺ���**********************************************************
//������Ļ,��δ������������� �Ǻ�
HBITMAP CClientDlg::CopyScreenToBitmap(LPRECT lpRect,BOOL bSave)
//lpRect ����ѡ������
{
	HDC       hScrDC, hMemDC;      
	// ��Ļ���ڴ��豸������
	HBITMAP    hBitmap, hOldBitmap;   
	// λͼ���
	int       nX, nY, nX2, nY2;      
	// ѡ����������
	int       nWidth, nHeight;
	
	// ȷ��ѡ������Ϊ�վ���
	if (IsRectEmpty(lpRect))
		return NULL;
	//Ϊ��Ļ�����豸������
	hScrDC = CreateDC("DISPLAY", NULL, NULL, NULL);

	//Ϊ��Ļ�豸�����������ݵ��ڴ��豸������
	hMemDC = CreateCompatibleDC(hScrDC);
	// ���ѡ����������
	nX = lpRect->left;
	nY = lpRect->top;
	nX2 = lpRect->right;
	nY2 = lpRect->bottom;

	//ȷ��ѡ�������ǿɼ���
	if (nX < 0)
		nX = 0;
	if (nY < 0)
		nY = 0;
	if (nX2 > m_xScreen)
		nX2 = m_xScreen;
	if (nY2 > m_yScreen)
		nY2 = m_yScreen;
	nWidth = nX2 - nX;
	nHeight = nY2 - nY;
	// ����һ������Ļ�豸��������ݵ�λͼ
	hBitmap = CreateCompatibleBitmap
		(hScrDC, nWidth, nHeight);
	// ����λͼѡ���ڴ��豸��������
	hOldBitmap = (HBITMAP)SelectObject(hMemDC, hBitmap);
	// ����Ļ�豸�����������ڴ��豸��������
	if(bSave)
	{
		//����������DC,��bSaveΪ��ʱ�ѿ�ʼ�����ȫ��λͼ,����ȡ���δ�С����
		CDC dcCompatible;
		dcCompatible.CreateCompatibleDC(CDC::FromHandle(hMemDC));
		dcCompatible.SelectObject(m_pBitmap);
        
		BitBlt(hMemDC, 0, 0, nWidth, nHeight,
			dcCompatible, nX, nY, SRCCOPY);

	}
	else
	{
		BitBlt(hMemDC, 0, 0, nWidth, nHeight,
			hScrDC, nX, nY, SRCCOPY);
	}

	hBitmap = (HBITMAP)SelectObject(hMemDC, hOldBitmap);
	//�õ���Ļλͼ�ľ��
	//��� 
	DeleteDC(hScrDC);
	DeleteDC(hMemDC);
	
	if(bSave)
	{
				
		if (OpenClipboard()) 
		{
        //��ռ�����
        EmptyClipboard();
        //����Ļ����ճ������������,
        //hBitmap Ϊ�ղŵ���Ļλͼ���
        SetClipboardData(CF_BITMAP, hBitmap);
        //�رռ�����
        CloseClipboard();
      }
	}
	// ����λͼ���

	return hBitmap;
}
//��ʾ������ʾ��Ϣ
void CClientDlg::DrawTip()
{

	//�õ�ǰ��������,
	CPoint pt;
	GetCursorPos(&pt);

	//������ǰR,G,B,������ֵ
	COLORREF color;
	CClientDC dc(this);
	color=dc.GetPixel(pt);
	BYTE rValue,gValue,bValue;
	rValue=GetRValue(color);
	gValue=GetGValue(color);
	bValue=GetGValue(color);

	//����ʽ�ŷ��ַ���
	CString string;
	CString strTemp;
	string.Format("\r\n\r\n\r\n ����ǰ����RGB (%d,%d,%d)\r\n",rValue,gValue,bValue);

	if(!m_bDraw&&!m_bFirstDraw)
	{
		strTemp="\r\n ����������������ѡ���ȡ\r\n ��Χ\r\n\r\n ����ESC��������Ҽ��˳�";
	}
	else
		if(m_bDraw&&m_bFirstDraw)
		{
			strTemp="\r\n ���ɿ�������ȷ����ȡ��Χ\r\n\r\n ����ESC���˳�";
		}
		else
			if(m_bFirstDraw)
			{
				strTemp="\r\n ����������������ȡ��Χ��\r\n ��С��λ��\r\n\r\n ����ȡ��Χ��˫����������\r\n ��ͼ�񣬽�������\r\n\r\n ���������Ҽ�����ѡ��";
			}
			string+=strTemp;
			//��ʾ���༩����,������ʾ����
			m_tipEdit.SetWindowText(string);
}
//��ʾ��ȡ������Ϣ
void CClientDlg::DrawMessage(CRect &inRect,CDC * pDC)
{
	//��ȡ���δ�С��Ϣ�������
	const int space=3;

	//����������ɫ��С

	CPoint pt;
	CPen pen(PS_SOLID,1,RGB(147,147,147));

	//dc.SetTextColor(RGB(147,147,147));
	CFont font;
	CFont * pOldFont;
	font.CreatePointFont(90,"����");
	pOldFont=pDC->SelectObject(&font);

	//�õ������Ⱥ͸߶�
	GetCursorPos(&pt);
	int OldBkMode;
	OldBkMode=pDC->SetBkMode(TRANSPARENT);

	TEXTMETRIC tm;
	int charHeight;
	CSize size;
	int	lineLength;
	pDC->GetTextMetrics(&tm);
	charHeight = tm.tmHeight+tm.tmExternalLeading;
	size=pDC->GetTextExtent("����λ��  ",strlen("����λ��  "));
	lineLength=size.cx;

	//��ʼ������, �Ա�֤д����������
	CRect rect(pt.x+space,pt.y-charHeight*6-space,pt.x+lineLength+space,pt.y-space);

	//������ʱ����
	CRect rectTemp;
	//�����ε��������Եʱ��������ʹ�С
	if((pt.x+rect.Width())>=m_xScreen)
	{
		//�����Ϸ���ʾ���¾���
		rectTemp=rect;
		rectTemp.left=rect.left-rect.Width()-space*2;
		rectTemp.right=rect.right-rect.Width()-space*2;;
		rect=rectTemp;
	}

	if((pt.y-rect.Height())<=0)
	{
		//�����ҷ���ʾ���¾���
		rectTemp=rect;
		rectTemp.top=rect.top+rect.Height()+space*2;;
		rectTemp.bottom=rect.bottom+rect.Height()+space*2;;
		rect=rectTemp;

	}

	//�����ջ�ˢ������
	CBrush * pOldBrush;
	pOldBrush=pDC->SelectObject(CBrush::FromHandle((HBRUSH)GetStockObject(NULL_BRUSH)));

	pDC->Rectangle(rect);
	rect.top+=2;
	//�ھ�������ʾ����
	CRect outRect(rect.left,rect.top,rect.left+lineLength,rect.top+charHeight);
	CString string("����λ��");
	pDC->DrawText(string,outRect,DT_CENTER);

	outRect.SetRect(rect.left,rect.top+charHeight,rect.left+lineLength,charHeight+rect.top+charHeight);
	string.Format("(%d,%d)",inRect.left,inRect.top);
	pDC->DrawText(string,outRect,DT_CENTER);


	outRect.SetRect(rect.left,rect.top+charHeight*2,rect.left+lineLength,charHeight+rect.top+charHeight*2);
	string="���δ�С";
	pDC->DrawText(string,outRect,DT_CENTER);

	outRect.SetRect(rect.left,rect.top+charHeight*3,rect.left+lineLength,charHeight+rect.top+charHeight*3);
	string.Format("(%d,%d)",inRect.Width(),inRect.Height());
	pDC->DrawText(string,outRect,DT_CENTER);

	outRect.SetRect(rect.left,rect.top+charHeight*4,rect.left+lineLength,charHeight+rect.top+charHeight*4);
	string="�������";
	pDC->DrawText(string,outRect,DT_CENTER);

	outRect.SetRect(rect.left,rect.top+charHeight*5,rect.left+lineLength,charHeight+rect.top+charHeight*5);
	string.Format("(%d,%d)",pt.x,pt.y);
	pDC->DrawText(string,outRect,DT_CENTER);

	pDC->SetBkMode(OldBkMode);
	pDC->SelectObject(pOldFont);
	pDC->SelectObject(pOldBrush);

}

void CClientDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ

	CDialog::OnMouseMove(nFlags, point);
}
