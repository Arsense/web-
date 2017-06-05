#include <WINSOCK2.H>
#include <windows.h>
#include <malloc.h>
#include <string.h>
#include <stdio.h>

#pragma comment(lib,"Ws2_32")


enum CMDTYPE 
{
	CONNECT,BIND,UDP
};


typedef struct _SERVICESOCKET_ 
{
	SOCKET TcpSocket;
	CMDTYPE CmdType;


    unsigned short ServiceListenPort;

	SOCKET Svctcp_server;
	SOCKET Svctcp_client;

}ServiceSocket, *pServiceSocket;




bool InitSocket();

bool Identification(SOCKET s);



void ReceiveBuffer(SOCKET s,char* Buffer,int Len);
void SendBuffer(SOCKET s,char* Buffer,int Len);


//������
void GetRequest(ServiceSocket& ServiceSocketTemp);

//Ӧ����
void ReplyRequest(ServiceSocket& ServiceSocketTemp);

bool CreateSocket(SOCKET& s);


void GetHostIP(char* Buffer);

in_addr GetName(char* Name);
DWORD WINAPI WorkThread(LPVOID lpParam);
DWORD WINAPI TCPProxyThread(LPVOID lpParam);

DWORD WINAPI TCPClientToServer(LPVOID lpParam);
DWORD WINAPI TCPServerToClient(LPVOID lpParam) ;

char* g_Name= NULL;
char* g_Password = NULL;

int g_ServiceListenPort = 2000;

//������������У������������
char User[100]="shine";          
char Password[100]="2356"; 

void main()
{
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

		    //printf("%d\n",Buffer[i]-0x30);
			
			break;
		}
	}

	//��������������ܵ�һ�������֤��һ�����ݰ�ʱ���Ǳ�����ͻ�һ���ظ���

	//���ǵ�һ���ظ���

	
	/*
		+----+----------+
		|VER | NMETHODS |  
		+----+----------+
		| 1  |    1     | 
		+----+----------+
	*/


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


	
	//�������Ҫ�ú����һ��
	ReceiveBuffer(s,Reply,2);         //����Ҫ�������������е��ڴ����
    //�������Ҫ�ú����һ��




	memset(Reply,0,2);
	memset(Buffer,0,1024);
	//����Ҫ��������ܵڶ������ݰ��Ĳ�Ҫ�ж�ǰ�����ֽ�
	ReceiveBuffer(s,Buffer,2); 

	if (Buffer[0]!=0x01)
	{
		return false;
	}

	int ulen = Buffer[1]-0x30;     //�����������ݾ��������û������ĳ������ǽ��ж�̬�����ڴ�


	g_Name = (char*)malloc(sizeof(ulen));

	//��������
	ReceiveBuffer(s,g_Name,ulen);


	//����
	g_Name[ulen]='\0';
	printf("User Come:%s\n",g_Name);


	//����Password
	memset(Buffer,0,1024);
	
	ReceiveBuffer(s,Buffer,1);
	
	ulen = Buffer[0]-0x30;


	g_Password = (char*)malloc(sizeof(ulen));
	
	//��������
	ReceiveBuffer(s,g_Password,ulen);
	
	
	//����
	g_Password[ulen]='\0';
	printf("User Password:%s\n",g_Password);



	//�����ǽ��ܵ��������������еڶ��ε����ݰ��Ļظ�


	/*
	+----+--------+
	|VER | STATUS |
	+----+--------+
	| 1  |   1    |
	+----+--------+
	*/

	memset(Reply,0,2);
	Reply[0] = 0x05;

	//У��
	if (!strcmp(g_Name,User) && !strcmp(g_Password,Password))
	{
	

		//״̬Ϊ��Ϊ��֤�ɹ�
		
		Reply[1]=0x00;    
		SendBuffer(s,Reply,2);


		//��û�д���
		return true;

	}

	else
	{
		//����Ļ����ǽ������ݷ���
		Reply[1]=0x01;

		SendBuffer(s,Reply,2);
	}


	return false;
}




