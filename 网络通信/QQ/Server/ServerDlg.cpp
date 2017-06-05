// ServerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Server.h"
#include "ServerDlg.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


#define  WM_SHOWTASK   WM_USER+1
#define  WM_SERVER_ACCEPT  WM_USER+2
/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CServerDlg dialog

CServerDlg::CServerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CServerDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CServerDlg)
	m_strShow = _T("");

	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_People = 0;
}

void CServerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CServerDlg)
	DDX_Control(pDX, IDC_STATIC_PEOPLE, m_strPeople);
	DDX_Control(pDX, IDC_STATIC_HOST, m_strHost);
	DDX_Text(pDX, IDC_EDIT_SHOW, m_strShow);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CServerDlg, CDialog)
	//{{AFX_MSG_MAP(CServerDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_MESSAGE(WM_SHOWTASK,OnShowTask)
	ON_MESSAGE(WM_SERVER_ACCEPT,OnServerAccept)
	ON_WM_DESTROY()
	ON_COMMAND(ID_EXIT,OnDlgExit)
	ON_COMMAND(ID_SHOW,OnDlgShow)
	ON_COMMAND(ID_HIDE,OnDlgHide)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CServerDlg message handlers

BOOL CServerDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon	
	
	
	InitNotify();
	SetWindowText("������");//�ڶԻ���������ʾ����

                   //1 ����Socket��
	InitSocket();  //2 ��ʼ��Socket��



	return TRUE;  // return TRUE  unless you set the focus to a control
}

VOID CServerDlg::GetHostInfor()
{
	char szHostName[128];   //���Shine-PC                                                                            // �洢������
	if (gethostname(szHostName,128)==0)                                                   //��ȷ�����
	{
		hostent* pHost;    //Host Content
		pHost = gethostbyname(szHostName);  //������Ϣ                                                   //pHost���ص���ָ���������б�
		LPCSTR psz = inet_ntoa(*(struct in_addr *)pHost->h_addr_list[0]);//�õ�ָ��ip��psz����
	
		CString strTemp;
		strTemp += "������:";
		strTemp += szHostName;
		strTemp +='  ';

		strTemp+="IP��ַ:";
		strTemp+=psz;

		m_strHost.SetWindowText(strTemp);



		CString strPeople;
		strPeople.Format("%d",m_People);
		m_strPeople.SetWindowText(strPeople);
	}
}

VOID CServerDlg::InitSocket()
{
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2,2),&wsaData);

	GetHostInfor();  


	//���������׽���

	m_sListen = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);  

	if(m_sListen==INVALID_SOCKET)
	{
		MessageBox("Create SOCKET Failed");
		WSACleanup(); 
		return;
	}

	//��ʼ������
	//sockaddr_in(2.2) sockaddr(1.0)
	m_ServerAddr.sin_family = AF_INET;
	m_ServerAddr.sin_addr.s_addr = INADDR_ANY;
	m_ServerAddr.sin_port = htons(9527); 
	

	//1.0
	//��
	if(bind(m_sListen,
		(sockaddr*)&m_ServerAddr,sizeof(m_ServerAddr))==SOCKET_ERROR)
	{
		MessageBox("Bind Failed");
		WSACleanup(); 
		closesocket(m_sListen);
		return;
	}
	//����
	if(listen(m_sListen,20)==SOCKET_ERROR)
	{
		MessageBox("Listen Failed");
		WSACleanup(); 
		closesocket(m_sListen);
		return;
	}

	WSAAsyncSelect(m_sListen,m_hWnd,WM_SERVER_ACCEPT,
		FD_ACCEPT|FD_READ|FD_CLOSE);


	for(int i=0;i<MAX;i++)
	{
		m_sClient[i] = INVALID_SOCKET;
	}

	m_strShow="Now server is working!\r\nWaiting for client's connecting .......";
	UpdateData(false);

	return;

}

void CServerDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CServerDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CServerDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}



