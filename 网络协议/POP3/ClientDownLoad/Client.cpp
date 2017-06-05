#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <iostream>

using namespace std;

#pragma comment(lib,"Ws2_32.lib")




//�ó�����Ϊ�˺����������ĵ�ͼ��������ǲ�ȡ�˴���ķ�������ʹ����Socket5


//ȫ�ֱ���
int ServerPort =110;  //POP3�ʼ��������Ķ˿ں�


SOCKET sReal;



//��ʼ��socket��
bool InitSocket();


//�û���½����
bool Login(char* ServerName,char* UserName,int UserPassWord,SOCKET& s,char* ProxyServerName,char* ProxyUserName,
		   int ProxyUserPassWord,int ProxyPort);

//�����û�Ŀ¼����
void CreateUserDirectory(char* UserName);


//��ַת������
unsigned long ConvertServerNameToLongServerAddr(char* ServerName);


//����������������
bool Socket5Start(char* ProxyServerName,int ProxyPort,char* ProxyUserName,int ProxyPassWord,unsigned long ServerAddr,int sPort,
				  SOCKET& s);




//����ļ�����
void MailSTAT(SOCKET& s);  //�ú�����ͨ�������ͱ��STAT����ȡ���û����ż���Ϣ

//��ѯ�ļ���Ϣ����ָ��鿴�ĵ�

void MailLIST(SOCKET& s,char a);

void MailDownLoad(SOCKET& s, char* srt); //�ļ�����




//�����߳�
DWORD WINAPI WorkThread(LPVOID lpParam);

DWORD WINAPI RecvProc(LPVOID lpParam);
DWORD WINAPI SendProc(LPVOID lpParam);
void main()
{
	if (!InitSocket())
	{
		printf("Initlize Socket Error\n");
		return;
	}

	//�ʼ���������Ϣ
	char ServerName[20]={0};




	printf("Input Server Name\n");

	scanf("%s",ServerName);

	printf("Input User Name\n");

	char UserName[20] = {0};

	scanf("%s",UserName);
	
	printf("Input User PassWord\n");

	int UserPassWord = 0;

	scanf("%d",&UserPassWord);


	//�ʼ����ʹ��������

	printf("Input ProxyServer Name\n");
	
	char ProxyServerName[20] = {0};

	scanf("%s",ProxyServerName);

	int ProxyPort = 0;

	printf("Input ProxyServer Port\n");
	
	scanf("%d",&ProxyPort);

	char ProxyUserName[20] = {0};

	printf("Input ProxyUserServer Name\n");

	scanf("%s",ProxyUserName);

	int ProxyUserPassWord = 0;

	printf("Input Proxy Server PassUserWord\n");

	scanf("%d",&ProxyUserPassWord);


	//����һ���׽���

	SOCKET s;


	if (!Login(ServerName,UserName,UserPassWord,s,ProxyServerName,ProxyUserName,ProxyUserPassWord,ProxyPort))
	{
		//����

		printf("Login() Error");

		return;
	}

	

	//����������Ҫ����һ���߳�ѭ��������Ϣ����ʽ�������ʼ���������������ͨ��
	
	HANDLE hThread;

	hThread = CreateThread(NULL,0,WorkThread,(LPVOID)sReal,0,NULL);

	




	WaitForSingleObject(hThread,INFINITE);








}

bool InitSocket()
{
	WSADATA wsadata;

	if (WSAStartup(MAKEWORD(2,2),&wsadata)!=0)
	{
		return false;
	}

	return true;
}


