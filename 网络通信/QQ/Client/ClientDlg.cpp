// ClientDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Client.h"
#include "ClientDlg.h"
#include "Login.h"
#include <Mmsystem.h>
#include <afxsock.h>
#pragma comment(lib,"Winmm.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

#define FILEMAX  17520

static DWORD WINAPI RecvFileThread(LPVOID lPParam);
static DWORD WINAPI SendFileThread(LPVOID lPParam);

#define  WM_SOCKET_READ  WM_USER+2
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
// CClientDlg dialog

CClientDlg::CClientDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CClientDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CClientDlg)
	m_EditShow = _T("");
	m_EditSend = _T("");
	m_bOnly = FALSE;
	m_bMe = FALSE;
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDI_ICON);
	m_dwFileSize = 0;
	m_ByteTotal = 0;
	m_Temp = 0;
}



VOID CClientDlg::GetCurrentTime()
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

void CClientDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CClientDlg)
	DDX_Control(pDX, IDC_BUTTON_RECV, m_ButtonRecv);
	DDX_Control(pDX, IDC_BUTTON_END, m_ButtonCancle);
	DDX_Control(pDX, IDC_PROGRESS, m_Progress);
	DDX_Control(pDX, IDC_COMBO_USER, m_ComBoUser);
	DDX_Control(pDX, IDC_LIST_USER, m_ListUser);
	DDX_Text(pDX, IDC_EDIT_SHOW, m_EditShow);
	DDX_Text(pDX, IDC_EDIT_SEND, m_EditSend);
	DDX_Check(pDX, IDC_CHECK_ONLY, m_bOnly);
	DDX_Check(pDX, IDC_CHECK_ME, m_bMe);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CClientDlg, CDialog)
	//{{AFX_MSG_MAP(CClientDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_MESSAGE(WM_SOCKET_READ,OnSocketRead)
	ON_BN_CLICKED(IDC_BUTTON_TRANS, OnButtonTrans)
	ON_BN_CLICKED(IDC_BUTTON_SHAKE, OnButtonShake)
	ON_BN_CLICKED(IDC_BUTTON_SEND, OnButtonSend)
	ON_BN_CLICKED(IDC_BUTTON_END, OnButtonEnd)
	ON_BN_CLICKED(IDC_BUTTON_RECV, OnButtonRecv)
	ON_LBN_DBLCLK(IDC_LIST_USER, OnDblclkListUser)
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CClientDlg message handlers



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


BOOL CClientDlg::OnInitDialog()
{
	CDialog::OnInitDialog();


	CLogin  LoginDlg;

	if (LoginDlg.DoModal()==IDOK)
	{

		//LoginDlg ���ServerIP ����
		CString strIP;



		m_strName = LoginDlg.m_strName;
	
		ULONG ulServerIP;                //127.0.0.1  192.168.1.101
		ulServerIP = inet_addr(LoginDlg.m_strIP);//�ַ���IPת32λIP



		m_ClientSocket = socket(AF_INET,SOCK_STREAM,0);
		if(m_ClientSocket==INVALID_SOCKET)
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
	
		if(bOk==SOCKET_ERROR)
		{
			MessageBox("���ӹ��̷�������\n��ȷ��IP������ȷ����",NULL,MB_OK);
			CDialog::OnCancel();
		}
		else
		{
		
			SendMsg(0,m_strName,"");   //0 ���ݵ�����  ����������͵�һ����Ϣ ����
		}



		WSAAsyncSelect(m_ClientSocket,m_hWnd,WM_SOCKET_READ,FD_READ|FD_CLOSE);
		//ע��һ���Զ�����Ϣ����ӷ������������ݰ�����ͷ������ر�


	
	}

	else
	{
		CDialog::OnCancel();
	}



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
	

	m_ComBoUser.AddString("���");
	m_ComBoUser.SetWindowText("���");


	//����BitMapButton

	CButton* Temp; 
	HBITMAP  hBitmap;
	Temp =(CButton *)GetDlgItem(IDC_BUTTON_TRANS);
	hBitmap =::LoadBitmap(::AfxGetInstanceHandle(),MAKEINTRESOURCE(IDB_BITMAP_SEND));
	Temp->SetBitmap(hBitmap);


	Temp =(CButton *)GetDlgItem(IDC_BUTTON_SHAKE);
	hBitmap =::LoadBitmap(::AfxGetInstanceHandle(),MAKEINTRESOURCE(IDB_BITMAP_LIGHT));
	Temp->SetBitmap(hBitmap);




	
	return TRUE;  // return TRUE  unless you set the focus to a control
}   

