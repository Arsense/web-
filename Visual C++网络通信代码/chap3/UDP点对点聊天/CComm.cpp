

#include "CComm.h"


CComm::CComm()
{
	//���캯��
	ListenSocket = INVALID_SOCKET;	// ��ʼ����ΪINVALID_SOCKET
	
	#ifdef _WIN32	// �����win32ϵͳ
		WORD VersionRequested = MAKEWORD(1,1);
		WSADATA wsaData;
		
		WSAStartup(VersionRequested, &wsaData);	// ����winsock����
		if ( wsaData.wVersion != VersionRequested )
		{
			printf("Wrong version or WinSock not loaded\n");
			fflush(0);	
		}
	#endif
}

//��������
CComm::~CComm()
{
	if ( ListenSocket != INVALID_SOCKET )
		closesocket( ListenSocket );	// ����Ѿ���������ر�

	#ifdef _WIN32	// ����WSACleanup
		WSACleanup();
	#endif
}

bool CComm::SendMsg( char *Msg, int Len, char *host, short port )
{
	signed int Sent;
	hostent *hostdata;
	if ( atoi(host) )	// �Ƿ�IP��ַΪ��׼��ʽ
	{
		u_long ip = inet_addr( host );
		hostdata = gethostbyaddr( (char *)&ip, sizeof(ip), PF_INET );
	}
	else	// ����������ǻ�����
	{
		hostdata = gethostbyname( host );
	}

	if ( !hostdata )
	{
		printf("��û���������\n");
		fflush(0);
		return false;
	}

	sockaddr_in dest;	// ����Ŀ���ַ
	dest.sin_family = PF_INET;
	dest.sin_addr = *(in_addr *)(hostdata->h_addr_list[0]);
	dest.sin_port = htons( port );
	printf("��Ϣ�Ѿ������͵����� %s �˿�Ϊ %i\n", inet_ntoa(dest.sin_addr), ntohs(dest.sin_port));
	//���ݷ���
	Sent = sendto(ListenSocket, Msg, Len, 0, (sockaddr *)&dest, sizeof(sockaddr_in));
	
	if ( Sent != Len )
	{
		printf("������UDP��Ϣ\n");
		fflush(0);
		return false;
	}
	
	return true;
}

void *CComm::ListenThread( void *data )
{
	char buf[4096];
	CComm *Comm = (CComm *)data;
	int len = sizeof(Comm->client);
	while(1)	// һֱѭ��
	{
		//��������
		int result = recvfrom( Comm->ListenSocket, buf, sizeof(buf)-1, 0, (sockaddr *)&Comm->client, (socklen_t *)&len);
		if ( result > 0 )
		{
			buf[result] = 0;
			printf("Message received from host %s port %i\n", inet_ntoa(Comm->client.sin_addr), ntohs(Comm->client.sin_port));
			printf(">> %s", buf);
			fflush(0);
		}	
	}	
}


//��ַ�󶨣�ע����UDPЭ���У�����Ҫlisten�����ﺯ��listenֻ�ǰ�һ���˿�
bool CComm::Listen( int PortNum )
{
	ListenSocket = socket(PF_INET, SOCK_DGRAM, 0);
	if ( ListenSocket == INVALID_SOCKET )
	{
		printf("Error: socket����ʧ��\n");
		fflush(0);
		return false;
	}

	srv.sin_family = PF_INET;
	srv.sin_addr.s_addr = htonl( INADDR_ANY );	// �κε�ַ
	srv.sin_port = htons( PortNum );

	if ( bind( ListenSocket, (struct sockaddr *)&srv, sizeof(srv)) != 0 )
	{
		printf("Error: ��ʧ��\n");
		fflush(0);
		closesocket( ListenSocket );
		return false;
	}

	int ThreadID;	// �߳�id
	
		DWORD thread;
		//����createthread�����߳�
		ThreadID = (int)CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)(CComm::ListenThread), (void *)this, 0, &thread);
		ThreadID = ThreadID ? 0 : 1;	// ����ɹ����򷵻�Ϊ0

	if(ThreadID)	// ThreadID�����Ϊ0�����̴߳���ʧ��
	{
		printf("�̴߳���ʧ��\n");
		return false;
	}
	else
		return true;
}
