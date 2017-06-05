#include <winsock2.h>
#pragma comment(lib, "WS2_32")

#include <stdio.h>
#include <windows.h>

//�����߳�
//DWORD WINAPI RecvProc(LPVOID lpParam);
//DWORD WINAPI SendProc(LPVOID lpParam);



//���������û����ʼ��������е��ļ���
void CreateUserDirectory(char* UserName);  

//ȷ���ļ������ļ��ļ�����
//�������ǵ��ļ����ֵĴ�����Ҫ�㷨�������ݿ���Ϊ�����޷�ȷ�����͵��ֽ���
void  CheckFile(char* UserDirect,int* FileNum,char* FileName,char* Name);



//�����ļ���������Ϣ
void FindFileByIndex(char a,char* Length,char* pName);



//���ܵ�����˷��͹������û�������Pass������֤
DWORD WINAPI RecvProc(LPVOID lpParam);




typedef struct _SOCKETLIST_ 
{
	SOCKET sClient;

	_SOCKETLIST_* pNext;

}SocketList,*pSocketList;


pSocketList pSocketHead = NULL;
pSocketList pSocketTail = NULL;


//��¼�û���Ϣ�Ľṹ
typedef struct _USERINFOR_
{
	char NameBuffer[20];
	char PassBuffer[20];
	char UserDirec[50];

	_USERINFOR_* pNext;

}UserInfor,*pUserInfor;

pUserInfor pUserHead = NULL;
pUserInfor pUserTail = NULL;

//�û��ļ������ļ�����Ϣ

typedef struct _PERSON_ 
{
	char NameBuffer[20];
	char Index;
	char FileName[100];			//���ﲻ�ܼ�¼�����ļ��Ĵ�С
	char FileLength;
	_PERSON_* pPersonNext;

}Person,*pPerson;




typedef struct _FILEINFOR_
{
     pPerson pPersonHead;
	 pPerson pPersonTail;

	_FILEINFOR_* pFileInforNext;

}FileInfor,*pFileInfor;


pFileInfor pFileHead = NULL;
pFileInfor pFileTail = NULL;




pPerson CreatePerson(char i,char* FileName,char* Name);
pFileInfor CreateFileInfor();
bool CheckInsert(pFileInfor pFileInforTemp,pPerson pPersonTemp);
void InsertPersonToFileInfor(pPerson pPersonTemp);
void LinkFileInfor(pFileInfor pFileInforTemp);

void CreateSocketList(SOCKET& sClient);

void CreateUserList(char* UserBuffer,char* PassBuffer);

void main()
{
	
	//���������ֱ�ӽ���������

	char UserBuffer[20] = "shine";
	char PassBuffer[20] = "1234";

	CreateUserList(UserBuffer,PassBuffer);


	memset(UserBuffer,0,20);
	memset(PassBuffer,0,20);

	strcpy(UserBuffer,"wangxue");
	strcpy(PassBuffer,"1234");

	CreateUserList(UserBuffer,PassBuffer);


	pFileInfor pFileInforTemp = CreateFileInfor();

	LinkFileInfor(pFileInforTemp);
	
	
	
	
	WSADATA wsaData;
	WORD sockVersion = MAKEWORD(2,2);
	if(WSAStartup(sockVersion, &wsaData) != 0)
	{
		exit(0);
	}


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
	ProxyAddr.sin_port = htons(110);
	
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

	HANDLE hThread[2];


	bool bOK = false;
	while (1)
	{
		sClient = accept(sListen,(sockaddr*)&ClientAddr,&nLen);

		CreateSocketList(sClient);


		if (!bOK)
		{
	
//			hThread[1] = CreateThread(NULL,0,SendProc,(LPVOID)pSocketHead,0,NULL);

		

			bOK = true;
		}
		
//		hThread[0] = CreateThread(NULL,0,RecvProc,(LPVOID)pSocketTail,0,NULL);


		hThread[0] = CreateThread(NULL,0,RecvProc,(LPVOID)pSocketTail,0,NULL);

	}



	Sleep(INFINITE);
	//WaitForMultipleObjects(1,hThread[0],false,INFINITE);

}

