
/*
	Inet.cpp : implementation of the CInet class

	Implements the internet communication class responsible for downloading files
	from the web server.  It is basically a wrapper class for the WinInet calls.

	Author: Steven E. Sipe
*/

#include "stdafx.h"

#include <direct.h>
#include <process.h>
#include <io.h>

#include "inet.h"

// Filenames to create if a link just uses a directory for a reference -- 
// g_szDefPage is for HTML pages, g_szUnknown is for multimedia files (in which
// case something fatal happened)
// 
const char *g_szDefPage = "index.htm";
const char *g_szUnknown = "unknown.fil";

// ����Э�����ͼ�HTTP����
static const char *szHttp = "http://";

// Request flags for session initialization:
//	Use existing INET setup, don't redirect to other servers automatically,
//	don't save pages in the browser's cache, always reload a page from the 
//	server (don't use the browser's cached copy)

static DWORD dwHttpRequestFlags =
	   INTERNET_FLAG_EXISTING_CONNECT|
	   INTERNET_FLAG_NO_AUTO_REDIRECT|
       INTERNET_FLAG_DONT_CACHE|
       INTERNET_FLAG_RELOAD; 

// �ܹ�������������ͣ�����Ķ����ʾ�ܹ��������е�����
static const TCHAR szHeaders[] = 
//		_T("Accept: */*\r\n");
		_T("Accept: */* \r");

// ���캯��
CInet::CInet()
{
	m_hSession = NULL;
	m_nUniqueCount = 0;
}

// ��������
CInet::~CInet()
{
}

// ��ʼ��һ���µ�WinInet �Ự
CInet::RESULTS CInet::OpenSession(BOOL bUseProxy, CString& strProxyName)
{
	CInet::RESULTS ret;

	// ��
	m_hSession = ::InternetOpen("SiteSnagger",
					bUseProxy?INTERNET_OPEN_TYPE_PROXY:INTERNET_OPEN_TYPE_PRECONFIG,
					bUseProxy?(LPCTSTR)strProxyName:(LPCTSTR)NULL,
					NULL,0);

	// �Ƿ�ɹ�
	if(m_hSession != NULL)
	{
		DWORD dwTimeout = 4000;
		::InternetSetOption(m_hSession,INTERNET_OPTION_CONNECT_TIMEOUT,
									&dwTimeout,sizeof(dwTimeout));
						
		ret = SUCCESS;
	}
	else ret = BAD_URL;

	return(ret);
}

// Close the WinInet session
void CInet::CloseSession()
{
	::InternetCloseHandle(m_hSession);
}

//////////////////////
// Private operations
//////////////////////

