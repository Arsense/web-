#include <WINSOCK2.H>
#include <windows.h>
#include <stdio.h>
#include <malloc.h>



//������Ҫһ���õ����ݽṹ��û�й���øó���˼�����

#pragma comment(lib,"Ws2_32")


//����һ���˿ں�

#define uPort  23
#define ConnectMax 2





//һ���ṹ��������û�����Ϣ

typedef struct _USER_ 
{
	char szName[20];     //�û�������
	char szInput[256];   //�û��������Ϣ
	SOCKET sClient;      //���û�ͨ�ŵ��׽���
	sockaddr_in UserAddr; //�û��ĵ�ַ
	int    nLen;          //��ַ�ĳ���
}User,*pUser;


//һ����Ϣ�Ľṹ��

typedef struct _MESSAGE_
{
	int nUser;
	char cMsg[256];

}MESSAGE,*pMESSAGE;

char CONNECT_MSG[30]={0};
char SERVER_FULL_MSG[30]={0};



bool InitSocket();

void AcceptConnects(SOCKET& s,pUser& pTemp);

void SendUser(int Index,pUser& pTemp,char* Buffer);

void CloseClientSocket(int Index,pUser& pTemp);

void AcceptMessages(pUser& pTemp,pMESSAGE& mMsg);

MESSAGE GetMessage(pMESSAGE& mMsg);

void CloseEmptySockets(pUser& pTemp);

void main()
{

	char Buffer[4096]={0};
	//��ʼ���׽��ֿ�
	
	if (!InitSocket())
	{
		printf("Initilize Socket Error\n");

		return;
	}


	//����һ�������׽���

	SOCKET sListen = socket(AF_INET,SOCK_STREAM,0);

	if (sListen==INVALID_SOCKET)
	{
		printf("Create Listen Error\n");

		return;
	}

	//��ʼ����������

	sockaddr_in LocalAddr;
	LocalAddr.sin_family = AF_INET;
	LocalAddr.sin_port = htons(uPort);
	LocalAddr.sin_addr.S_un.S_addr = INADDR_ANY;


	//������
	int Ret = bind(sListen,(sockaddr*)&LocalAddr,sizeof(LocalAddr));

	if (Ret==SOCKET_ERROR)
	{
		closesocket(sListen);

		printf("Bind Error\n");
		
		return;
	}

	//����

	listen(sListen,ConnectMax);


	//�����ڴ��������������������ڴ�û��������

	pUser pTemp = (pUser)malloc(sizeof(User)*6);

	if (pTemp==NULL)
	{
		closesocket(sListen);

		printf("Apply Memory Error\n");
		
		return;
	}


	//��ʼ���ڴ��е���Ϣ

	int i = 0;
	for (i=0;i<6;i++)
	{
		pTemp[i].sClient = INVALID_SOCKET;
		memset(pTemp[i].szName,0,20);
		memset(pTemp[i].szInput,0,256);
		pTemp[i].nLen = sizeof(sockaddr_in);
	}




	//���������ﹹ���������ݰ�һ����֪ͨ�û�����Ե�½��һ�����ݰ���֪ͨ�û���½����

	CONNECT_MSG[0] = 0;

	strcat(CONNECT_MSG,"Welcome!");

	char Temp[5]={0};
	Temp[0] = (char)13;
	Temp[1] = (char)10;   //�������ǻس�����
	Temp[2] = 0;

	strcat(CONNECT_MSG,Temp);

	


	SERVER_FULL_MSG[0] = 0;
	
	strcat(SERVER_FULL_MSG,"Full Member Wait...!");
	
	strcat(SERVER_FULL_MSG,Temp);


	//����һ����Ϣ���е����鲢��ʼ����Ϣ����
	pMESSAGE mMeg = (pMESSAGE)malloc(sizeof(MESSAGE)*10);

	for (i=0;i<10;i++)
	{
		mMeg[i].cMsg[0]=0;
		mMeg[i].nUser = -1;
	}


	MESSAGE mTemp = {0};

	while (1)
	{
		AcceptConnects(sListen,pTemp);   //��������


		AcceptMessages(pTemp,mMeg);                //���ܴ��ݹ�������Ϣ���͵���Ϣ������



		mTemp = GetMessage(mMeg);       //�����õĆ����Ի����


		if (mTemp.cMsg[0]!=0)
		{
			//����Ϣ��ĩβ���
			strcat(mTemp.cMsg,Temp);


			printf("%s",mTemp.cMsg);
		}


		CloseEmptySockets(pTemp);   //�ͷ��Ѿ��Ͽ��Ŀͻ���
	}



}

// �رտ�����
void CloseEmptySockets(pUser& pTemp)
{
	for (int i = 0; i< (ConnectMax + 1); i++)
	{
		SendUser(i,pTemp, "");
	}
}