VOID CServerDlg::InitNotify()
{
	m_nid.cbSize = sizeof(NOTIFYICONDATA);
	m_nid.hWnd = m_hWnd;
	m_nid.uID = IDI_ICON_NOTIFY; 
	m_nid.uFlags = NIF_ICON|NIF_MESSAGE|NIF_TIP; 
	m_nid.uCallbackMessage = WM_SHOWTASK;   //�Զ�����Ϣ   �������ϴ�����궯��


	m_nid.hIcon = LoadIcon(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDI_ICON_NOTIFY)); 
	strcpy(m_nid.szTip,"����");//������������ʱ������ʾ������ 
	Shell_NotifyIcon(NIM_ADD,&m_nid);//�����������ͼ�� 

		
}



LRESULT CServerDlg::OnShowTask(WPARAM wParam,LPARAM lParam)
{
//wParam���յ���ͼ���ID����lParam���յ���������Ϊ   

	if (wParam!=IDI_ICON_NOTIFY)
	{
		return 1;
	}


	switch(lParam)
	{
	case  WM_RBUTTONUP:
		{

			CMenu Menu;
			Menu.CreatePopupMenu();
			Menu.AppendMenu(MF_STRING, ID_SHOW,"��ʾ");
			Menu.AppendMenu(MF_STRING, ID_HIDE,"����");	
			Menu.AppendMenu(MF_STRING, ID_EXIT,"�˳�");	
		
			CPoint Pt;
			GetCursorPos(&Pt);         //�õ����λ��
			Menu.TrackPopupMenu(TPM_RIGHTBUTTON, Pt.x, Pt.y, this);
			Menu.DestroyMenu();        //������Դ
		
			break;
		}
	}

	return 0;
}

void CServerDlg::OnDestroy() 
{
	CDialog::OnDestroy();
	
	Shell_NotifyIcon(NIM_DELETE,&m_nid);
}



void CServerDlg::OnDlgExit()
{

	SendMessage(WM_CLOSE,NULL,NULL);
}


void CServerDlg::OnDlgHide()
{
	ShowWindow(SW_HIDE);
}

void CServerDlg::OnDlgShow()
{
	::ShowWindow(this->m_hWnd,SW_NORMAL);
}



//FD_ACCEPT  FD_CLOSE   FD_READ
LRESULT CServerDlg::OnServerAccept(WPARAM wParam,LPARAM lParam)
{

	// WSAGETSELECTEVENT == LOWORD(lParam)
	int iEvent = WSAGETSELECTEVENT(lParam); //����Winsock API�������õ������¼�����
	switch(iEvent) 
	{ 
	case FD_ACCEPT://�ͻ������������¼� 
		{
			OnAccept();   //�û�����������   Connect
			break;
		}
	case FD_CLOSE://�ͻ��˶Ͽ��¼�: 
		{

			OnClose(wParam);   //WParam ˭���ر�
			break;
		}
	 
	case FD_READ://�������ݰ������¼� 
		{

			OnRecive(wParam);    //Send 
			break;
		}
	default: break; 
	} 
	return 0;
}