// ��Զ�̷��������ָ�����ļ�
CInet::RESULTS CInet::DoGet(CString& strURL, BYTE **pbyBuffer, int& nLen, 
											BOOL bIsTextPage)
{
	CInet::RESULTS nRetCode = UNKNOWN_ERROR;
	CString strTempURL = strURL;
	HINTERNET hConnect = NULL;
	HINTERNET hURL = NULL;
   
    try
	{
		// ��ʼ�����ص����ݴ�Ц
		nLen = 0;
		
		// ȷ���ļ�·����ȷ
		strTempURL.MakeLower();
		if(strTempURL.Find(szHttp) < 0)
		{
			strTempURL = szHttp+strURL;
			strURL = szHttp+strURL;
		}
		else strTempURL = strURL;

		// ȷ���ļ�·���Ϸ�
		CString strServer,strObject,strUser,strPassword;
		INTERNET_PORT nPort;
		DWORD dwServiceType;

		// �ļ�·������
		strTempURL.SpanExcluding("#");
		if(!AfxParseURLEx(strTempURL,dwServiceType,strServer,strObject,nPort,
						strUser,strPassword,ICU_NO_ENCODE))
			return(CInet::BAD_URL);

		// ��������������
		hConnect = ::InternetConnect(m_hSession,strServer,
                                          INTERNET_INVALID_PORT_NUMBER,
                                          strUser,
                                          strPassword,
                                          INTERNET_SERVICE_HTTP,
                                          0,
                                          0) ;

		// ��HTTP�Ự
		strObject = strObject.SpanExcluding("#");
		const char *szTempX[2] = {szHeaders,NULL};
		hURL = ::HttpOpenRequest(hConnect,"GET",strObject,
									HTTP_VERSION,NULL,
                                    szTempX,dwHttpRequestFlags,0) ;

		// ��������
 		::HttpSendRequest(hURL,NULL,0,0,0);

		// ���ʧ�ܣ��򷵻�
	    if(hURL == NULL) 
			return(CInet::BAD_URL);

		// ��鷵����
		DWORD dwRet, dwLen;
		char szTemp[500];
		dwLen = sizeof(szTemp);
		::HttpQueryInfo(hURL,HTTP_QUERY_STATUS_CODE,szTemp,&dwLen,NULL);
		dwRet = (DWORD) atoi(szTemp);
		dwLen = sizeof(szTemp);
		
		m_dwRet = dwRet;

		// ��Ҫ�ض����ҳ��
		if(dwRet == HTTP_STATUS_MOVED || dwRet == HTTP_STATUS_REDIRECT ||
			       dwRet == HTTP_STATUS_REDIRECT_METHOD)
		{                  
			// ����µĵ�ַ
			CString strNewLocation;
			::HttpQueryInfo(hURL,HTTP_QUERY_LOCATION,NULL,&dwLen,0);
			::HttpQueryInfo(hURL,HTTP_QUERY_LOCATION,
						strNewLocation.GetBufferSetLength(dwLen), 
						&dwLen,0);
			strNewLocation.ReleaseBuffer(dwLen);

			// ȷ���µ�ַ������������
			CString strNewServer;
			AfxParseURLEx(strNewLocation,dwServiceType,strNewServer,strObject,nPort,
							strUser,strPassword,ICU_NO_ENCODE);

			// �����������������
			if(strNewServer.IsEmpty())
			{
				// ���ӷ�������
				CString strTempLocation = strServer;
				if(strNewLocation.Left(1) != "/")
					strTempLocation += "/";
				strTempLocation += strNewLocation;
				strNewLocation = strTempLocation;
			}

			//�����µĵ�ַ
			strURL = strNewLocation;

			::InternetCloseHandle(hURL);
			::InternetCloseHandle(hConnect);
			return(LOCATION_MOVED);
		}
		else if(dwRet == HTTP_STATUS_REQUEST_TIMEOUT || 
					dwRet == HTTP_STATUS_GATEWAY_TIMEOUT ||
					dwRet == HTTP_STATUS_NOT_FOUND)
		{
			Sleep(500L);
			::InternetCloseHandle(hURL);
			::InternetCloseHandle(hConnect);
			return(TIMEOUT);
		}
		else if(dwRet == HTTP_STATUS_OK)
		{
			// ��ȡ�ļ�
			BYTE *pbyTempBuffer = *pbyBuffer;
			DWORD dwTotalBytes = 0;
			DWORD dwBytesRead;
			BOOL bOkay = TRUE;

			// ��ô�С
			dwLen = sizeof(szTemp);
			::HttpQueryInfo(hURL,HTTP_QUERY_CONTENT_LENGTH,szTemp,&dwLen,NULL);
			dwLen = (DWORD) atol(szTemp);

			// ȷ��û�л�ô�����ļ�����
			if(dwLen <= 0) 
				dwLen = 10000;

			// ѭ����ȡ����
			while(bOkay)
			{
				// ��ȡ����
				bOkay = ::InternetReadFile(hURL,pbyTempBuffer,dwLen,&dwBytesRead);

				// ����������������
				if(bOkay && dwBytesRead > 0 && dwTotalBytes < MAX_INET_BUFFER-1)
				{
					dwTotalBytes += dwBytesRead;
					pbyTempBuffer += dwBytesRead;
				}
				else break;
			}

			// ��NULL����������
			pbyTempBuffer[0] = '\0';
			nLen = dwTotalBytes;

			// �ر��ļ�
			::InternetCloseHandle(hURL);
			::InternetCloseHandle(hConnect);

			return(SUCCESS);
		}
	}
	catch(...)
	{
		// Fatal error, just get out
		if(hURL) 
			::InternetCloseHandle(hURL);

		if(hConnect)
			::InternetCloseHandle(hConnect);

		return(UNKNOWN_ERROR);
	}
}

