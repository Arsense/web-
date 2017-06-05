#include <WINSOCK2.H>
#include <windows.h>
#include <malloc.h>
#include <string.h>
#include <stdio.h>

#pragma comment(lib,"Ws2_32")

//����ĵ�һ����Ա�����ǵ�һ����ͻ��˽������ӵ��׽���
//����ĵ�������Ա���������ʼ��������������ӵ��׽���
//����ĵ��ĸ���Ա������������ͻ��˽������ӵ��׽���
typedef struct _SERVICESOCKET_ 
{
	SOCKET TcpSocket;
	
	unsigned short ServiceListenPort;
	SOCKET Svctcp_server;
	SOCKET Svctcp_client;
	
}ServiceSocket, *pServiceSocket;


//�����ݽṹ�Ǽ�¼�ͻ���Ϣ��Ϊ�����ǿ��������Ϣ��ͻ��˽�������У���
typedef struct _USERINFOR_
{
	char UserName[20];
	char UserPass[20];
	_USERINFOR_* pNext;

}UserInfor, *pUserInfor;   


pUserInfor pHead = NULL;
pUserInfor pTail = NULL;

//ͨ������Ҫ��½��������Ҫ����ע����������ͽ����ǵĿͻ����������ʽ����������

void CreateList(char* NameBuffer,char* UserPass);



//��ʼ���׽��ֿ�
bool InitSocket();

DWORD WINAPI WorkThread(LPVOID lpParam);    //�����߳�

bool Identification(SOCKET s);

void ReceiveBuffer(SOCKET s,char* Buffer,int Len); 

void SendBuffer(SOCKET s,char* Buffer,int Len);


//ת���˿ں�

int ConvertPort(char* Buffer);

void GetRequest(ServiceSocket& ServiceSocketTemp);
void ReplyRequest(ServiceSocket& ServiceSocketTemp);

DWORD WINAPI TCPClientToServer(LPVOID lpParam);
DWORD WINAPI TCPServerToClient(LPVOID lpParam);

DWORD WINAPI TCPProxyThread(LPVOID lpParam);

char* g_Name = NULL;

char* g_Password = NULL;




int g_ServiceListenPort = 2000;

void main()
{
	char NameBuffer[20] = "shine";
	char UserPass[20] = "1234";

	CreateList(NameBuffer,UserPass);

	memset(NameBuffer,0,20);
	memset(UserPass,0,20);
	
	strcpy(NameBuffer,"wangxue");
	strcpy(UserPass,"1234");


	CreateList(NameBuffer,UserPass);
	
	
	
	
	if (!InitSocket())
	{
		printf("Init Socket Error\n");
		
		return;
	}

	//����һ�������׽���
	
	SOCKET sListen;
	
	
	sListen= socket(AF_INET,SOCK_STREAM,0);
	
	if (sListen==INVALID_SOCKET)
	{
		printf("Create Listen Socket Error\n");
		
		return;
	}
	
	//��ʼ������
	sockaddr_in ProxyAddr;
	
	ProxyAddr.sin_addr.S_un.S_addr = INADDR_ANY;
	ProxyAddr.sin_family = AF_INET;
	ProxyAddr.sin_port = htons(2356);
	
	//��
	int Ret = bind(sListen,(sockaddr*)&ProxyAddr,sizeof(ProxyAddr));
	
	if (Ret==SOCKET_ERROR)
	{
		printf("Bind Error\n");
		
		closesocket(sListen);
		
		
		return;
	}
	
	//����
	
	listen(sListen,5);
	
	//����һ����ͻ�ͨ�ŵ��׽���
	
	SOCKET sClient;
	
	
	sockaddr_in ClientAddr;
	
	int nLen = sizeof(ClientAddr);


	HANDLE hThread;
	//ѭ������
	while (1)
	{
		sClient = accept(sListen,(sockaddr*)&ClientAddr,&nLen);
		
		if (sClient==INVALID_SOCKET)
		{
			continue;
		}
		
		//�����߳�
		
		hThread = CreateThread(NULL,0,WorkThread,(LPVOID)sClient,0,NULL);
		
		
		if (hThread==NULL)
		{
			printf("Create Thread Error\n");
			
			
			break;
		}
		
		
	}
	
	
	
	
	WaitForSingleObject(hThread,INFINITE);
	
	closesocket(sListen);



}


