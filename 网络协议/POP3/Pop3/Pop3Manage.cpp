#include "StdAfx.h"
#include <Windows.h>
#include "Pop3Manage.h"
#include "Pop3Dlg.h"
#include <istream>
using namespace std;

#include <string>

#define MAX_BUFF 20000

CPop3Manage::CPop3Manage(void)
{
	m_state=FIRST;   //��һ����ʼ����Ҫ
	m_error="Not connected to server\r\n";
	m_bDelete=FALSE;
}


CPop3Manage::~CPop3Manage(void)
{
}

void CPop3Manage::SetProp(CString UserName, CString PassWord)
{
	m_UserName=UserName;
	m_PassWord=PassWord;
}


void CPop3Manage::DelAfterRead(BOOL bDelete)
{
	m_bDelete=bDelete;
}


void CPop3Manage::OnReceive(int nErrorCode)
{
	// TODO: �ڴ����ר�ô����/����û���
	if(nErrorCode==0) //if no error
	{
		char buff[MAX_BUFF] = {0};
		int rec=Receive(buff,MAX_BUFF); //receive data
		buff[rec]=NULL;
		m_lastMsg=buff;
		ParseMessage(); //parse data
	}
	else
	{
		m_error="Error while receiving!\r\n";
		((CPop3Dlg*)m_pWnd)->Dispatch(RECEIVE);

	}
	
	
}

void CPop3Manage::ParseMessage()
{
	
	CString  s;
	strstream str;   //һ���ַ�����

	CString check;
	str<<m_lastMsg;
	//str>>check;
	check = m_lastMsg;
	int v = m_lastMsg.Find(" ");
	check = m_lastMsg.Left(v);

	if (check == "-ERR")  //����д�����
	{
		m_error="Received -ERR from server :"+m_lastMsg;
		Close(); //�Ͽ�Ȼ��ر�
		return;
	}
	switch (m_state)
	{
	case FIRST:
		m_Messsages.clear();
		((CPop3Dlg*)m_pWnd)->Dispatch(RECEIVE);
		s.Format("user %s%c%c",m_UserName,13,10);
		
		Send((LPCSTR)s,s.GetLength()); 
		m_state=USER;
		break;
	case USER:
		((CPop3Dlg*)m_pWnd)->Dispatch(RECEIVE);
		s.Format("pass %s%c%c",m_PassWord,13,10); 
		
	
		
		Send((LPCSTR)s,s.GetLength()); //��������
		m_state=PASS;
		break;
	case PASS:
		((CPop3Dlg*)m_pWnd)->Dispatch(RECEIVE);
		s.Format("stat%c%c",13,10);
		
		Send((LPCSTR)s,s.GetLength()); //����stat����
		m_state=STAT; 
		break;

	case STAT:   //������������������������С����Ϣ
		{   //m_lastMsg;  +ok 1 size
			string s1;
			str.seekg(0);   //seekg�����������ļ������ļ���ָ��λ��
			str>>s1>>m_numMsg>>m_sizeMsg; //��������ʹ�С
			flush(str);
			((CPop3Dlg*)m_pWnd)->Dispatch(GETNUMMSGS);
			((CPop3Dlg*)m_pWnd)->Dispatch(GETSIZEMSGS);
			if(m_numMsg>0) //������ʼ�������RETR����ʼ���Ϣ
			{
				m_state=RETR;
				s.Format("retr 1%c%c",13,10);
			
				
				m_retrMsg=1;
				MESSAGEPROP prop;
				prop.MsgSize=0;
				prop.retrSize=0;
				prop.text="";
				m_Messsages.push_back(prop); 
				Send((LPCSTR)s,s.GetLength());

			}
			else //���û���ʼ�����Ͽ�
			{
				m_error="No new messages\r\n";
				Close();
			}
		}
		break;

	case RETR:  //ȡ���ʼ�  ����ȡ��ָ���ʼ��ŵ��ʼ�
		{

			if(m_Messsages[m_retrMsg-1].MsgSize==0) //�����һ�ν��յ�����
			{
				string temp;
				str.seekg(0);
				str>>temp>>m_Messsages[m_retrMsg-1].MsgSize; //�õ���Ϣ��С
			}
			m_Messsages[m_retrMsg-1].text+=m_lastMsg; //��������
			m_Messsages[m_retrMsg-1].retrSize+=m_lastMsg.GetLength(); //�������ݴ�С
			if(m_Messsages[m_retrMsg-1].retrSize>=m_Messsages[m_retrMsg-1].MsgSize) //�ж��Ƿ�����������
			{	//����Ƿ��������ʼ�
				if(m_retrMsg<m_numMsg) //������У����������retr����
				{
					MESSAGEPROP prop;
					prop.MsgSize=0;
					prop.retrSize=0;
					prop.text="";
					m_Messsages.push_back(prop);
					m_retrMsg++;
					s.Format("retr %d%c%c",m_retrMsg,13,10); //request another
					
					Send((LPCSTR)s,s.GetLength());
				}
				else
				{
					//���ȫ��������ϣ��ж��Ƿ�Ҫɾ��
					if(m_bDelete && m_numMsg>0) 
					{
						m_state=DELE;
						m_delMsg=1;
						s.Format("dele %d%c%c",m_delMsg,13,10);
						
						Send((LPCSTR)s,s.GetLength());
					}
					else //�����˳�
					{
						m_state=ENDRETR;
						((CPop3Dlg*)m_pWnd)->Dispatch(ENDRETR1);
						m_error="Session ended\r\n";
						s.Format("quit%c%c",13,10);
						
						Send((LPCSTR)s,s.GetLength());
						Close();
					}
				}
			}

		}break;
	case DELE: //ɾ���ʼ�
		{
			//ɾ��ʣ���ʼ�
			if(m_delMsg<m_numMsg)
			{
				m_delMsg++;
				s.Format("dele %d%c%c",m_delMsg,13,10);
				
				Send((LPCSTR)s,s.GetLength());
			}
			else //����Ѿ�ɾ�����
			{
				((CPop3Dlg*)m_pWnd)->Dispatch(ENDRETR1);
				m_state=GOON;
				m_error="Deleted all messages\r\n";
				s.Format("quit%c%c",13,10);
				
				Send((LPCSTR)s,s.GetLength());
				Close();
			}
		}
		break;
	case GOON: //Ĭ��
	default:
		((CPop3Dlg*)m_pWnd)->Dispatch(RECEIVE);
		break;
	}
	
	
}