// ��������
CInet::RESULTS CInet::SaveFile(CString& strFileName, LPCTSTR lpszDirectory,
								BYTE *pbyBuffer, int nLen)
{
	CFile fileOut;
	CFileException ex;
	CString strTempName = lpszDirectory+strFileName;
	strTempName = strTempName.SpanExcluding("#");

	// ���������ļ�
	if(fileOut.Open(strTempName,CFile::modeCreate|CFile::modeWrite,&ex))
	{
		// д������
		fileOut.Write(pbyBuffer,nLen);

		// �ر��ļ�
		fileOut.Close();
	}
	else return(BAD_FILENAME);

	return(SUCCESS);
}

/////////////////////
// Public operations
/////////////////////

// Retrieves a page from the web server by calling DoGet().  Also handles
// redirection to other servers as well as retries.
CInet::RESULTS CInet::GetPage(CString& strURL, BYTE **pbyBuffer, int& nLen,
									BOOL bRedirectOkay)
{
	int nRetries = 2;
	CInet::RESULTS ret;

	do 
	{
		// Get the page
		ret = DoGet(strURL,pbyBuffer,nLen,TRUE);
	}
	while(nRetries-- && ((ret == LOCATION_MOVED && bRedirectOkay) || 
							ret == TIMEOUT));

	return(ret);
}

// Retrieves a file from the web server by calling DoGet().  Handles
// redirection to other web servers as well as retries.
CInet::RESULTS CInet::GetFile(CString& strURL, BYTE **pbyBuffer, int& nLen)
{
	int nRetries = 2;
	CInet::RESULTS ret;

	do 
	{
		// Get the file
		ret = DoGet(strURL,pbyBuffer,nLen,FALSE);
	}
	while(nRetries-- && (ret == LOCATION_MOVED || ret == TIMEOUT));

	return(ret);
}

// Reset the "tie breaker" filename counter used by GenerateUniqueFileName().
void CInet::ResetUniqueCount()
{
	m_nUniqueCount = 0;
}