LRESULT CClientDlg::OnSocketRead(WPARAM wParam,LPARAM lParam)  //Read  Close
{

	int iEvent = WSAGETSELECTEVENT(lParam);

	switch(iEvent) 
	{ 
	case FD_CLOSE://�������Ͽ��¼�: 
		{			
		
			break;
		}
		
	case FD_READ:              //�������ݰ������¼� 
		{			
			OnRecive(wParam);    
			break;
		}
	default: break; 
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




            
VOID CClientDlg::SendMsg(int iType,CString strName,CString strMsg)
{
	SENDINFOR SendInfor = {0};
	SendInfor.iType = iType;
	

	memcpy(SendInfor.szName,strName.LockBuffer(),strName.GetLength());
	memcpy(SendInfor.szMsg,strMsg.LockBuffer(),strMsg.GetLength());

	int bOk = send(m_ClientSocket,(char*)&SendInfor,sizeof(SENDINFOR),0);
	if(bOk==SOCKET_ERROR)
	{
		MessageBox("���͹����з���һ������",NULL,MB_OK);
		return;
	}
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

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CClientDlg::OnPaint() 
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
HCURSOR CClientDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
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


void CClientDlg::OnButtonTrans() 
{

	UpdateData(TRUE);
    
	CString Temp;
	m_ComBoUser.GetWindowText(Temp);
	if(Temp==m_strName)
	{
		MessageBox("���ܸ��Լ������ļ�");
		return;
	}

	if (Temp=="���")
	{
		MessageBox("���ܸ���ҷ����ļ�");
		return;
	}

	CFileDialog Dlg(TRUE,NULL,NULL,OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT,  
		"�����ļ� (*.*)|*.*||"); 


	if (Dlg.DoModal()==IDOK)
	{

		m_strFilePath = Dlg.GetPathName(); 
		m_strFileName = Dlg.GetFileName();

		CFile File(m_strFilePath,CFile::modeRead); 
		m_dwFileSize = File.GetLength();
		File.Close();

		m_strFileSize = KindChange(m_dwFileSize);

		//��ʾ������

		m_Progress.ShowWindow(SW_SHOW);
		//GetDlgItem(IDC_PROGRESS)->ShowWindow(SW_SHOW);

		m_ButtonCancle.ShowWindow(SW_SHOW);
		GetDlgItem(IDC_STATIC_RECV)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_STATIC_RECV)->SetWindowText("�����ļ���");

		GetDlgItem(IDC_STATIC_FILENAME)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_STATIC_FILENAME)->SetWindowText(m_strFileName);
		GetDlgItem(IDC_STATIC_FILESIZE)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_STATIC_FILESIZE)->SetWindowText(m_strFileSize);   //1025  1K

		GetDlgItem(IDC_STATIC)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_STATIC)->SetWindowText("0%");

		GetDlgItem(IDC_BUTTON_TRANS)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_STATIC_SPEED)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_STATIC_SPEED)->SetWindowText("�ٶ�: 0");



		//��������������ݰ�
		CString strMsg;
		CString strFileSize;
		strFileSize.Format("%d",m_dwFileSize);  //1025
	
		strMsg = m_strName+"|"+m_strFileName+"|"+strFileSize+"|";
	
		//11--12
		//5  12  11|1.txt|5|
		SendMsg(5,Temp,strMsg);


	
		strFileSize = KindChange(m_dwFileSize);
		
		strFileSize="("+strFileSize+")";
		
		
		m_EditShow+="\r\n\r\n";
		m_EditShow+="�������";
		m_EditShow+=Temp;
		m_EditShow+="�����ļ� ";
		m_EditShow+=m_strFileName;
		m_EditShow+=strFileSize;
		
		UpdateData(FALSE);








		//5  ���� �ŷ�|1.txt|5|


//		Sendto=m_strUser;
//		IsTrans=1;
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


