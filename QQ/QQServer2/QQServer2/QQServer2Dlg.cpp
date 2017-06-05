
// QQServer2Dlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "QQServer2.h"
#include "QQServer2Dlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define  WM_SHOWTASK   WM_USER+1
#define  WM_SERVER_ACCEPT  WM_USER+2
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


// CQQServer2Dlg �Ի���




CQQServer2Dlg::CQQServer2Dlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CQQServer2Dlg::IDD, pParent)
{
	m_Timer = _T("");
	m_People = 0;
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CQQServer2Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_PEOPLE, m_strPeople);
	DDX_Control(pDX, IDC_STATIC_HOST, m_strHost);
	DDX_Text(pDX, IDC_EDIT_SHOW, m_strShow);
}

BEGIN_MESSAGE_MAP(CQQServer2Dlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_MESSAGE(WM_SHOWTASK,OnShowTask)
	ON_MESSAGE(WM_SERVER_ACCEPT,OnServerAccept)
	ON_WM_DESTROY()
	ON_COMMAND(ID_EXIT,OnDlgExit)
	ON_COMMAND(ID_SHOW,OnDlgShow)
	ON_COMMAND(ID_HIDE,OnDlgHide)
	//ON_BN_CLICKED(IDC_STATIC_HOST, OnStaticHost)
	//ON_BN_CLICKED(IDC_STATIC_PEOPLE, OnStaticPeople)
	//ON_EN_CHANGE(IDC_EDIT_SHOW, OnChangeEditShow)
END_MESSAGE_MAP()


// CQQServer2Dlg ��Ϣ�������

BOOL CQQServer2Dlg::OnInitDialog()
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


	InitNotify();
	SetWindowText(L"������"); //�Ѵ����������QQServer2�ĳɷ�����
	//1 ����Socket��
	InitSocket();  //2 ��ʼ��Socket��



	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

void CQQServer2Dlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CQQServer2Dlg::OnPaint()
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
HCURSOR CQQServer2Dlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

VOID CQQServer2Dlg::InitNotify()
{
	m_nid.cbSize = sizeof(NOTIFYICONDATA);
	m_nid.hWnd = m_hWnd;
	m_nid.uID = IDI_ICON_NOTIFY; 
	m_nid.uFlags = NIF_ICON|NIF_MESSAGE|NIF_TIP; 
	m_nid.uCallbackMessage = WM_SHOWTASK;   //�Զ�����Ϣ   �������ϴ�����궯��


	m_nid.hIcon = LoadIcon(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDI_ICON_NOTIFY)); 
	strcpy(m_nid.szTip,"����");//������������ʱ������ʾ������ 
	Shell_NotifyIconA(NIM_ADD,&m_nid);//�����������ͼ�� 


}
VOID CQQServer2Dlg::InitSocket()
{
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2,2),&wsaData);

	GetHostInfor();  


	//���������׽���

	m_sListen = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);  

	if(m_sListen==INVALID_SOCKET)
	{
		MessageBox(L"Create SOCKET Failed");
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
		MessageBox(L"Bind Failed");
		WSACleanup(); 
		closesocket(m_sListen);
		return;
	}
	//����
	if(listen(m_sListen,20)==SOCKET_ERROR)
	{
		MessageBox(L"Listen Failed");
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



VOID CQQServer2Dlg::GetHostInfor()
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
		strTemp +=' ';

		strTemp+="IP��ַ:";
		strTemp+=psz;

		m_strHost.SetWindowText(strTemp);



		CString strPeople;
		strPeople.Format(L"%d",m_People);
		m_strPeople.SetWindowText(strPeople);
	}
}
//��Щ��
LRESULT CQQServer2Dlg::OnShowTask(WPARAM wParam,LPARAM lParam)
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
			Menu.AppendMenu(MF_STRING, ID_SHOW,L"��ʾ");//һ�ֿ�����ָ���Ĳ˵���������ʽ�˵����Ӳ˵����ݲ˵���ĩβ׷��һ���²˵���ĺ�����
			Menu.AppendMenu(MF_STRING, ID_HIDE,L"����");	
			Menu.AppendMenu(MF_STRING, ID_EXIT,L"�˳�");	

			CPoint Pt;
			GetCursorPos(&Pt);         //�õ����λ��
			Menu.TrackPopupMenu(TPM_RIGHTBUTTON, Pt.x, Pt.y, this);
			Menu.DestroyMenu();        //������Դ

			break;
		}
	}

	return 0;
}