// ����һ��Ψһ���ļ������������ͬ���ļ��������ں�������_x����ʾ�µ��ļ���
// x��ʾ�����������ǵڼ���ͬ����
void CInet::GenerateUniqueFileName(CString& strName, CString& strFileName, 
						MAP_FILES_ARRAY &arrNameMap, BOOL bIsText)
{
	BOOL bFound = FALSE;
	MAP_FILES entry;
	CString strTempName = strName;
	strTempName.MakeLower();
	CString strTempFileName;

	// �����URL
	CString strServer,strObject,strUser,strPassword;
	INTERNET_PORT nPort;
	DWORD dwServiceType;
	AfxParseURLEx(strName,dwServiceType,strServer,strObject,nPort,strUser,
							strPassword,ICU_NO_ENCODE);

	// ��úϷ����ļ���
	strFileName = SplitFileName(strObject,CInet::FNAME|CInet::EXT);

	// �Ƿ���һ���ļ���
	if(strFileName.IsEmpty())
	{
		// ������ǣ�ʹ��Ĭ�ϵ��ļ������ߴ���
		if(bIsText)
			strFileName = g_szDefPage;
		else strFileName = g_szUnknown;
	}
	else
	{
		// �����HTML�ļ�
		if(bIsText)
		{
			// ȷ���ļ�����html�ļ���β����������ʹ��.asp�ļ�Ҳ�ܹ�ִ��
			CString strExt = SplitFileName(strFileName,CInet::EXT);
			strExt.MakeLower();

			if(strExt.Find("#") < 0 && strExt != ".htm" && strExt != ".html")
			{
				strFileName += ".htm";
			}
		}
	}

	// ����µ��ļ���
	strTempFileName = strFileName;
	strTempFileName.MakeLower();
	strTempName.MakeLower();

	MAP_FILES *pEntry;
	CString strPage;

	POSITION pos = arrNameMap.GetStartPosition();

	// �ж��Ƿ����
	for(int nEntryNum = 0; nEntryNum < arrNameMap.GetCount(); nEntryNum++)
	{
		arrNameMap.GetNextAssoc(pos,strPage,(CObject *&) pEntry);
		if(pos == NULL || pEntry == NULL) break;

		// �����ҳ�Ѿ�����
		if(strPage == strTempName)
		{
			strName = strPage;
			strFileName = pEntry->strFileName;
			return;
		}

		// ����ҵ�һ������������ļ�
		//�����һ��Ψһ��
		if(pEntry->strFileName == strTempFileName)
		{
			// �˳�ѭ��
			bFound = TRUE;
			break;
		}
	}

	// ����ҵ������ģ��򴴽�һ���µ�
	if(bFound)
	{
		CString strTemp;
		CString strNewFileName;

		// Ѱ�Һ�׺��
		int nIndex = strFileName.Find(".");
		if(nIndex >= 0)
			strNewFileName = strFileName.Left(nIndex);
		else strNewFileName = strFileName;

		// ���뵽�ļ�����
		strTemp.Format("__%d",++m_nUniqueCount);
		strNewFileName += strTemp;
		if(nIndex >= 0)
			strNewFileName += strFileName.Mid(nIndex);

		// �������ļ���
		strFileName = strNewFileName;
	}
}

// �ָ��ļ���
CString CInet::SplitFileName(LPCTSTR lpszPath, int nSplit)
{
	CString strResult;

	char szPath[800],szDrive[800],szFileName[800],szExt[800];
	_splitpath(lpszPath,szDrive,szPath,szFileName,szExt);

	if(nSplit & CInet::DRIVE)
		strResult += szDrive;
	if(nSplit & CInet::PATH)
		strResult += szPath;
	if(nSplit & CInet::FNAME)
		strResult += szFileName;
	if(nSplit & CInet::EXT)
		strResult += szExt;

	return(strResult);
}

CString CInet::GetErrorText()
{
	CString strError;

	switch(m_dwRet)
	{
		case HTTP_STATUS_MOVED:
		case HTTP_STATUS_REDIRECT:
		case HTTP_STATUS_REDIRECT_METHOD:
			strError = "Redirected, but not found";
			break;

		case HTTP_STATUS_REQUEST_TIMEOUT:
		case HTTP_STATUS_GATEWAY_TIMEOUT:
			strError = "Timed out on request";
			break;

		case HTTP_STATUS_NOT_FOUND:
		case HTTP_STATUS_BAD_REQUEST:
		case HTTP_STATUS_GONE:
			strError = "Page or file not found";
			break;
	
		case HTTP_STATUS_DENIED:
		case HTTP_STATUS_PAYMENT_REQ:
		case HTTP_STATUS_FORBIDDEN:
		//case HTTP_STATUS_AUTH_REFUSED:
		//	strError = "Access denied";
		//	break;

		case HTTP_STATUS_PROXY_AUTH_REQ:
			strError = "Proxy authentication required";
			break;

		default:
			strError.Format("WinInet error %d",m_dwRet);
			break;
	}

	return(strError);
}
