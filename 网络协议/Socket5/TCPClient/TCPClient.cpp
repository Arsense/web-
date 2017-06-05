//////////////////////////////////////////////////////////
// TCPClient.cpp�ļ�



#include <winsock2.h>
#pragma comment(lib, "WS2_32")

#include <stdio.h>
#include <iostream.h>

DWORD WINAPI RecvProc(LPVOID lpParam);
DWORD WINAPI SendProc(LPVOID lpParam);


int main()
{
	WSADATA wsaData;
	WORD sockVersion = MAKEWORD(2,2);
	if(WSAStartup(sockVersion, &wsaData) != 0)
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
	



	//�������Ƿ��͵ĵ�һ�����ݰ�

	/*
	+----+----------+----------+
	|VER | NMETHODS | METHODS  |   
	+----+----------+----------+
	| 1  |    1     | 1 to 255 |
	+----+----------+----------+
    */
	char a[3] ={0x05,0x01,0x02};      //����Ҳ��ʹ��ʮ������  a[3]={0x01,0x01,0x02};

	send(s,a,strlen(a),0);



	//���ܵ�һ���ظ���

	char Buffer[512]={0};


	int Ret = recv(s,Buffer,512,0);

	Buffer[Ret]='\0';


	if (Buffer[0]!=0x05||Buffer[1]!=0x02)
	{
		
		return -1;
	}

	printf("Please Input Your Name And Password\n");

	

	char NameBuffer[20]={0};
	
	scanf("%s",NameBuffer);

	char PassWord[20]={0};
	
	scanf("%s",PassWord);
	//�������ǵĵڶ������ݰ��������������룬������ݰ������û��Ĳ�ͬҪ���в�ͬ�Ķ���

	//��������û�н����κΰ�ȫ����


	//ע����������İ汾�ű�����0x01���������ǸոմӴ�������0x05
	
	/*
	+----+------+----------+------+----------+
	|VER | ULEN |  UNAME   | PLEN |  PASSWD  |
	+----+------+----------+------+----------+
	| 1  |  1   | 1 to 255 |  1   | 1 to 255 |
	+----+------+----------+------+----------+
	*/

	//��������
	Buffer[0]=0x01;
	Buffer[1]=strlen(NameBuffer)+0x30;


	

	send(s,Buffer,2,0);

	send(s,NameBuffer,strlen(NameBuffer),0);





	//��������

	memset(Buffer,512,0);

	Buffer[0]=strlen(PassWord)+0x30;
		
	send(s,Buffer,1,0);
	
	send(s,PassWord,strlen(PassWord),0);




	//���ܵڶ������ݰ���У��
	Ret = recv(s,Buffer,512,0);
	
	if(Buffer[0]==0x05&&Buffer[1]==0x00)
	{
		printf("Success Welcome!!!\n");
	}

	else
	{
		printf("Error\n");
	}



	//���������

	//�������ݰ�
	/*
	+----+-----+-------+------+----------+----------+
	|VER | CMD |  RSV  | ATYP | DST.ADDR | DST.gListenSocket |
	+----+-----+-------+------+----------+----------+
	| 1  |  1  | X'00' |  1   | Variable |    2     |
	+----+-----+-------+------+----------+----------+
    */


	memset(Buffer,512,0);

	Buffer[0] = 0x05;

	printf("Input Cmd\n");

	char Cmd[10]={0};

	scanf("%s",Cmd);


	if (stricmp(Cmd,"Connect")==0)
	{
		Buffer[1]=0x01;
	}

	else if (stricmp(Cmd,"Bind")==0)
	{
		Buffer[1]=0x02;
	}

	else if (stricmp(Cmd,"Udp")==0)
	{
		Buffer[1]=0x03;
	}

	else
	{
		Buffer[1]=0xff;

		printf("Cmd Error\n");
	}


	Buffer[2]=0x00;    //�����Ǳ�������Ϊ0x00��


	Buffer[3]=0x01;
	
	
    //Ŀ��IP�ֶεĹ���
	
	printf("Input Destination IP:\n");

	char IP[20]={0};

	scanf("%s",IP);

	char* p = IP;
	
	char Temp[20]={0};
	
	int i = 0;
	
	int j = 4;
	
	while(p!=NULL)
	{
		p = strstr(IP,".");
		
		if (p!=NULL)
		{
			strcpy(Temp,p+1);
			
			*p='\0';
			
			i = atoi(IP);

			strcpy(IP,Temp);
			
			
			Buffer[j] = char(i);
			
		}
				
		else
		{
			i = atoi(Temp);
			
			Buffer[j] = char(i);
		}
		
		
		j++;
	}

	 //Ŀ��Port�ֶεĹ���
	
	printf("Input Destination Port:\n");

	char Port[20]={0};
	
	scanf("%s",Port);


	i = 0;

	i = atoi(Port);


	Buffer[8] = (char)(i/256);
	
	Buffer[9] = (char)(i-Buffer[0]*256);

	send(s,Buffer,10,0);



	//����Ӧ�����ݰ�

	memset(Buffer,0,512);


	Ret = recv(s,Buffer,512,0);        



	//�յ����ݰ�����Ҫ�������ݰ��ķ���



	sockaddr_in addrin;    //��ַ��Ϣ


	
	
	addrin.sin_addr.s_addr =
		MAKELONG(MAKEWORD((Buffer[4]&0xff),(Buffer[5]&0xff)),
                      MAKEWORD((Buffer[6]&0xff),(Buffer[7]&0xff)));



	
	//����
	//printf("%s",inet_ntoa(addrin.sin_addr));
	

	int uPort = 0;


	uPort = ((int)Buffer[8])*256+(unsigned char)Buffer[9];


	addrin.sin_port = ntohs(uPort);

	addrin.sin_family = AF_INET;
	//����
	//printf("%d",uPort);



	closesocket(s);

	//���ǵõ��µ�������Ϣ������Ҫ�����µ���������������

	SOCKET sReal = socket(AF_INET,SOCK_STREAM,0);


	while (1)
	{
		if(connect(sReal,(sockaddr*)&addrin,sizeof(addrin))==SOCKET_ERROR)
		{
			printf("Connect Error\n");
			
			continue;
		}

		break;

	}

	


	HANDLE hThread[2];
	
	
	hThread[0] = CreateThread(NULL,0,RecvProc,(LPVOID)sReal,0,NULL);
	
	hThread[1] = CreateThread(NULL,0,SendProc,(LPVOID)sReal,0,NULL);
	
	
	
	WaitForMultipleObjects(2,hThread,false,INFINITE);



	return 0;
}



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
}