DWORD WINAPI RecvProc(LPVOID lpParam)
{
	SOCKET s = ((pSocketList)lpParam)->sClient;

	int  FileNum = 0;                 //�����ļ����е��ļ�����
	char FileName[4096] = {0};       //�����ļ����и����ļ����ֲ�����\r\nΪ�����ļ����ֵı��   
	
	char Buffer[512]={0};

	char Name[20] = {0};
    char Pass[20] = {0};


	pUserInfor pUserTemp = pUserHead; 
	
	while (1)
	{
		int Ret = recv(s,Buffer,512,0);    //����ֻ����һ�ξͿ�����
		
		Buffer[Ret]='\0';
		
		//����
	    // printf("%s\n",Buffer);


		//����������õ�����˴��������û�������������Ҫ�����������֤


		char* pTemp = Buffer;

	
		char i = 0;

		if (strstr(Buffer,"USER "))
		{
			pTemp = pTemp+strlen("USER ");

			while (*pTemp!='\r'&&*(pTemp+1)!='\n')
			{

				//������þ��Ǽ���һ�������ĳ��ȶ�̬�����ڴ棬�����������������

				Name[i] = *pTemp;

				pTemp++;

				i++;
			}

			Name[i]='\0';

	
			//��������Ͳ���Ҫ�����ٴε���recv������Ϊ���ǵĿͻ��˳���ִ�еĺܿ�

	        i=0;
		  //memset(Buffer,0,512);
		  //Ret = recv(s,Buffer,512,0);


		  //Buffer[Ret]='\0';

		  //���ϵ�ע��Ҫ�ú����
			
		    pTemp = Buffer;

			if (strstr(Buffer,"PASS "))
			{
			
				_asm
				{
					mov pTemp,eax     //Ͷ����
				}

				pTemp= pTemp+strlen("PASS ");
				
				while (*pTemp!='\r'&&*(pTemp+1)!='\n')
				{
					
					//������þ��Ǽ���һ�������ĳ��ȶ�̬�����ڴ棬�����������������
					
					Pass[i] = *pTemp;
					
					pTemp++;
					
					i++;
				}
				
				Pass[i]='\0';

				while (pUserTemp!=NULL)
				{
					if (strcmp(pUserTemp->NameBuffer,Name)==0&&strcmp(pUserTemp->PassBuffer,Pass)==0)
					{

						printf("User:%s Comming\n",Name);
						memset(Buffer,0,512);

					
						strcat(Buffer,"welcome ");

						strcat(Buffer,Name);
						send(s,Buffer,strlen(Buffer),0);      
	
					    CheckFile(pUserTemp->UserDirec,&FileNum,FileName,Name);
						break;
					
					}
					
					else
					{
						//������Ҫʹ��send����
					}

					pUserTemp = pUserTemp->pNext;
				}
				
			}

			else
			{
				//�������������һ��������Ҫʹ��һ��send����
			}
		
		}

		if (strstr(Buffer,"STAT"))
		{
			
			
			sprintf(Buffer,"+OK %d ",FileNum);        //���ʼ��������ȷ���ص�ָ����+OK ����ķ���ָ����-ERR

			strcat(Buffer,FileName);
			
			send(s,Buffer,strlen(Buffer),0);

		}

		if (strstr(Buffer,"LIST"))
		{
		
			
			pTemp = Buffer+strlen("LIST ");

			char a = *pTemp;

			char Length = '0';

			FindFileByIndex(a,&Length,Name);

			int j = 0;

			if (Length=='0')
			{
				j = sprintf(Buffer,"-ERR 0 ");

				sprintf(Buffer+j,"0");

				send(s,Buffer,strlen(Buffer),0);
			}

			else
			{
				a = a-'0';
				j = sprintf(Buffer,"+OK %d ",a);
				
				sprintf(Buffer+j,"%d",Length);
				
				send(s,Buffer,strlen(Buffer),0);

			}

		
		}
		
		memset(Buffer,0,512);
	}
	
	
	return 0;
}