//����Ĳ���s���ʹ�����ù̶��ڴ�
bool Login(char* ServerName,char* UserName,int UserPassWord,SOCKET& s,char* ProxyServerName,char* ProxyUserName,
		   int ProxyUserPassWord,int ProxyPort)
{
	//����һ���û�Ŀ¼����ǰ������

	CreateUserDirectory(UserName);

	char Buffer[512]={0};

	if (ProxyServerName!=NULL)    //��������ж�����Ҳ���Ը���ϸЩ
	{
		unsigned long ServerAddr = ConvertServerNameToLongServerAddr(ServerName);

		if (!Socket5Start(ProxyServerName,ProxyPort,ProxyUserName,ProxyUserPassWord,ServerAddr,ServerPort,s))
		{
			
			//����
			printf("Socket5Start() Error");
			
			return  false;
		}


		//��������˳ɹ������ϲ��ҽ�����Ҫ���ӵ��ʼ�����������Ϣ�ɹ����͵�����˲��ĳɹ��ķ���

        

		//����ǳ���Ҫ��������Ҫ��������½��������Ա�ʵ��ת�������Ǹ��ص�


		recv(s,Buffer,10,0);
		
		sockaddr_in addrin;
		
		
		
		addrin.sin_addr.s_addr =
			MAKELONG(MAKEWORD((Buffer[4]&0xff),(Buffer[5]&0xff)),
			MAKEWORD((Buffer[6]&0xff),(Buffer[7]&0xff)));
		
		
		int uPort = 0;

		uPort = ((int)Buffer[8])*256+(unsigned char)Buffer[9];
		
		//����
		//printf("%s\n",inet_ntoa(ServerAddr.sin_addr));
		
		//printf("%d",uPort);
		
		
		addrin.sin_family = AF_INET;
		addrin.sin_port = ntohs(uPort); 
		
		//���ǵõ��µ�������Ϣ������Ҫ�����µ���������������
		
		sReal = socket(AF_INET,SOCK_STREAM,0);
		
		while (1)
		{
			if(connect(sReal,(sockaddr*)&addrin,sizeof(addrin))==SOCKET_ERROR)
			{
				printf("Connect Error\n");
				
				continue;
			}
			
			break;
			
		}

	

		//�������������õ������߳���Ҫ����Ϊ�����������ӵĲ���


	//	HANDLE hThread[2];
		
		
	//	hThread[0] = CreateThread(NULL,0,RecvProc,(LPVOID)sReal,0,NULL);
		
	//	hThread[1] = CreateThread(NULL,0,SendProc,(LPVOID)sReal,0,NULL);
		
		
		
	//	WaitForMultipleObjects(2,hThread,false,INFINITE);

		

		//�����Ǹ����õ�ServerName������ͨ��������������͵��ʼ�������������֤

		memset(Buffer,0,512);

		


	    sprintf(Buffer,"USER %s\r\n",UserName);

		//int n = strlen(Buffer); ����
		send(sReal,Buffer,strlen(Buffer),0);


		memset(Buffer,0,512);
		sprintf(Buffer,"PASS %d\r\n",UserPassWord);
		send(sReal,Buffer,strlen(Buffer),0);



		//������ý��д���Ĵ���ʹ��Recv������û�д���ʹ��whileѭ�����û������µ�½�Ļ���

		memset(Buffer,0,512);


		int nRet =recv(sReal,Buffer,512,0);

		Buffer[nRet]='\0';

		printf("%s\n",Buffer);
		

		return true;
		
	}

	return false;

}

void CreateUserDirectory(char* UserName)
{
	//���������ǿ���ʹ���ļ����ҵļ�����غ���
	
	WIN32_FIND_DATA FileData;   
	
	HANDLE hSearch;

	char Temp[20] = {0};

	strcpy(Temp,".\\");

	strcat(Temp,UserName);

	hSearch = FindFirstFile(Temp,&FileData);

	if (hSearch==INVALID_HANDLE_VALUE||FileData.dwFileAttributes!=FILE_ATTRIBUTE_DIRECTORY)
	{
		//���Ŀ¼��û�и��û���Ŀ¼���ǽ��д������QQ�д����ļ�Ŀ¼�ķ���һ����һ�������õ���֪ʶ��

		//������������ý����������֤Ҫ�õ����ݿ�������ݽṹ����������û���������(���ڶ��ν��в���ʱ)

		CreateDirectory(Temp,NULL);
	}

	//���Ҫ�ر��ļ����

	FindClose(hSearch);
}

unsigned long ConvertServerNameToLongServerAddr(char* ServerName)
{


	//������ҪѧϰDEMO ���������ǽ����ǵļ�������������ʹ��DEMO���÷�
	HOSTENT* host = gethostbyname(ServerName);


	if (host==NULL)
	{
		printf("Can Not Resolve Host Name\n");

		return -1;
	}

	return ntohl((*(struct in_addr*)host->h_addr).S_un.S_addr);
}