DWORD WINAPI WorkThread(LPVOID lpParam)
{
	
	SOCKET sClient = (SOCKET)lpParam;
	
	
	ServiceSocket ServiceTemp;
	
	ServiceTemp.TcpSocket = sClient;

	
	if (Identification(ServiceTemp.TcpSocket))
	{
		
		GetRequest(ServiceTemp);
		
		ReplyRequest(ServiceTemp);
	}
	
	//�����ǵ���֤ͨ��ʱ����Ҫ��������������ͨ��
	
	
	
	return 0;
}



bool Identification(SOCKET s)
{
	
	//����Ҫ����������ܵ���һ�����ݰ��Ľ��ܲ��ҽ���У��
	
	char Buffer[1024]={0};    //���ǽ������ݴ�ŵĻ�����
	
	ReceiveBuffer(s,Buffer,2);   //������ȡǰ�����ֽ�
	
	int NumMethod = Buffer[1]; 
	
	memset(Buffer,1024,0);
	
	ReceiveBuffer(s,Buffer,NumMethod);    //����ȡ����һ���ֽ����жϷ���
	
	//����
	//printf("%d\n",NumMethod);
	
	
	int i = 0;
	for (i = 0;i<NumMethod;i++)
	{
		if (Buffer[i]==0x02)
		{
			//����
			
			//printf("OK");
			
			break;
		}
	}


	//����һ�����������ڷ���
	
	char Reply[2]={0};
	
	Reply[0] = 0x05;
	
	
	if (i==NumMethod)
	{
		Reply[1] = 0xff;
		
		return false;
	}
	
	else
	{
		Reply[1] = 0x02;
	}
	
	SendBuffer(s,Reply,2);    //���ǿ����ڿͻ��˽����ж�



	
	memset(Buffer,0,1024);
	//����Ҫ��������ܵڶ������ݰ��Ĳ�Ҫ�ж�ǰ�����ֽ�
	ReceiveBuffer(s,Buffer,2); 
	
	if (Buffer[0]!=0x01)
	{
		return false;
	}
	
	int ulen = (int)Buffer[1];     //�����������ݾ��������û������ĳ������ǽ��ж�̬�����ڴ�
	
	
	g_Name = (char*)malloc(sizeof(ulen));



	ReceiveBuffer(s,g_Name,ulen);
	
	
	//����
	g_Name[ulen]='\0';
	printf("User Come:%s\n",g_Name);
	
	
	//����Password
	memset(Buffer,0,1024);
	
	ReceiveBuffer(s,Buffer,1);
	
	ulen = (int)Buffer[0];
	
	
	g_Password = (char*)malloc(sizeof(ulen));
	
	ReceiveBuffer(s,g_Password,ulen);
	
	
	//����
	g_Password[ulen]='\0';
	printf("User Password:%s\n",g_Password);


	//���ϵĶ�ȡ���ݺ���ǰSocket5��Щ��һ��Ҫע����������޸�



	memset(Reply,0,2);
	Reply[0] = 0x05;
	
	//У��

	pUserInfor pTemp = pHead;
	while (pTemp!=NULL)
	{
		if (!strcmp(g_Name,pTemp->UserName) && !strcmp(g_Password,pTemp->UserPass))
		{
			
			
			//״̬Ϊ��Ϊ��֤�ɹ�
			
			Reply[1]=0x00;    
			SendBuffer(s,Reply,2);
			
			
			//��û�д���
			return true;
			
		}

		else
		{
			pTemp = pTemp->pNext;
		}
	}

	

		//����Ļ����ǽ������ݷ���
		Reply[1]=0x01;
		
		SendBuffer(s,Reply,2);

	
	
	return false;

}