/*����������߳���Client�еĲ����̶߳�Ӧ
DWORD WINAPI RecvProc(LPVOID lpParam)
{
	SOCKET s = ((pSocketList)lpParam)->sClient;

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
	pSocketList pTempHead  = (pSocketList)lpParam;
	
	char Buffer[512]={0};
	
	int Num = 0;


	while (1)
	{
		pSocketList pTemp = pTempHead;

		
		
		Num = 0;
		scanf("%d",&Num);

		Num = Num-1;
		while (Num--)
		{
			pTemp = pTemp->pNext;
		}
		
		scanf("%s",Buffer);
		
		int Ret = send(pTemp->sClient,Buffer,strlen(Buffer),0);	
		
		memset(Buffer,0,512);
	}


	return 0;
}

*/

//��������
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CreateSocketList(SOCKET& sClient)
{
	pSocketList pSocketTemp;

	pSocketTemp = (pSocketList)malloc(sizeof(SOCKET));

	if (pSocketTemp==NULL)
	{
		printf("No Memory\n");

		return;
	}

	else
	{
		pSocketTemp->sClient = sClient;

		pSocketTemp->pNext = NULL;

		if (pSocketHead==NULL)
		{
			pSocketHead = pSocketTail = pSocketTemp;
		}
		else
		{
			pSocketTail->pNext = pSocketTemp;
			pSocketTail = pSocketTail->pNext;
		}
	}
}