VOID CServerDlg::OnRecive(WPARAM wParam)
{
	SENDINFOR  Data = {0};

	int i = 0;
	for (i=0;i<MAX;i++)
	{
		if (m_sClient[i]==wParam)
		{
			break;
		}
	}
	
	if (i==MAX) 
	{
		return;
	}
	
	int iRet = recv(m_sClient[i],(char*)&Data,sizeof(Data),0);

    //0        ����     
	//��Ϣ���� �û����� ��Ϣ����

	//�������ݰ�
	switch(Data.iType)
	{

	case 20:
	case 6:
		{
			//6  11  �Է��ѽ������󣬽�������.......

			int j = 0;
			for (j=0;j<MAX;j++)
			{
				if (m_sClient[j]!=INVALID_SOCKET&&
					strcmp(m_PeopleInfor[j].strName.LockBuffer(),Data.szName)==0)
				{
					send(m_sClient[j],(char*)&Data,sizeof(Data),0);
				}
			}

			break;
		}
	case 7:
		{

			int j = 0;
			for (j=0;j<MAX;j++)
			{
				if (m_sClient[j]!=INVALID_SOCKET&&
					strcmp(m_PeopleInfor[j].strName.LockBuffer(),Data.szName)==0)
				{
					send(m_sClient[j],(char*)&Data,sizeof(Data),0);
				}
			}



			break;
		}
	case 5:    //11 -12   
		{
            //11--->12
			//5 12 11|1.txt|5|
			//5 12 192.168.0.100|11|1.txt|5|    --->12

			CString strTemp;	
			strTemp+=m_PeopleInfor[i].strIp;
			strTemp+="|";
			strTemp+=Data.szMsg;
			
			strcpy(Data.szMsg,strTemp.LockBuffer());



			int j = 0;
			for (j=0;j<MAX;j++)
			{
				if (m_sClient[j]!=INVALID_SOCKET&&
					strcmp(m_PeopleInfor[j].strName.LockBuffer(),Data.szName)==0)
				{
					send(m_sClient[j],(char*)&Data,sizeof(Data),0);
				}
			}

			break;
		}

	case 120:
		{

			int j = 0;
			for (j=0;j<MAX;j++)
			{
				if (m_sClient[j]!=INVALID_SOCKET&&
					strcmp(m_PeopleInfor[j].strName.LockBuffer(),Data.szName)==0)
				{
					send(m_sClient[j],(char*)&Data,sizeof(Data),0);
				}
			}


			break;
		}
	case 110:
		{
			int j = 0;
			for (j=0;j<MAX;j++)
			{
				if (m_sClient[j]!=INVALID_SOCKET&&j!=i)
				{
					send(m_sClient[j],(char*)&Data,sizeof(Data),0);
				}
			}
			break;
		}
	case 2:
		{

			int j = 0;
			for (j=0;j<MAX;j++)
			{
				if (strcmp(m_PeopleInfor[j].strName,Data.szName)==0
					&&m_sClient[j]!=INVALID_SOCKET)
				{
					send(m_sClient[j],(char*)&Data,sizeof(Data),0);

					break;
				}
			}



			break;
		}
	case 1:   
		{

			int j = 0;
			for (j=0;j<MAX;j++)
			{
				if (m_sClient[j]!=INVALID_SOCKET&&j!=i)
				{
					send(m_sClient[j],(char*)&Data,sizeof(Data),0);
				}
			}
			break;
		}
	case 0:   //New People
		{
			m_PeopleInfor[i].strName = Data.szName;

			//��ؼ��������
			OnlineOrOutline(i,"����");


			//ˢ���û�����
			//1���ϵ�֪ͨ�µ��û�(�Լ��õ��Լ�)
			int j = 0;
			for (j=0;j<MAX;j++)
			{
				if (m_sClient[j]!=INVALID_SOCKET)
				{
					CString Temp;

				

					Data.iType = 0;
					strcpy(Data.szName,m_PeopleInfor[i].strName); 

					Temp +="\r\n";
					GetCurrentTime();
					Temp+=m_Timer.LockBuffer();
					Temp+="ϵͳ��Ϣ:\r\n  ";
					Temp+=m_PeopleInfor[i].strName;
					Temp+="����������";

					strcpy(Data.szMsg,Temp.LockBuffer());


					send(m_sClient[j],(char*)&Data,sizeof(Data),0);
				}
			}


			//���µ�֪ͨ�ϵ��û�
			j = 0;
			for (j=0;j<MAX;j++)
			{
				if (m_sClient[j]!=INVALID_SOCKET && j!=i)
				{

					Data.iType = 0;
					memset(Data.szMsg,0,400);
					strcpy(Data.szName,m_PeopleInfor[j].strName); 
					send(m_sClient[i],(char*)&Data,sizeof(Data),0);
				}
			}

			break;
		}
	}
}


