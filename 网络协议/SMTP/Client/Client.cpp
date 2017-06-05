#include <winsock2.h>
#include <windows.h>
#include <malloc.h>
#include <string.h>
#include <stdio.h>


#pragma comment(lib,"Ws2_32.lib")


#define uPort 25


bool InitSocket();



void main()
{
	//��ʼ���׽��ֿ�
	
	if (!InitSocket())
	{
		printf("Initilize Socket Error\n");


		return;
	}


	//������������Ҫ�Ļ�����Ҳ������������Ϊ��SMTP�Ļ�������ָ��

	char Buffer[30]={0};

	char RemoteIP[30]={0};

	memset(RemoteIP,20,0);
	memset(Buffer,20,0);

	printf("Server IP:\n");
	scanf("%s",RemoteIP);

	char HELO[30] = "HELO ";                  //��������ַ�Ļ�����Ҳ��������

	strcat(HELO,RemoteIP);

	strcat(HELO,"\r\n");

	char MAIL[30] = "MAIL FROM:<";			//�����ߵĵ�ַ

	memset(Buffer,0,20);

	printf("Send From:\n");
	
	scanf("%s",Buffer);
	
	strcat(MAIL,Buffer);

	strcat(MAIL,">\r\n");


	memset(Buffer,0,20);           
	
	char RCPT[30] = "RCPT TO:<";             //�ռ��ߵĵ�ַ

	printf("Recive To:\n");
	
	scanf("%s",Buffer);

	strcat(RCPT,Buffer);

	strcat(RCPT,">\r\n");



	char szText[1024] = {0};                //�ż�������


	printf("Message:\n");

	scanf("%s",szText);


	strcat(szText,"\r\n\r\n.\r\n");         //��������ǧ��Ҫ����


	//�����ﶨ����һ��ָ������ÿ��ָ��ָ��һ��������
	char *MailMessage[] = 
	{
		HELO,
		MAIL,
		RCPT,
		"DATA\r\n",
		szText,
		"QUIT\r\n",
		NULL
	};



	//����һ���������׽���

	SOCKET sClient = socket(AF_INET,SOCK_STREAM,0);

	if (sClient==INVALID_SOCKET)
	{
		printf("Create Socket Error\n");

		return;
	}

	//��ʼ��������Ҫ���͵�Զ�̵�ַ�ռ�
	sockaddr_in RemoteAddr;

	RemoteAddr.sin_addr.S_un.S_addr = inet_addr(RemoteIP);

	RemoteAddr.sin_family = AF_INET;

	RemoteAddr.sin_port = ntohs(uPort);



	//������������������

	if (connect(sClient,(sockaddr*)&RemoteAddr,sizeof(RemoteAddr))==SOCKET_ERROR)
	{
		closesocket(sClient);

		printf("Connect Error\n");


		return;
	}


	int iLength = 0;
	int iEnd = 0;

	char sBuff[255] ={0};
	int Index = 0;
	//��������

	do 
	{
		//���ܵ���Ӧ��Ϣ

		iLength = recv(sClient,(LPSTR)sBuff+iEnd, sizeof(sBuff)-iEnd,0);
		iEnd += iLength;
		sBuff[iEnd] = '\0';
		send(sClient,(LPSTR)MailMessage[Index],strlen(MailMessage[Index]),0);
		Index++;


	} while (MailMessage[Index]);

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