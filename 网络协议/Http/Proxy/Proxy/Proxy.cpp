#include <winsock2.h>
#include <windows.h>
#include <malloc.h>
#include <string.h>
#include <stdio.h>


#pragma comment(lib,"Ws2_32")

#define MAXBUFFERSIZE 1000


typedef struct _PROXYSOCKET_ 
{
    SOCKET  user_proxy;			             //���ػ�����PROXY �������socket
	SOCKET  proxy_server;		             //PROXY �������Զ��������socket
	BOOL    IsUser_ProxyClosed;              // ���ػ�����PROXY �����״̬
	BOOL    IsProxy_ServerClosed;            // PROXY �������Զ������״̬

}ProxySocket,*pProxySocket;




//�������ﶨ����һ���ṹ�����Զ�̷���������Ϣ

typedef struct _REMOTEADDR_
{
	char Address[256];        //Զ�̷���ĵ�ַ
	int  uPort ;              //������Զ�̷�������Ӷ˿�
	HANDLE User_SvrOK;        //���Ǹ��¼���������Զ�̷��������Ͼ�Ϊ�����ŷ�������
	pProxySocket pPros;       // ά��һ��SOCKET��ָ��

}RemoteAddr,*pRemoteAddr;




bool InitSocket();
bool ResolveInformation(char* Buffer,char* RemoteServerAddr,int* Port);


DWORD WINAPI UseProxy(LPVOID lpParma);

DWORD WINAPI ProxyToServer(LPVOID lpParam);


void main()
{
	if (!InitSocket())
	{
		printf("Initilize Socket Error\n");

		return;
	}

	//����һ�������׽���

	SOCKET sListen;

	sListen = socket(AF_INET,SOCK_STREAM,0);

	if (sListen==INVALID_SOCKET)
	{
		printf("Create Listen Error\n");

		return;
	}

	//��ʼ����������

	sockaddr_in LocalAddr;
	LocalAddr.sin_addr.S_un.S_addr = INADDR_ANY;
	LocalAddr.sin_family = AF_INET;
	LocalAddr.sin_port = htons(2356);          //�������ǿ����ṩ���û����û����趨


	//�󶨼����׽���

	int Ret = bind(sListen,(sockaddr*)&LocalAddr,sizeof(LocalAddr));

	if (Ret==SOCKET_ERROR)
	{
		printf("Bind Error\n");

		closesocket(sListen);
		return;
	}


	//����

	listen(sListen,5);


	//����һ��ͨ���׽���������ͻ��˽������ӵ��׽���
	
	SOCKET sClient;

	sockaddr_in RemoteClientAddr;
	int nLen = sizeof(RemoteClientAddr);
	
	while (1)
	{
		sClient = accept(sListen,(sockaddr*)&RemoteClientAddr,&nLen);


		//���ҽ��յ��ͻ��˵���������Ҫ����һ��������
		CreateThread(NULL,0,UseProxy,(LPVOID)sClient,0,NULL);

	
	}
}




DWORD WINAPI UseProxy(LPVOID lpParma)
{
	ProxySocket ProSockTemp;

	
	ProSockTemp.user_proxy = (SOCKET)lpParma;

	RemoteAddr RemoteAddrTemp;


	char Buffer[MAXBUFFERSIZE];


	if (ProSockTemp.user_proxy==INVALID_SOCKET)
	{
		return -1;
	}

	//���ܴӿͻ��˷������ĵ�һ�����ݰ�

	ProSockTemp.IsUser_ProxyClosed=FALSE;
	ProSockTemp.IsProxy_ServerClosed=TRUE;


	int Ret = recv(ProSockTemp.user_proxy,Buffer,MAXBUFFERSIZE,0);

	if (Ret==SOCKET_ERROR)
	{
		printf("Recv Error\n");

		closesocket(ProSockTemp.user_proxy);


		return -1;
	}


	//�����ǽ��յ�����ʱ����Ҫ�����ݰ����н���

	Buffer[Ret]='\0';


	//����
	printf("%s",Buffer);


	RemoteAddrTemp.pPros = &ProSockTemp;

	RemoteAddrTemp.User_SvrOK = CreateEvent(NULL,TRUE,FALSE,NULL);
	
	if (!ResolveInformation(Buffer,RemoteAddrTemp.Address,&RemoteAddrTemp.uPort))
	{

		printf("Resolve Error\n");
		
		closesocket(ProSockTemp.user_proxy);
		return -1;
	}


	//����һ���̸߳��߳��Ǵ��������Զ�̷������ͨ�ŵ��߳�

	HANDLE hThread = CreateThread(NULL,0,ProxyToServer,(LPVOID)&RemoteAddrTemp,0,NULL);

	WaitForSingleObject(RemoteAddrTemp.User_SvrOK ,INFINITE);    //����������ȴ��¼��Ĵ���


	while (ProSockTemp.IsProxy_ServerClosed==FALSE&&ProSockTemp.IsUser_ProxyClosed==FALSE)
	{
		Ret = send(ProSockTemp.proxy_server,Buffer,sizeof(Buffer),0);


		if (Ret==SOCKET_ERROR)
		{
			printf("SendError\n");
			
			if(ProSockTemp.IsProxy_ServerClosed==FALSE)
			{
				closesocket(ProSockTemp.proxy_server);
				
				ProSockTemp.IsProxy_ServerClosed=TRUE;
			}
			continue;
		}


		Ret=recv(ProSockTemp.user_proxy,Buffer,sizeof(Buffer),0);
		
		if(Ret==SOCKET_ERROR)
		{
			printf("Recv Error\n"); 
			if(ProSockTemp.IsUser_ProxyClosed==FALSE)
			{
				closesocket(ProSockTemp.user_proxy);
				ProSockTemp.IsUser_ProxyClosed=TRUE;
			}
			continue;
		}

		//����
		printf("%s",Buffer);
		
	}




	WaitForSingleObject(hThread,INFINITE);
	return 0;
}