void CreateUserList(char* UserBuffer,char* PassBuffer)
{
	pUserInfor pUserTemp;

	char CurrentDirec[50]={0};
	
	pUserTemp = (pUserInfor)malloc(sizeof(UserInfor));
	
	if (pUserTemp==NULL)
	{
		printf("No Memory\n");
		
		return;
	}
	
	else
	{
		//�����û����ʼ��������еĸ����ļ��е�·���������ļ���

		
		GetCurrentDirectory(50,CurrentDirec);

		strcpy(pUserTemp->UserDirec,CurrentDirec);

		strcat(pUserTemp->UserDirec,"\\");
		strcat(pUserTemp->UserDirec,UserBuffer);


		CreateUserDirectory(UserBuffer);




		//�����û����ʼ��������еĸ���������Password
		strcpy(pUserTemp->NameBuffer,UserBuffer);
		strcpy(pUserTemp->PassBuffer,PassBuffer);
		
		pUserTemp->pNext = NULL;
		
		if (pUserHead==NULL)
		{
			pUserHead = pUserTail = pUserTemp;
		}
		else
		{
			pUserTail->pNext = pUserTemp;
			pUserTail = pUserTemp->pNext;
		}
	}
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


void  CheckFile(char* UserDirect,int* FileNum,char* FileName,char* Name)     //���������������Ǵ���ֵ
{
	   
	WIN32_FIND_DATA Data;         //�ļ���Ϣ�ṹ��
	
	HANDLE hFile;

	char FileRoad[50]={0};

	char Temp[50] = {0};

	pPerson pPerTemp = NULL;                     

	
	strcpy(FileRoad,UserDirect);	
	strcat(UserDirect,"\\*.*");
	strcat(FileRoad,"\\");
	
	hFile = FindFirstFile(UserDirect,&Data);
	
	char i = 0 ;
	
	if (hFile!=INVALID_HANDLE_VALUE)
	{
		
		do 
		{
			if (Data.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
			{
				continue;
			}
			
			memset(Temp,0,50);
		
			//��¼�ļ�����Ϣ������������

			strcpy(Temp,FileRoad);

			strcat(Temp,Data.cFileName);
	
			
			
		    strcat(FileName,Data.cFileName);

			
			pPerson pPersonTemp = CreatePerson(i,Temp,Name);         //i = 0     Temp = c:\shine\3.doc    name =shine
			
			InsertPersonToFileInfor(pPersonTemp);
			
			i++;

			strcat(FileName,"\r\n");


			
			
		} while (FindNextFile(hFile,&Data));
	}



	*FileNum = i;

}


pFileInfor CreateFileInfor()
{
	pFileInfor pFileInforTemp = (pFileInfor)malloc(sizeof(FileInfor));
	if (pFileInforTemp == NULL)
	{
		printf("No Memory\n");
		
		return NULL;
	}
	
	pFileInforTemp->pPersonHead = pFileInforTemp->pPersonTail = NULL;

	pFileInforTemp->pFileInforNext = NULL;
	
	return pFileInforTemp;
}


void InsertPersonToFileInfor(pPerson pPersonTemp)
{
	pFileInfor pFileInforTemp = pFileHead;			

	while (pFileInforTemp != NULL)
	{
		//	�������ڵ�ɹ�������
		if (CheckInsert(pFileInforTemp,pPersonTemp) == true)
		{
			return ;
		}
		
		//	�������ڵ㲻�ɹ��Ļ�
		
		pFileInforTemp = pFileInforTemp->pFileInforNext;
	}
	
	if (pFileInforTemp == NULL)
	{
		pFileInforTemp = CreateFileInfor();
		
		LinkFileInfor(pFileInforTemp);
	}
	
	InsertPersonToFileInfor(pPersonTemp);

}

bool CheckInsert(pFileInfor pFileInforTemp,pPerson pPersonTemp)
{

	if (pFileInforTemp->pPersonHead == NULL)				
	{
		pFileInforTemp->pPersonHead = pFileInforTemp->pPersonTail = pPersonTemp;
	}
	else
	{
	
	    if (strcmp(pFileInforTemp->pPersonHead->NameBuffer,pPersonTemp->NameBuffer)==0)
	    {
			pFileInforTemp->pPersonTail->pPersonNext = pPersonTemp;
			pFileInforTemp->pPersonTail = pPersonTemp;
	    }

		else
		{
			return false;
		}
	
	}
		
	return true;
}


pPerson CreatePerson(char i,char* FileName,char* Name)
{
	//	����һ���µĽڵ��ڴ�ռ�
	pPerson pPersonTemp = (pPerson)malloc(sizeof(Person));
	
	if (pPersonTemp == NULL)
	{
		printf("No Memory\n");
		
		return NULL;
	}
	
    FILE* fp;


	pPersonTemp->pPersonNext = NULL;
	
	fp = fopen(FileName,"rb");
	
	//�����ļ���С
	
	if (fp==NULL)
	{
		return NULL;
	}
	
	fseek(fp,0,SEEK_END);
	
	long nSize = ftell(fp); 
	
	fclose(fp);
	
	pPersonTemp->FileLength = (char)nSize;
	
	strcpy(pPersonTemp->NameBuffer,Name);
	
	pPersonTemp->Index = i;
	
	strcpy(pPersonTemp->FileName,FileName);
	
	return pPersonTemp;
}

void LinkFileInfor(pFileInfor pFileInforTemp)
{
	if (pFileHead == NULL)
	{
		pFileHead = pFileTail = pFileInforTemp;
	}
	else
	{
		pFileTail->pFileInforNext = pFileInforTemp;
		
		pFileTail = pFileInforTemp;
	}
}


void FindFileByIndex(char a,char* Length,char* pName)
{
   pFileInfor  pFileTemp = pFileHead;

   pPerson     pPersonTemp = NULL;
   
   a = a-'0';
   
   while (pFileTemp!=NULL)
   {	   
	  pPersonTemp = pFileTemp->pPersonHead;
	   
	   while (pPersonTemp!=NULL)
	   {
		   if ((pPersonTemp->Index==a)&&(strcmp(pPersonTemp->NameBuffer,pName)==0))
		   {
			   *Length = pPersonTemp->FileLength;
			   
			   break;
		   }

		   pPersonTemp = pPersonTemp->pPersonNext;
	   }	   

	   pFileTemp = pFileTemp->pFileInforNext;
   } 
}