//��ʾ���ͻ������ߵ�ʱ�䣬������IP
VOID CQQServer2Dlg::OnlineOrOutline(int iIndex,CString YesOrNo)
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
VOID CQQServer2Dlg::OnClose(WPARAM wParam)
{

	int i = 0;
	for (i=0;i<MAX;i++)
	{
		if (m_sClient[i]==wParam)      //�������ǵڼ����û�
		{
			break;
		}
	}

	if (i==MAX)    //�����������û��������˳�
	{
		return;
	}

	OnlineOrOutline(i,L"����");

	m_People--;     //�û�����1

	CString strPeople;
	strPeople.Format(L"%d",m_People);
	m_strPeople.SetWindowText(strPeople);



	SENDINFOR  Data;
	int j = 0;
	for (j=0;j<MAX;j++)
	{
		if (m_sClient[j]!=INVALID_SOCKET&&j!=i)
		{
			Data.iType = -1;

			CString strMsg;
			GetCurrentTime();        //�����������ΰ�ʱ��Ū��strMsg�ϵģ�
			strMsg+=m_Timer.LockBuffer();
			strMsg+=m_PeopleInfor[i].strName;
			strMsg+="�˳��˸�������";                 //

			strcpy(Data.szName,(char*)m_PeopleInfor[i].strName.LockBuffer());
			strcpy(Data.szMsg,( char*)strMsg.LockBuffer()); 
			send(m_sClient[j],(char*)&Data,sizeof(Data),0);
		}
	}


	//����˳��û�����Դ
	closesocket(m_sClient[i]);
	m_sClient[i] = INVALID_SOCKET;

	m_PeopleInfor[i].strIp = "";
	m_PeopleInfor[i].strName = "";


}

void CQQServer2Dlg::OnDlgExit()
{

	SendMessage(WM_CLOSE,NULL,NULL);
}

void CQQServer2Dlg::OnDlgShow()
{
	::ShowWindow(this->m_hWnd,SW_NORMAL);
}

void CQQServer2Dlg::OnDlgHide()
{
	ShowWindow(SW_HIDE);
}
void CQQServer2Dlg::OnDestroy() 
{
	CDialog::OnDestroy();

	Shell_NotifyIconA(NIM_DELETE,&m_nid);
}