BOOL CClientDlg::PreTranslateMessage(MSG* pMsg) 
{


	if (pMsg->message==WM_KEYDOWN)
	{
		if (pMsg->wParam==VK_RETURN)
		{
			OnButtonSend();

			return TRUE;
		}
	}
	
	return CDialog::PreTranslateMessage(pMsg);
}

void CClientDlg::OnButtonEnd() 
{
	
	CString strTemp;
	m_ButtonCancle.GetWindowText(strTemp);
	if (strTemp=="ȡ��")
	{
		m_EditShow+="\r\n";
		m_EditShow+="��ȡ�����ļ�����!";
		UpdateData(FALSE);



		CString SendTo;
		m_ComBoUser.GetWindowText(SendTo);   //����
		SendMsg(7,SendTo,"�Է�ȡ�����ļ�����!");

		HideItem();
	}

	else if (strTemp=="�ܾ�")
	{
		m_EditShow+="\r\n";
		m_EditShow+="��ܾ����ļ�����!";
		UpdateData(FALSE);


		CString SendFrom;
	
		SendFrom = m_strOtherName;    //5 192.168.0.100|11|1.txt|5|
	
		//7 11 �Է��ܾ����ļ�����
		SendMsg(7,SendFrom,"�Է��ܾ����ļ�����!");    //11->12    12 
		//7  �ŷ�  �Է�ȡ�����ļ�����

		HideItem();
	}


	//5  12   192.168.0.111|11|1.txt|5|   ==>  m_strOtherFileName ClientDlg
	
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

void CClientDlg::OnButtonRecv() 
{                           //Server
	//���������߳�    P2P   12 connect 11  Accept
	CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)RecvFileThread,this,0,NULL);
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
		ClientDlg->m_ByteTotal = 0;
		DWORD dwTemp  = 0;
		DWORD dwTemp1 = 0;
		CString strTemp;
	    CString strTemp1;
		char Buffer[FILEMAX] = {0}; 
		ClientDlg->m_Temp = 0;
		ClientDlg->SetTimer(2,500,0);
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
	
			ClientDlg->m_ByteTotal+=dwLen;
			dwTemp = int(ClientDlg->m_ByteTotal*100.0/ClientDlg->m_dwFileSize);
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

		ClientDlg->KillTimer(2);
		ClientDlg->m_Progress.SetPos(0);
		ClientDlg->HideItem();
		
		
		if(ClientDlg->m_ByteTotal==ClientDlg->m_dwFileSize)
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


		ClientDlg->m_ByteTotal = 0;
		DWORD dwTemp  = 0;
		DWORD dwTemp1 = 0;
		CString strTemp;
		CString strTemp1;
		//�����ļ�  �������ļ�
		char Buffer[FILEMAX] = {0}; 
		ClientDlg->m_Temp = 0;
		ClientDlg->SetTimer(2,500,0);
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

			ClientDlg->m_ByteTotal+=dwLen;
			dwTemp = int(ClientDlg->m_ByteTotal*100.0/ClientDlg->m_dwOtherFileSize);
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

		ClientDlg->KillTimer(2);
		FileSocket.Close();	
		File.Close();
		ClientDlg->m_Progress.SetPos(0);
		ClientDlg->HideItem();
	
		if(ClientDlg->m_ByteTotal==ClientDlg->m_dwOtherFileSize)
		{
			ClientDlg->SendMsg(20,ClientDlg->m_strOtherName,"�ļ��������!");
		}
	
	}

	return 0;

}

void CClientDlg::OnDblclkListUser() 
{

	int Index = m_ListUser.GetCurSel();

	if (Index<0)
	{
		return;
	}

	CString strString;
	m_ListUser.GetText(Index,strString);


	m_ComBoUser.SetWindowText(strString);

	UpdateData(FALSE);


	
}

void CClientDlg::OnTimer(UINT nIDEvent) 
{


	switch(nIDEvent)
	{
	case 2:
		{
			CString strFileSize;
			strFileSize=KindChange(m_ByteTotal-m_Temp);//60 1000
			m_Temp=m_ByteTotal;
			strFileSize="�ٶ�:"+strFileSize+"/s";
			GetDlgItem(IDC_STATIC_SPEED)->SetWindowText(strFileSize);
			break;
		}
	}

	CDialog::OnTimer(nIDEvent);
}