void GetRequest(ServiceSocket& ServiceSocketTemp)
{
	//�������ݰ�
/*
+----+-----+-------+------+----------+----------+
|VER | CMD |  RSV  | ATYP | DST.ADDR | DST.gListenSocket |
+----+-----+-------+------+----------+----------+
| 1  |  1  | X'00' |  1   | Variable |    2     |
+----+-----+-------+------+----------+----------+
    */


	char Buffer[1024]={0};


	ReceiveBuffer(ServiceSocketTemp.TcpSocket,Buffer,10);


	//��������ý��а汾��У���Ҿ������ﲻ������

	
	printf("The COMMAND:\n");

    switch (Buffer[1])
	{
	case 0x01:
		{
			printf("Connect\n");
			ServiceSocketTemp.CmdType = CONNECT;
			break;
		}
			
	case 0x02:
		{
			printf("Bind\n");
			ServiceSocketTemp.CmdType = BIND;
			break;
		}
			
	case 0x03:
		{
			printf("UDP ASSOCIATE\n");
			ServiceSocketTemp.CmdType = UDP;
			break;
		}
	
	default:
		{
			break;
		}

    }


	if (Buffer[3]==0x01)              //��������û�й���Ĵ���
	{
		if(ServiceSocketTemp.CmdType == UDP)
		{
			
        }
        else if(ServiceSocketTemp.CmdType == CONNECT)
		{
			
			//�������ݰ��д��ڵ�Ŀ�ĵ�ַ
			//����
			printf("The Destination IP :  ");

			printf("%d.%d.%d.%d    Port :",Buffer[4]&0xff,Buffer[5]&0xff,Buffer[6]&0xff,Buffer[7]&0xff);


			int uPort = ((int)Buffer[8])*256+(unsigned char)Buffer[9];

			printf("%d",uPort);



			if (!CreateSocket(ServiceSocketTemp.Svctcp_server))
			{
				return;
			}

			//����Զ�̷���˵�������Ϣ
			sockaddr_in RemoteServerAddr;

			RemoteServerAddr.sin_family = AF_INET;
			RemoteServerAddr.sin_port = ntohs(uPort);


			RemoteServerAddr.sin_addr.s_addr =
				MAKELONG(MAKEWORD((Buffer[4]&0xff),(Buffer[5]&0xff)),
                      MAKEWORD((Buffer[6]&0xff),(Buffer[7]&0xff)));



			if(connect(ServiceSocketTemp.Svctcp_server,(sockaddr*)&RemoteServerAddr,sizeof(RemoteServerAddr)) ==SOCKET_ERROR)
			{
				printf("Connect Error\n");
				return;
			}
			

			//����

		//	send(ServiceSocketTemp.Svctcp_server,"Hello",strlen("Hello"),0);

		}
		else if (ServiceSocketTemp.CmdType == BIND)
		{
			
			//û��������
		}
	}
}


//Ӧ������
void ReplyRequest(ServiceSocket& ServiceSocketTemp)
{
/*
+----+-----+-------+------+----------+----------+
|VER | REP |  RSV  | ATYP | BND.ADDR | BND.gListenSocket |
+----+-----+-------+------+----------+----------+
| 1  |  1  | X'00' |  1   | Variable |    2     |
+----+-----+-------+------+----------+----------+
    */


	char Reply[1024] ={0};
    Reply[0] = 0x05;
    Reply[1] = 0x00;
    Reply[2] = 0x00;
    Reply[3] = 0x01;


	g_ServiceListenPort++;

	ServiceSocketTemp.ServiceListenPort = g_ServiceListenPort;

	//GetHostIP(Reply+4);   ���øú�������õ�����


	sockaddr_in addr;

	
	addr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");

	Reply[4] = addr.sin_addr.S_un.S_un_b.s_b1;
	Reply[5] = addr.sin_addr.S_un.S_un_b.s_b2;
	Reply[6] = addr.sin_addr.S_un.S_un_b.s_b3;
	Reply[7] = addr.sin_addr.S_un.S_un_b.s_b4;


	Reply[8] = ServiceSocketTemp.ServiceListenPort/256;
    Reply[9] = ServiceSocketTemp.ServiceListenPort%256;


	//����һ�����ݰ���ӭ���������ݰ�

	SendBuffer(ServiceSocketTemp.TcpSocket,Reply,10);


	if (ServiceSocketTemp.CmdType == CONNECT)
	{
        CreateSocket(ServiceSocketTemp.Svctcp_client);
       
		sockaddr_in ServerAddr;
        
		memset(&ServerAddr,0x00,sizeof(sockaddr_in));
        
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

bool CreateSocket(SOCKET& s)
{
	s = socket(AF_INET,SOCK_STREAM,0);

	if (s==INVALID_SOCKET)
	{
		printf("Create Server Socket Error\n");

		return false;
	}


	return true;


}

void GetHostIP(char* Buffer)
{
	int Ret = 0;
    char HostName[100];
    Ret = gethostname(HostName,1024);
    
	if (Ret != 0)
	{
		printf("%s",HostName);
        
		return;
    }

	in_addr HostAdd = GetName(HostName);
    Buffer[0] = HostAdd.S_un.S_un_b.s_b1;
    Buffer[1] = HostAdd.S_un.S_un_b.s_b2;
    Buffer[2] = HostAdd.S_un.S_un_b.s_b3;
    Buffer[3] = HostAdd.S_un.S_un_b.s_b4;
}


in_addr GetName(char* Name)
{
    hostent* he = gethostbyname(Name);
    
	return *((in_addr*)he->h_addr);
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