void CPop3Manage::GetLastMsg(CString &s)
{
	s = m_lastMsg;
}

CString CPop3Manage::GetError()
{
	return m_error;
}

int CPop3Manage::GetNumMsg()
{
	return m_numMsg;
}

int CPop3Manage::GetSizeMsg()
{
	return m_sizeMsg;
}

int CPop3Manage::GetRetrMsgNum()
{
	return m_Messsages.size();
}
/***************��ȡSubject������*********************/
CString CPop3Manage::GetMsgSubject(int i)
{
	int where=m_Messsages[i].text.Find("Subject:");
	CString ret;
	if (where!=-1)
	{
		ReadLn(where,m_Messsages[i].text,ret);
	}
	return ret;
}



void CPop3Manage::ReadLn(int index,CString src, CString &dst)
{
	CString comp;
	comp=src[index];
	while(comp!="\r")
	{
		dst+=comp;
		comp=src[++index];
	}
	
}

CString CPop3Manage::GetMsgStuff(int i)
{
	CString ret;
	int where=m_Messsages[i].text.Find("From:");
	ReadLn(where,m_Messsages[i].text,ret);
	ret+="\r\n";

	where=m_Messsages[i].text.Find("To:");
	if (where!=-1)
	{
		ReadLn(where,m_Messsages[i].text,ret);
		ret+="\r\n";
	}
	where=m_Messsages[i].text.Find("Date:");
	if (where!=-1)
	{
		ReadLn(where,m_Messsages[i].text,ret);	
		ret+="\r\n";
	}
	ret+=GetMsgSubject(i);
	ret+="\r\n";
	return ret;
}

CString CPop3Manage::GetMsgBody(int i)
{
	CString ret;
	int where=m_Messsages[i].text.Find("\r\n\r\n");
	if(where!=-1)
	where+=4;
	else where=0;
	ret=m_Messsages[i].text.Right(m_Messsages[i].text.GetLength()-where);
	ret=ret.Left(ret.GetLength()-3);
	return ret;

}