DWORD WINAPI ProxyToServer(LPVOID lpParam)
{
	//���߳��еĲ����ͱ����˹���Զ�̷���������Ϣ

	pRemoteAddr pTemp = (pRemoteAddr)lpParam;

	int uPort = pTemp->uPort;

	int Ret = 0;

	char Buffer[MAXBUFFERSIZE] ={0};


	//����һ��Զ�����ӵ�������Ϣ

	sockaddr_in Remoteaddr_in ={0};

                     
	char RemoteIP[512]={0};
	strcpy(RemoteIP,pTemp->Address);
	Remoteaddr_in.sin_addr.S_un.S_addr = inet_addr(RemoteIP);
    Remoteaddr_in.sin_family = AF_INET;
	Remoteaddr_in.sin_port = htons(uPort); 
	
	
	
	
	
	//����һ�����ӵ��׽���

	SOCKET sConnect = socket(AF_INET,SOCK_STREAM,0);


	if (sConnect==INVALID_SOCKET)
	{
		printf("Create Connect Socket Error\n");


		return -1;
	}

	if (connect(sConnect,(sockaddr*)&Remoteaddr_in,sizeof(Remoteaddr_in))== SOCKET_ERROR) 
	{
		printf("Connect RemoteServer Error\n");
		
		pTemp->pPros->IsProxy_ServerClosed = TRUE;
		
		SetEvent(pTemp->User_SvrOK);    //֪ͨUserProxy�̴߳���	
		
		return -1;
	}

	//���ӳɹ�ʱ

	pTemp->pPros->proxy_server = sConnect;
	pTemp->pPros->IsProxy_ServerClosed = FALSE;
	SetEvent(pTemp->User_SvrOK); 

	while (!pTemp->pPros->IsProxy_ServerClosed &&!pTemp->pPros->IsUser_ProxyClosed)
	{
		Ret = recv(sConnect,Buffer,sizeof(Buffer),0);

		if (Ret==SOCKET_ERROR)
		{
			closesocket(sConnect);
			pTemp->pPros->IsProxy_ServerClosed = TRUE;

			break;
		}


		Ret = send(pTemp->pPros->user_proxy,Buffer,sizeof(Buffer),0);

		if (Ret == SOCKET_ERROR) 
		{
			
			closesocket(pTemp->pPros->user_proxy);
			pTemp->pPros->IsUser_ProxyClosed=TRUE;
			break;						
		}
	}





	return 0;
}



//�������ݰ�����

bool ResolveInformation(char* Buffer,char* RemoteServerAddr,int* Port)
{
	char Command[512] = {0};
	char Protocol[128]={0};
	char Temp[MAXBUFFERSIZE]={0};

	sscanf(Buffer,"%s%s%s",Command,Temp,Protocol);   //����һ�¿�����֪����



	// "GET http://127.0.0.1/ HTTP1.1"; 

	//�ڻ�����Temp�в���Http

	char* p = NULL;
	p = strstr(Temp,"http://");

	//����ָ��
	if (p)
	{
		p = p+strlen("http://");

		//����Ҫ������Զ�̷������ĵ�ַ

		for (int i=0;i<strlen(p);i++)
		{
			if (*(p+i)=='/')
			{
				break;
			}
		}                                 
		
		//�ҵ��ַ�/����Ҫ����ȥ��

		*(p+i) = 0;

		strcpy(RemoteServerAddr,p);   //127.0.0.1           // 1  accept client 127.0.0.1
                                                            // 2  RemoteSever.sin.sun.sin = inet_addr(127.0.0.1) //connect 
															//    RemoteServer.port = htons(80);
																				//AF_INET
		//ȥ��Զ�̵�����������

		p = strstr(Buffer,"http://");


		for (int j=0;j<i+strlen("http://");j++)
		{
			*(p+j) = ' '; 
		}


		
		p = Buffer;

		j = 0;

	
		for (i=0;i<strlen(Buffer);i++)
		{
			if (*(p+i)!=' ')
			{
					
			   Buffer[j] = *(p+i);
			   j++;
		
			}
		}

		Buffer[j]='\0';


		p = strstr(Buffer,"/");

		//����һ����ʱ������

		char Temp[512]={0};

		strcpy(Temp,p);

		int n = p-Buffer;

		Buffer[n] = ' ';

		Buffer[n+1]='\0';

		strcat(Buffer,Temp);

		strcat(Buffer,"\r\n\r\n");
		//����Զ�̵Ķ˿�

		*Port = 80;
		
	}

	else
	{
		return false;
	}


	return true;

}


bool InitSocket()
{
	WSADATA wsadata;
	if (WSAStartup(MAKEWORD(2,2),&wsadata)==0)
	{
		return true;
	}

	return false;
}