DWORD WINAPI WorkThread(LPVOID lpParam)
{
	SOCKET s = (SOCKET)lpParam;


	char ComBuffer[20]={0};



	int nRet = 0;

	printf("\n");
	printf("Please Input STAT First\n");
	while (1)
	{
		
		memset(ComBuffer,0,20);
		
		printf("Input Command\n");
	//	scanf("%s",ComBuffer);
		cin.getline(ComBuffer, 20);

		if (strcmp(ComBuffer,"STAT")==0)
		{
			MailSTAT(s);              
		}

		if (strstr(ComBuffer,"LIST"))
		{
			char* pTemp = ComBuffer;

			pTemp = pTemp + strlen("LIST-");


			
			MailLIST(s,*pTemp);
		} 	    

		
		if (strstr(ComBuffer, "RETR "))
		{
			MailDownLoad(s, ComBuffer);
			
		}
		
	}
}

void MailSTAT(SOCKET& s)
{
	char Buffer[4096]={0};

	int NumOfMails = 0;
	
	sprintf(Buffer,"STAT\r\n");
	
	send(s,Buffer,strlen(Buffer),0);
	
	memset(Buffer,0,4096);
	
	int nRet = recv(s,Buffer,4096,0);
	
	Buffer[nRet]='\0';
	
	//�յ���ȷ����Ϣ�������ż���Ϣ��ʽ�������ǲ���NumOfMails�������������ʹ������
	
	sscanf(Buffer+3,"%d",&NumOfMails);


	printf("Mails Is:%d\n",NumOfMails);


	char* pTemp = Buffer+6; 

	while (*pTemp!='\0')
	{
		while (*pTemp!='\r'&&*(pTemp+1)!='\n')
		{
			printf("%c",*pTemp);             //��������û�����ַ����������õ����ַ���������ļ�����ʱ��Ҫ�����ĵ���Ϊ���ĵ�ASC�벻ֹһ���ֽ�
			pTemp++;
		}
		pTemp+=2;

		printf("\n");
	}

}



void MailLIST(SOCKET& s,char a)
{
	char Buffer[4096]={0};

	int NumOfMail = 0;

	int CountOfMail = 0;
	
	sprintf(Buffer,"LIST %c",a);
	
	send(s,Buffer,strlen(Buffer),0);
	
	memset(Buffer,0,4096);


	int nRet = recv(s,Buffer,4096,0);

	Buffer[nRet] = '\0';

//	char* pTemp = Buffer+6; 

	sscanf(Buffer+3,"%d",&NumOfMail);

	sscanf(Buffer+6,"%d",&CountOfMail);

	printf("%dFile:  %dBytes\n",NumOfMail,CountOfMail);

}