void GetRequest(ServiceSocket& ServiceSocketTemp)
{
	ServiceSocketTemp.Svctcp_server = socket(AF_INET,SOCK_STREAM,0);
	
	if (ServiceSocketTemp.Svctcp_server==INVALID_SOCKET)
	{
		printf("Create Server Socket Error\n");
		
		return;
	}

	char Buffer[1024]={0};   //�������ݲ��ұ��浽�������� 
	
	char Reply[2]={0};
	
	ReceiveBuffer(ServiceSocketTemp.TcpSocket,Buffer,10);

	sockaddr_in ServerAddr;
	

	
	ServerAddr.sin_addr.s_addr =
		MAKELONG(MAKEWORD((Buffer[4]&0xff),(Buffer[5]&0xff)),
		MAKEWORD((Buffer[6]&0xff),(Buffer[7]&0xff)));
	
	
	int uPort = ConvertPort(Buffer);
	
	//����
	printf("%s\n",inet_ntoa(ServerAddr.sin_addr));
	
	printf("%d",uPort);


	ServerAddr.sin_family = AF_INET;
	ServerAddr.sin_port = ntohs(uPort); 

	//������������ý��õ����ʼ���������IP��ַ�Ͷ˿ڽ��м�¼��Ȼ������������ӳɹ��ͼ�¼������������Ӳ�ͨ���Ǿ�����ڴ���ɾ��������
	//һ������ָ�������ҾͲ��ж���


	if(connect(ServiceSocketTemp.Svctcp_server,(sockaddr*)&ServerAddr,sizeof(ServerAddr)) ==SOCKET_ERROR)
	{
		printf("Connect Error\n");
		return;
	}

	else
	{
		Reply[0] = 0x05;
		Reply[1] = 0x00;
		
	//  ����	
	//	send(ServiceSocketTemp.Svctcp_server,"Hello",strlen("Hello"),0);

		SendBuffer(ServiceSocketTemp.TcpSocket,Reply,2);
	}



	

}

void ReplyRequest(ServiceSocket& ServiceSocketTemp)
{
	char Reply[1024] ={0};
    Reply[0] = 0x05;
    Reply[1] = 0x00;
    Reply[2] = 0x00;
    Reply[3] = 0x01;
	
	
	g_ServiceListenPort++;
	
	ServiceSocketTemp.ServiceListenPort = g_ServiceListenPort;    //����Ķ˿����������û����ӵĶ˿�����ǳ���Ҫ

	sockaddr_in addr;
	
	
	addr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");    //�����IP��ַ�Ǵ���˵�IP��ַ
	
	Reply[4] = addr.sin_addr.S_un.S_un_b.s_b1;
	Reply[5] = addr.sin_addr.S_un.S_un_b.s_b2;
	Reply[6] = addr.sin_addr.S_un.S_un_b.s_b3;
	Reply[7] = addr.sin_addr.S_un.S_un_b.s_b4;
	
	
	Reply[8] = ServiceSocketTemp.ServiceListenPort/256;
    Reply[9] = ServiceSocketTemp.ServiceListenPort%256;
	 
	
	
	//����һ�����ݰ���ӭ���������ݰ�
	
	SendBuffer(ServiceSocketTemp.TcpSocket,Reply,10);      //���͸��ͻ��ÿͻ����¸�������


	ServiceSocketTemp.Svctcp_client = socket(AF_INET,SOCK_STREAM,0);
	
	if (ServiceSocketTemp.Svctcp_client==INVALID_SOCKET)
	{
		printf("Create Client Socket Error\n");
		
		return;
	}


	sockaddr_in ServerAddr;
	
	memset(&ServerAddr,0,sizeof(sockaddr_in));
	
	ServerAddr.sin_family = AF_INET;
	ServerAddr.sin_port   = htons(ServiceSocketTemp.ServiceListenPort);
	ServerAddr.sin_addr.S_un.S_addr =INADDR_ANY;
	
	if (bind(ServiceSocketTemp.Svctcp_client,(sockaddr*)&ServerAddr,sizeof(ServerAddr))==SOCKET_ERROR)
	{
		printf("Bind Error\n");
		return;
	}
	if (listen(ServiceSocketTemp.Svctcp_client,5)==SOCKET_ERROR)
	{
		printf("Listen Error\n");
		return;
    }

	//�����ǵļ����׽���ֱ��ת����ͨ���׽���
	sockaddr_in RemoteAddr;
	int nlen = sizeof(sockaddr_in);
	ServiceSocketTemp.Svctcp_client = accept(ServiceSocketTemp.Svctcp_client,(sockaddr*)&RemoteAddr,&nlen);
	
	HANDLE hThread = CreateThread(NULL,0,TCPProxyThread,(LPVOID)&ServiceSocketTemp, 0, 0);
	
	if (hThread == NULL)
	{
		printf("TCPProxyThread Error\n");
		return;
	}
	
	
	
	SuspendThread(GetCurrentThread());



}