void AcceptConnects(SOCKET& s,pUser& pTemp)
{
	fd_set fds;

	FD_ZERO(&fds);
	
	FD_SET(s,&fds);

	int iConnect = 0;

	timeval tTime;

	tTime.tv_sec = 0;
	tTime.tv_usec = 0;

	iConnect = select(0,&fds,NULL,NULL,&tTime);

	int i = ConnectMax + 1;     //�����i�ǿ�������������ǵ���������ʱ�������˵���ʱ����ҲҪ�������Ӳ���������һ����Ϣ

	while ((iConnect)&&(i))
	{
		int j = 0;
		while ((pTemp[j].sClient!=INVALID_SOCKET)&&(j<(ConnectMax + 1)))
		{
			j++;
		}

		if (j<ConnectMax)
		{
			pTemp[j].sClient = accept(s,(sockaddr*)&pTemp[j].UserAddr,(int*)&pTemp[j].nLen);

			if (pTemp[j].sClient!=INVALID_SOCKET)
			{
				SendUser(j,pTemp,&CONNECT_MSG[0]);
			}
		}


		else
		{
			pTemp[j].sClient = accept(s,(sockaddr*)&pTemp[j].UserAddr,(int*)&pTemp[j].nLen);
			
			if (pTemp[j].sClient!=INVALID_SOCKET)
			{
				SendUser(j,pTemp,&SERVER_FULL_MSG[0]);

				CloseClientSocket(j,pTemp);
			}
		}


		iConnect = select(0,&fds,NULL,NULL,&tTime);

		i--;

	}


}



void AcceptMessages(pUser& pTemp,pMESSAGE& mMsg)
{

	int i = 0;
	int j = 0;
	int k = 0;
	int Ret = 0;
	u_long lMsg = 0;

	int nAtoI = 0;

	char* cInput = NULL;

	char cMsg[255]={0};

	char cTemp[2] = {0};
	for (i=0;i<ConnectMax;i++)
	{

		Ret = ioctlsocket(pTemp[i].sClient,FIONREAD,&lMsg);    //��MSDN�п����ú���

		if ((Ret==0)&&(lMsg>0))
		{
			//�������Ϣ�������ǽ��н���

			Ret = recv(pTemp[i].sClient, cMsg, 255, 0);

			if ((Ret != SOCKET_ERROR) && (Ret != 0))
			{
				//�������ݰ�
				for (j=0;j<(int)strlen(cMsg);j++)
				{
					nAtoI= (int)cMsg[j];

					//���if�е������ǳ��ؼ���MSDN���в�ASCII���
					if ((cMsg[0] != 0) && (strlen(pTemp[i].szInput) < 255) && (nAtoI >31) 
						&& (nAtoI < 127))
					{
						cInput = &pTemp[i].szInput[0];
						cTemp[0] = cMsg[j];
						cTemp[1] = 0;
						
						strcat(cInput, &cTemp[0]);            //���ݰ�����
					}


					if (nAtoI == 13)                          //������Ϣ���н����ǵ���Ϣ�͵���Ϣ������   
					{
						k = 0;
						while (mMsg[k].cMsg[0] != 0) 
						{ 
							k++; 
						}
						
						mMsg[k].nUser = i;
					
						strcat(&mMsg[k].cMsg[0], &pTemp[i].szInput[0]);
						
						pTemp[i].szInput[0] = 0;                      
					}
				}
				 

			}
		}


	}
}

MESSAGE GetMessage(pMESSAGE& mMsg)
{
	MESSAGE mMessage;
	
	mMessage.nUser = mMsg[0].nUser;
	mMessage.cMsg[0] = 0;
	if (mMsg[0].cMsg[0] != 0)
	{
		strcpy(&mMessage.cMsg[0], &mMsg[0].cMsg[0]);
	}
	
	for (int i = 0; i< 10; i++)
	{
		mMsg[i] = mMsg[i + 1];
	}
	
	return mMessage;
}

void SendUser(int Index,pUser& pTemp,char* Buffer)
{
	int Ret;

	Ret = send(pTemp[Index].sClient,Buffer,strlen(Buffer)+1,0);

	if (Ret==SOCKET_ERROR)
	{
		CloseClientSocket(Index,pTemp);
	}
}


void CloseClientSocket(int Index,pUser& pTemp)
{
	
	shutdown(pTemp[Index].sClient, 0x02);
	closesocket(pTemp[Index].sClient);

	memset(pTemp[Index].szName,0,20);

	memset((void*)&pTemp[Index].UserAddr,0,sizeof(sockaddr_in));


	pTemp[Index].sClient = INVALID_SOCKET;
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



