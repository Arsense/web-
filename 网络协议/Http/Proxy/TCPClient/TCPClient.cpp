//////////////////////////////////////////////////////////
// TCPClient.cpp�ļ�


//#include "../common/InitSock.h"
#include <winsock2.h>
#pragma comment(lib, "WS2_32")

#include <stdio.h>
#include <iostream.h>


int main()
{
	WSADATA wsaData;
	WORD sockVersion = MAKEWORD(2,2);
	if(::WSAStartup(sockVersion, &wsaData) != 0)
	{
		exit(0);
	}
	// �����׽���
	SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(s == INVALID_SOCKET)
	{
		printf(" Failed socket() \n");
		return 0;
	}
	
	// Ҳ�������������bind������һ�����ص�ַ
	// ����ϵͳ�����Զ�����
	
	// ��дԶ�̵�ַ��Ϣ
	sockaddr_in servAddr; 
	servAddr.sin_family = AF_INET;
	servAddr.sin_port = ntohs(2356);
	// ע�⣬����Ҫ��д����������TCPServer�������ڻ�����IP��ַ
	// �����ļ����û��������ֱ��ʹ��127.0.0.1����
	servAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	
	if(connect(s, (sockaddr*)&servAddr, sizeof(servAddr)) == SOCKET_ERROR)
	{
		printf(" Failed connect() \n");
		return 0;
	}
	
	// ��������





	char *p = "GET http://127.0.0.1/ HTTP1.1";   //  == > GET / HTTP1.1;


	char Buffer[512]={0};

	strcpy(Buffer,p);


	send(s,Buffer,strlen(Buffer),0);

		
		
	

    int Ret = recv(s,Buffer,1000,0);


	Buffer[Ret]='\0';

	printf("%s",Buffer);
	

//	send(s,Buffer,strlen(Buffer),0);



	Sleep(INFINITE);


	return 0;
}