DWORD WINAPI TCPProxyThread(LPVOID lpParam)
{
	pServiceSocket pTemp = (pServiceSocket)lpParam;
	
	ServiceSocket Temp;
	
	memcpy(&Temp,pTemp,sizeof(ServiceSocket));
	
	
	HANDLE hThread[2];
	
	
	hThread[0] = CreateThread(NULL,0,TCPClientToServer,(LPVOID)&Temp,0,0);
	
	
	if (hThread[0]==NULL)
	{
		printf("TCPClientToServer Thread Error\n");
		
		return -1;
	}
	
	
	hThread[1] = CreateThread(NULL,0,TCPServerToClient,(LPVOID)&Temp,0,0);
	
	
	if (hThread[1]==NULL)
	{
		printf("TCPServerToClient Thread Error\n");
		
		return -1;
	}
	
	
	WaitForMultipleObjects(2,hThread,false,INFINITE);
	
}




DWORD WINAPI TCPClientToServer(LPVOID lpParam) 
{
    //���̺߳��������ǽ��ͻ����յ������ݽ���ת���������
	
	
	pServiceSocket pTemp = (pServiceSocket)lpParam;
	
    char Buffer[4096];
    int count;
    while (1)
	{
        count = recv(pTemp->Svctcp_client,Buffer,4096,0);
		
		if (count==-1)
		{			
			continue;
		}
        
		SendBuffer(pTemp->Svctcp_server,Buffer,count);
    }
}



DWORD WINAPI TCPServerToClient(LPVOID lpParam) 
{
	//���̺߳��������ǽ�������յ������ݽ���ת�����ͻ���
	
	pServiceSocket pTemp = (pServiceSocket)lpParam;
	
	char Buffer[4096];
    int count;
	
	
	while (1)
	{
		count = recv(pTemp->Svctcp_server,Buffer,4096,0);
		
		if (count==-1)
		{
			continue;
		}
        
		SendBuffer(pTemp->Svctcp_client,Buffer,count);
	}
	
	
}



//��������
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//����ĵ����������ǽ��ܵ��ֽڳ���
void ReceiveBuffer(SOCKET s,char* Buffer,int Len)
{
	if (!Len)
	{
		return;   //������ǽ��ܵĳ���Ϊ0ʱ���Ǿ��˳�����
	}
	
	int Temp = Len;
	
	int Ret = 0;
	while (1)
	{
		
		Temp = Temp-recv(s,Buffer+(Len-Temp),Len,0);
		
		if (Temp==0)
		{
			break;
		}
	}
}


void SendBuffer(SOCKET s,char* Buffer,int Len)
{
	if (!Len)
	{
		return;
	}
	
	int Temp = Len;
	
	while (1)
	{
		Temp = Temp-send(s,Buffer+(Len-Temp),Len,0);
		
		if (Temp==0)
		{
			break;
		}
	}
}


int ConvertPort(char* Buffer)
{
	char a1 = Buffer[8];
	
	char a2 = Buffer[9];
	
	int b1 = a1;
	
	int b2 = a2;
	
	if (b1==0)
	{
		return b2;
	}
	
	
	else
	{
		b2<<=8;
		
		b1+=b2;
		
		return b1;
	}
	
}

void CreateList(char* NameBuffer,char* UserPass)
{
	pUserInfor pTemp;

	pTemp = (pUserInfor)malloc(sizeof(UserInfor));

	if (pTemp==NULL)
	{
		printf("No Memory\n");
	}

	else
	{
		strcpy(pTemp->UserName,NameBuffer);

		strcpy(pTemp->UserPass,UserPass);

		pTemp->pNext = NULL;
		
		if (pHead==NULL)
		{
			pHead = pTail = pTemp;
		}

		else
		{
			pTail->pNext = pTemp;

			pTemp = pTail->pNext;
		}
	}
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