//FD_ACCEPT  FD_CLOSE   FD_READ
//�ж��û���������
LRESULT CQQServer2Dlg::OnServerAccept(WPARAM wParam,LPARAM lParam)
{

	//WSAGETSELECTEVENT == LOWORD(lParam)
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
VOID CQQServer2Dlg::OnRecive(WPARAM wParam)
{
	SENDINFOR  Data = {0};

	int i = 0;
	for (i=0;i<MAX;i++)
	{
		if (m_sClient[i]==wParam)  //��������ǰ�û��ǵڼ���
		{
			break;
		}
	}

	if (i==MAX) 
	{
		return;
	}

	int iRet = recv(m_sClient[i],(char*)&Data,sizeof(Data),0);   //�����û���Ϣ

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
					memcmp(m_PeopleInfor[j].strName.LockBuffer(),Data.szName,sizeof(PEOPLEINFOR))==0)  //LockBuffer�������ڴ����м��� ,ʹ���ڴ治�ܱ�����
				{
					send(m_sClient[j],(char*)&Data,sizeof(Data),0);      //�ش����û���Ϣ
				}
			}

			break;
		}
	case 7:    //6.7��ɶ����
		{

			int j = 0;
			for (j=0;j<MAX;j++)
			{
				if (m_sClient[j]!=INVALID_SOCKET&&
					memcpy(m_PeopleInfor[j].strName.LockBuffer(),Data.szName,sizeof(PEOPLEINFOR))==0)
				{
					send(m_sClient[j],(char*)&Data,sizeof(Data),0);
				}
			}



			break;
		}
	case 5:    //11 -12   
		//�����˽��
		{
			//11--->12
			//5 12 11|1.txt|5|
			//5 12 192.168.0.100|11|1.txt|5|    --->12

			CString strTemp;	
			strTemp+=m_PeopleInfor[i].strIp;
			strTemp+="|";
			strTemp+=Data.szMsg;

			memcpy(Data.szMsg,strTemp.LockBuffer(),sizeof(PEOPLEINFOR)); //�����û�IP �ͻش���Ϣ


			int j = 0;
			for (j=0;j<MAX;j++)
			{
				if (m_sClient[j]!=INVALID_SOCKET&&
					memcpy(m_PeopleInfor[j].strName.LockBuffer(),Data.szName,sizeof(PEOPLEINFOR))==0)
				{
					send(m_sClient[j],(char*)&Data,sizeof(Data),0);
				}
			}

			break;
		}

	case 120:   //˽��
		{

			int j = 0;
			for (j=0;j<MAX;j++)
			{
				if (m_sClient[j]!=INVALID_SOCKET&&
					memcmp(m_PeopleInfor[j].strName.LockBuffer(),Data.szName,sizeof(PEOPLEINFOR))==0)
				{
					send(m_sClient[j],(char*)&Data,sizeof(Data),0);
				}
			}


			break;
		}
	case 110:   //Ⱥ��
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
				if (memcmp(m_PeopleInfor[j].strName,Data.szName,sizeof(PEOPLEINFOR))==0
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
			OnlineOrOutline(i,L"����");


			//ˢ���û�����
			//1���ϵ�֪ͨ�µ��û�(�Լ��õ��Լ�)
			int j = 0;
			for (j=0;j<MAX;j++)
			{
				if (m_sClient[j]!=INVALID_SOCKET)
				{
					CString Temp;



					Data.iType = 0;
					memcpy(Data.szName,m_PeopleInfor[i].strName,sizeof(PEOPLEINFOR)); 
					//�������LockBuffer ���Ƿ�ֹ������Ϣ��¼��ʱ��������ݱ������˸���
					Temp +="\r\n";
					GetCurrentTime();
					Temp+=m_Timer.LockBuffer();
					Temp+="ϵͳ��Ϣ:\r\n  ";
					Temp+=m_PeopleInfor[i].strName;
					Temp+="����������";

					strcpy(Data.szMsg,(char*)Temp.LockBuffer());   


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
					memcpy(Data.szName,m_PeopleInfor[j].strName,sizeof(PEOPLEINFOR)); 
					send(m_sClient[i],(char*)&Data,sizeof(Data),0);
				}
			}

			break;
		}
	}
}
VOID CQQServer2Dlg::OnAccept()
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

	m_sClient[i] = accept(m_sListen,(sockaddr*)&ClientAddr,&nLen); //������Ϣ



	LPCSTR strIP = inet_ntoa(*(struct in_addr*)&ClientAddr.sin_addr);
	m_PeopleInfor[i].strIp = strIP;    //�õ������û���IP�������û����ݽṹ����


	m_People++;   //�û�����1

	CString strPeople;
	strPeople.Format(L"%d",m_People);
	m_strPeople.SetWindowText(strPeople);

	UpdateData(FALSE);
}
VOID CQQServer2Dlg::GetCurrentTime()
{

	SYSTEMTIME st = {0};
	GetLocalTime(&st);    //�õ���ǰʱ��
	int j = 0;
	j  = sprintf((char*)m_Timer.LockBuffer(),"%d��",st.wYear);
	j += sprintf((char*)m_Timer.LockBuffer()+j,"%d��",st.wMonth);
	j += sprintf((char*)m_Timer.LockBuffer()+j,"%d��",st.wDay);
	j += sprintf((char*)m_Timer.LockBuffer()+j,"%dʱ",st.wHour);

	if (st.wMinute<10)  //Ϊʲô��С��10��
	{
		int i = 0;
		j += sprintf((char*)m_Timer.LockBuffer()+j,"%d",i); 
	}
	j += sprintf((char*)m_Timer.LockBuffer()+j,"%d����",st.wMinute);

	if (st.wSecond<10) 
	{
		int i = 0;
		j += sprintf((char*)m_Timer.LockBuffer()+j,"%d",i); 
	}
	j += sprintf((char*)m_Timer.LockBuffer()+j,"%d��",st.wSecond);


}
void CQQServer2Dlg::OnStaticHost() 
{
	// TODO: Add your control notification handler code here

}

void CQQServer2Dlg::OnStaticPeople() 
{
	// TODO: Add your control notification handler code here

}

void CQQServer2Dlg::OnChangeEditShow() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO: Add your control notification handler code here

}
