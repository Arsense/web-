
#include "CComm.h"
#include <stdio.h>

int main( int argc, char *argv[] )
{
	char buffer[4096];		// buffer we'll use to store msg read in from stdin
	short listeningport;	 // port to listen on
	short destport;			  // port to send to
	char *desthost;			// address of destination machine

	printf("��ʼ��...\n");

	// �趨�����в���
	// Make sure data passed on command line is correct
	//	if( argc != 4 || !(listeningport = atoi(argv[1])) 
	//		          || !(destport = atoi(argv[3])))
	//	{
	//		printf("ʹ�÷���: SChat [listen port number] [destination machine] [destination port]\n");
	//		return 0;
	//	}

	//�趨UDP�󶨶˿�
	listeningport=200;
	//�趨���Ͷ˿�
	destport=100;
	desthost = "192.168.0.3";//�趨Ŀ���ַ
	CComm myComm;		
	if (! myComm.Listen(listeningport))	// �󶨵�ַ
	{
		printf("�˿� %s��ʧ��\n", listeningport);	
		return 0;
	}

	printf("�������ɹ��������˿�Ϊ200: <Ctrl-C> �˳�\n\n");	// 

	while( fgets(buffer, sizeof(buffer), stdin) )	// �����������
	{
		myComm.SendMsg( buffer, strlen(buffer), desthost, destport );	// �������ݵ�Ŀ���ַ
	}
	return 0;
}