VOID CServerDlg::OnAccept()
{
	
	int i = 0;
	for (i=0;i<MAX;i++)
	{
		if (m_sClient[i]==INVALID_SOCKET)
		{
			break;
		}
	}
	
	if (i==MAX)
	{
		return;
	}
	
	sockaddr_in  ClientAddr;
	int nLen = sizeof(sockaddr_in);
	
	m_sClient[i] = accept(m_sListen,(sockaddr*)&ClientAddr,&nLen);
	
	
	
	LPCSTR strIP = inet_ntoa(*(struct in_addr*)&ClientAddr.sin_addr);
	m_PeopleInfor[i].strIp = strIP;
	
	
	m_People++;
	
	CString strPeople;
	strPeople.Format("%d",m_People);
	m_strPeople.SetWindowText(strPeople);
	
	UpdateData(FALSE);
}


VOID CServerDlg::GetCurrentTime()
{
	
	SYSTEMTIME st = {0};
	GetLocalTime(&st);  
	int j = 0;
	j  = sprintf(m_Timer.LockBuffer(),"%d��",st.wYear);
    j += sprintf(m_Timer.LockBuffer()+j,"%d��",st.wMonth);
	j += sprintf(m_Timer.LockBuffer()+j,"%d��",st.wDay);
    j += sprintf(m_Timer.LockBuffer()+j,"%dʱ",st.wHour);
	
	if (st.wMinute<10) 
	{
		int i = 0;
		j += sprintf(m_Timer.LockBuffer()+j,"%d",i); 
	}
	j += sprintf(m_Timer.LockBuffer()+j,"%d����",st.wMinute);
	
	if (st.wSecond<10) 
	{
		int i = 0;
		j += sprintf(m_Timer.LockBuffer()+j,"%d",i); 
	}
	j += sprintf(m_Timer.LockBuffer()+j,"%d��",st.wSecond);
	
	
}


VOID CServerDlg::OnlineOrOutline(int iIndex,CString YesOrNo)
{
	CString strTemp;
	
	GetCurrentTime();
	
	strTemp+=m_Timer.LockBuffer();
	strTemp+=' ';
	strTemp+="�û�:";
	strTemp+= m_PeopleInfor[iIndex].strName;
	strTemp+=" ";
	strTemp+=m_PeopleInfor[iIndex].strIp;
	strTemp+= YesOrNo;
	
	
	m_strShow +="\r\n";
	m_strShow += strTemp;
	
	UpdateData(FALSE);
}




VOID CServerDlg::OnClose(WPARAM wParam)
{
	
	int i = 0;
	for (i=0;i<MAX;i++)
	{
		if (m_sClient[i]==wParam)
		{
			break;
		}
	}
	
	if (i==MAX)
	{
		return;
	}
	
	OnlineOrOutline(i,"����");
	
	m_People--;
	
	CString strPeople;
	strPeople.Format("%d",m_People);
	m_strPeople.SetWindowText(strPeople);
	
	

	SENDINFOR  Data;
	int j = 0;
	for (j=0;j<MAX;j++)
	{
		if (m_sClient[j]!=INVALID_SOCKET&&j!=i)
		{
			Data.iType = -1;
	
			CString strMsg;
			GetCurrentTime();
			strMsg+=m_Timer.LockBuffer();
			strMsg+=m_PeopleInfor[i].strName;
			strMsg+="�˳��˸�������";
			
			strcpy(Data.szName,m_PeopleInfor[i].strName.LockBuffer());
			strcpy(Data.szMsg,strMsg.LockBuffer()); 
			send(m_sClient[j],(char*)&Data,sizeof(Data),0);
		}
	}
	
	
	//����˳��û�����Դ
	closesocket(m_sClient[i]);
	m_sClient[i] = INVALID_SOCKET;
	
	m_PeopleInfor[i].strIp = "";
	m_PeopleInfor[i].strName = "";
	
	
}