bool Socket5Start(char* ProxyServerName,int ProxyPort,char* ProxyUserName,int ProxyPassWord,unsigned long ServerAddr,int sPort,
				  SOCKET& s)
{
	//��ʼ���׽���
	s = socket(AF_INET,SOCK_STREAM,0);


	//��ʼ�����Ӷ���Ҳ����������Ҫ���ӵĴ��������

	sockaddr_in ProxyAddr;

	ProxyAddr.sin_addr.S_un.S_addr = htonl(ConvertServerNameToLongServerAddr(ProxyServerName));  //����Ҳ��������ͨ����������õ���IP��ַ

	ProxyAddr.sin_family = AF_INET;

	ProxyAddr.sin_port = htons(ProxyPort);   //����ڶ˿�һ��Ҫ�����ǵĴ���ķ������Ķ˿�һ��

	//���ԣ�

	//printf("Proxy Server IP: %s ",inet_ntoa(ProxyAddr.sin_addr));


	//�����ǳ�ʼ���õĴ����������������

	if(connect(s,(sockaddr*)&ProxyAddr,sizeof(sockaddr))==SOCKET_ERROR)
	{
		printf("Cannot connect to Proxy\n");

		closesocket(s);  
		
		return false;
	}


	//����ȷ�����ݰ�����Ҫ�鿴����Socket5��ѧϰ

	char Buffer[512] = {0};

	BYTE Version=5;
	BYTE authorWay1=0;	//����Ҫ��֤
	BYTE authorWay2=2;	//��֤
	int rsv=0;


	//����������Ҫ�������ǵĵ�һ�����ݰ�

	sprintf(Buffer,"%c%c%c%c",Version,2,authorWay1,authorWay2);     //�������ݰ��Ĺ���һ��Ҫ�����Ǵ���������Ĵ��������ݰ�����һ��


	send(s,Buffer,4,0);   //�����������Ҫ�������ݰ�

	int nRet = recv(s,Buffer,512,0);


	Buffer[nRet]='\0';

	if (Buffer[1]==authorWay2)
	{
		//����Ҫ�����ǵ��û������������뷢�͵������

		sprintf(Buffer,"%c%c%s%c%d",char(0x01),char(strlen(ProxyUserName)),ProxyUserName,char(sizeof(int)),ProxyPassWord);

		nRet = 7+strlen(ProxyUserName);   //����úõ����

		send(s,Buffer,nRet,0);


		nRet=recv(s,Buffer,512,0);


		Buffer[nRet]='\0';
	
		if(Buffer[1]!=0)
		{
			return false;
		}
	
		
	}

	
	//����������Ҫ���ʵ��ʼ���������IP��ַ�Ͷ˿ں�
	
	int uPort = htons(sPort);
	
	
	char* p = Buffer;
	
	
	//��ʽ���ʼ��������е���Ϣ
	
	memset(Buffer,0,sizeof(Buffer));
	
	sprintf(Buffer,"%c%c%c%c%c%c%c%c%c%c",Version,char(0x01),rsv,char(0x01),((char*)&ServerAddr)[3],
		((char*)&ServerAddr)[2],((char*)&ServerAddr)[1],((char*)&ServerAddr)[0],
		((char *)&uPort)[0],((char *)&uPort)[1]);   //����Ķ˿ں���110
	
	
	send(s,Buffer,10,0);    //����Ϣ���͵�������������ر�����������ֽ�Ҫ�����Ĳ���DEMO



	//�ȴ�У����Ϣ

	memset(Buffer,0,512);
	
	nRet = recv(s,Buffer,512,0);
	
	Buffer[nRet] = '\0';

	if (Buffer[1]!=0)
	{
		printf("Can Not Connect Destination\n");

		return false;
	}

	return true;
}





void MailDownLoad(SOCKET& s, char* srt)
{
	send(s, srt, strlen(srt), 0);

	char* Buffer = (char*)malloc(sizeof(char)*100);

	int Ret = recv(s, Buffer, 512, 0);

	char SaveDic[256];
	char CurrentDic[256];
	char Temp[12];
	DWORD dwWrite;

	GetCurrentDirectory(256, CurrentDic);

	strcat(CurrentDic, "\\wangxue");

	strcpy(SaveDic, CurrentDic);

	scanf(" %s", Temp);
	
	strcat(SaveDic, "\\");

	strcat(SaveDic, Temp);

	HANDLE hFile = CreateFile("1.txt", GENERIC_WRITE, 0, 
		NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	BOOL bRet = WriteFile(hFile, (LPVOID)Buffer, Ret, &dwWrite, NULL);

	if (bRet == TRUE)
	{
		MessageBox(NULL, "Receive File OK", "OK", MB_OK);
	}
}









/*
DWORD WINAPI RecvProc(LPVOID lpParam)
{
	SOCKET s = (SOCKET)lpParam;
	
	char Buffer[512]={0};
	
	while (1)
	{
		int Ret = recv(s,Buffer,512,0);
		
		Buffer[Ret]='\0';
		
		printf("%s\n",Buffer);
		
		
		memset(Buffer,0,512);
	}
	
	
	return 0;
}


DWORD WINAPI SendProc(LPVOID lpParam)
{
	SOCKET s = (SOCKET)lpParam;
	
	char Buffer[512]={0};
	
	while (1)
	{
		scanf("%s",Buffer);
		
		int Ret = send(s,Buffer,strlen(Buffer),0);	
		
		memset(Buffer,0,512);
	}
	
	
	return 0;
}*/


