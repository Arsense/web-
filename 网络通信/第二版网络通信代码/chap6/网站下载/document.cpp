
/*	
	Document.cpp : implementation of the CSnaggerDoc class
	
	Implements project file persistence for options, tree data and statistics.
	Also performs the actual retrieval of files from the host using the 
	CInet class.

	Author: Steven E. Sipe
*/

#include "stdafx.h"

#include "SiteSnag.h"
#include "Document.h"
#include "View.h"
#include "progress.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// File version for serialization
const long g_lFileVersion = 0x0101;

/////////////////////////////////////////////////////////////////////////////
// CSnaggerDoc

IMPLEMENT_DYNCREATE(CSnaggerDoc, CDocument)

BEGIN_MESSAGE_MAP(CSnaggerDoc, CDocument)
	//{{AFX_MSG_MAP(CSnaggerDoc)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CSnaggerDoc construction/destruction

// Constructor
CSnaggerDoc::CSnaggerDoc()
{
	// Set some default project options
	m_Options.nMaxDepth = 2;
	m_Options.nMaxPages = 0;
	m_Options.bFixupLinks = TRUE;
	m_Options.bContents = TRUE;
	m_Options.bMultimedia = TRUE;
	m_Options.bOffsiteLinks = FALSE;


	// Set the initial hash table sizes
	m_arrPagesDone.InitHashTable(1200);
	m_arrMediaDone.InitHashTable(2400);

	// Initialize some flags
	m_bProjectLoaded = FALSE;
	m_pProgress = NULL;
	m_bAutoMode = FALSE;

	m_nLevel = 0;
}

// Destructor
CSnaggerDoc::~CSnaggerDoc()
{
	try
	{
		// Remove the page and media maps
		ClearCacheMaps();
	}
	catch(...)
	{
	}
}

//����һ�����ĵ�
BOOL CSnaggerDoc::OnNewDocument()
{

	static bFirstTime = TRUE;

	// Is is this the empty project file?
	if(bFirstTime)
	{
		bFirstTime = FALSE;

		// Yes, set the title to "(No project)"
		CString strDefName;
		strDefName.LoadString(IDS_NO_PROJECT);
		SetTitle(strDefName);

		// Call the base class and get out...
		if (!CDocument::OnNewDocument())
			return FALSE;

		return(TRUE);
	}

	// Call the base class
	if (!CDocument::OnNewDocument())
		return FALSE;

	// Clear the statisitics and indicate that we now have a 
	// project loaded
	m_bProjectLoaded = TRUE;
	m_strStartPage.Empty();
	m_nGottenPageCount = 0;
	m_nGottenFileCount = 0;
	m_nQueuedPageCount = 0;
	m_nTotalBytes = 0;

	// Make sure that the info in the statistics window is reset
	POSITION pos = GetFirstViewPosition();
	CSnaggerView* pView = (CSnaggerView *) GetNextView(pos);
	m_pProgress = pView->GetProgress();
	m_pProgress->SetActionTitle("");

	return TRUE;
	
}

// ��һ���Ѿ�������ĵ�
BOOL CSnaggerDoc::OnOpenDocument(LPCTSTR lpszPathName) 
{
	POSITION pos = GetFirstViewPosition();
	CSnaggerView* pView = (CSnaggerView *) GetNextView(pos);

	// Save the current project (if necessary)
	SaveModified();

	// Initialize the tree
	pView->ClearTree();

	// Call the base class to read the document's contents from disk
	if (!CDocument::OnOpenDocument(lpszPathName))
		return FALSE;

	// Set the project location
	SetPathName(lpszPathName);
	m_strDirectory = CInet::SplitFileName(lpszPathName,
				CInet::DRIVE|CInet::PATH|CInet::FNAME)+"\\";

	// Set the document's title
	SetTitle(CInet::SplitFileName(lpszPathName,CInet::FNAME|CInet::EXT));


	// Indicate that the project is loaded
	m_bProjectLoaded = TRUE;

	// Update the project's information in the statistics window
	if(m_pProgress)
	{
		m_pProgress = pView->GetProgress();
		m_pProgress->SetActionTitle("");
		UpdateStatus();
	}

	// Make sure this document gets saved at the end
	SetModifiedFlag(TRUE);

	return TRUE;
}

// Saves files that have been change (DoFileSave() calls the document's 
// ::Serialize() method
BOOL CSnaggerDoc::SaveModified() 
{
	// Was the document changed??
	if(IsModified())
		return CDocument::DoFileSave();
	return(TRUE);
}

// Prevents the user from exiting the application if a snagging operation is
// in progress
BOOL CSnaggerDoc::CanCloseFrame(CFrameWnd* pFrame) 
{
	POSITION pos = GetFirstViewPosition();
	CSnaggerView* pView = (CSnaggerView *) GetNextView(pos);

	return(!pView->GetSnagging());
}

// ����һ���յĹ���
void CSnaggerDoc::Reset(LPCTSTR lpszProjName)
{
	CString strNewProjName;
		
	if(lpszProjName)
		strNewProjName = lpszProjName;

	// Use the default name -- (No Project)
	strNewProjName.LoadString(IDS_NO_PROJECT);
	m_strPathName.Empty();
	m_strDirectory.Empty();
	m_bProjectLoaded = FALSE;
	SetModifiedFlag(FALSE);
	SetTitle(strNewProjName);

	// Reset the statistics window's information
	m_strStartPage.Empty();
	m_nGottenPageCount = 0;
	m_nGottenFileCount = 0;
	m_nQueuedPageCount = 0;
	m_nTotalBytes = 0;

	// Update the statistics window
	POSITION pos = GetFirstViewPosition();
	CSnaggerView* pView = (CSnaggerView *) GetNextView(pos);
	m_pProgress = pView->GetProgress();
	m_pProgress->SetActionTitle("");

	UpdateStatus();
}

// Handles closing the document -- i.e. the user pressed the X button or chose the
// close menu item
void CSnaggerDoc::OnCloseDocument() 
{
	// Make sure that we save the current document
	SaveModified();

	CDocument::OnCloseDocument();
}

// ��ù���ѡ��
void CSnaggerDoc::GetOptions(CSnagOptions& Options)
{
	Options.nMaxDepth = m_Options.nMaxDepth;
	Options.nMaxPages = m_Options.nMaxPages;
	Options.bFixupLinks = m_Options.bFixupLinks;
	Options.bContents = m_Options.bContents;
	Options.bMultimedia = m_Options.bMultimedia;
	Options.bOffsiteLinks = m_Options.bOffsiteLinks;
}

//���ù���ѡ��,��ͨ����������ִ�е�ʱ�����
void CSnaggerDoc::SetOptions(CSnagOptions& Options) 
{ 
	m_Options.nMaxDepth = Options.nMaxDepth;
	m_Options.nMaxPages = Options.nMaxPages;
	m_Options.bFixupLinks = Options.bFixupLinks;
	m_Options.bContents = Options.bContents;
	m_Options.bMultimedia = Options.bMultimedia;
	m_Options.bOffsiteLinks = Options.bOffsiteLinks;
}

//�ļ����л�
void CSnaggerDoc::Serialize(CArchive& ar)
{
	long lFileVersion;
	POSITION pos = GetFirstViewPosition();
	CSnaggerView* pView = (CSnaggerView *) GetNextView(pos);	

	// �Ƿ�Ҫд��Ӳ��
	if (ar.IsStoring())
	{
		// д��汾��Ϣ
		ar << g_lFileVersion;
		lFileVersion = g_lFileVersion;

		// д��������ڵ���Ϣ
		ar << m_nGottenPageCount;
		ar << m_nGottenFileCount;
		ar << m_nTotalBytes;
	}
	else
	{
		// �����Ϣ
		ar >> lFileVersion;

		// ���������Ϣ
		ar >> m_nGottenPageCount;
		ar >> m_nGottenFileCount;
		ar >> m_nTotalBytes;

		// Reset the queued page count (used in the statistics window)
		m_nQueuedPageCount = 0;
	}

	// ��û��߱��湤����Ϣ
	m_Options.SetVersion(lFileVersion);
	m_Options.Serialize(ar);

	// ��û��߱������οؼ�������
	pView->SerializeTree(ar);

	m_nLevel = 0;
}

//����ָ����ҳ�棬ͨ��CInet���ߴ�Ӳ��������
//���ͨ��CInet���أ��򱣴���ļ���strFileName��ͬʱ�������ļ��е�����
BOOL CSnaggerDoc::GetPage(CString& strPage, CString& strFileName, LINKS& linkEntry)
{
	BYTE *pbyBuffer = m_byBuffer;
	int nLen;
	BOOL bPageInCache = FALSE;
	BOOL bRet = FALSE;
	CInet::RESULTS ret;
	MAP_FILES* pMapEntry;

	// ��ʼ�����Ӷ�ջ���
	linkEntry.arrLinks.SetSize(0,100);
	linkEntry.arrMedia.SetSize(0,100);
	linkEntry.arrOffsite.SetSize(0,100);
	linkEntry.nIndex = 0;

	// �ж��Ƿ���Ҫ���ظ�ҳ
	if(ShouldGetPage(strPage,pMapEntry))
	{
		// �������������룬Ҫ���ظ��ļ�
		ret = m_Inet.GetPage(strPage,&pbyBuffer,nLen,TRUE);
		if(ret == CInet::SUCCESS)
		{
			// ���������ֽ�������
			bRet = TRUE;
			m_nTotalBytes += nLen;
		}
	}
	else
	{
		// ����Ѿ����ع��ˣ����Ʊ����ļ�
		// ��Ϊ��Ҫ����ָ��ԭ�����ع����ļ�
		CFile fileIn;
		CFileException ex;

		// ����������ļ�·��
		strFileName = pMapEntry->strFileName;
		CString strTempFileName = m_strDirectory+strFileName;

		// �򿪸��ļ�
		if(fileIn.Open(strTempFileName,CFile::modeRead,&ex))
		{
			// ��������
			nLen = fileIn.Read(pbyBuffer,MAX_INET_BUFFER);
			fileIn.Close();
			bRet = TRUE;
		}

		// ָʾ��û���������ظ��ļ�
		bPageInCache = TRUE;
	}

	// �Ƿ����
	if(bRet)
	{
		// ��������Լ���ý���ļ���Ϣ
		CSnaggerHtmlParser Parser;
		Parser.SetPageURL(strPage);
		m_pProgress->SetActionTitle("Parsing Page: "+strPage);

		// �ж��Ƿ񳬳���󻺳���
		if(nLen > MAX_INET_BUFFER)
			nLen = MAX_INET_BUFFER;

		// ��ʼ�������÷����ӳ���parser
		pbyBuffer = m_byBuffer;
		Parser.SetFixupMode(FALSE);
		Parser.ResetArrays();
		Parser.SetGetMedia(m_Options.bMultimedia);
		Parser.ParseText((char *)pbyBuffer,nLen);
		m_strPageTitle = Parser.GetTitle();

		// ������ļ�
		if(!bPageInCache)
		{
			m_pProgress->SetActionTitle("Saving Page: "+strPage);
			pbyBuffer = m_byBuffer;
			m_Inet.SaveFile(strFileName,m_strDirectory,pbyBuffer,nLen);
		}

		// ���������ӵ�ҳ�������
		int nLinks;
		BOOL bOffsite;
		CString strNewPage;
		nLinks = Parser.GetLinks().GetSize();

		// ����ÿһ�����ӣ��Ƿ�Ҫ��ӵ����صȴ��б���
		for(int i = 0; i < nLinks; i++)
		{
			// �����ҳ��URL��ַ
			strNewPage = Parser.GetLinks().GetAt(i);

			// Get the offsite link flag for this page
			bOffsite = Parser.GetOffsiteFlags().GetAt(i);

			// See if we should at it to the download queue
			if(ShouldQueuePage(strNewPage,bOffsite)) 
			{
				linkEntry.arrLinks.Add(strNewPage);
				linkEntry.arrOffsite.Add(bOffsite);
			}
		}

		// Don't need the images if we've already parsed this page
		// before
		if(!bPageInCache)
		{
			// New page, so get the all of the media information
			int nMedia = Parser.GetMedia().GetSize();
			CString strMedia;
			for(i = 0; i < nMedia; i++)
			{
				strMedia = Parser.GetMedia().GetAt(i);
				if(ShouldGetMedia(strMedia,pMapEntry))
					linkEntry.arrMedia.Add(strMedia);
			}
		}

		// Success
		bRet = TRUE;
	}

	return(bRet);
}

// ���ָ���Ķ�ý���ļ�
BOOL CSnaggerDoc::GetMedia(CString& strMedia, CString& strFileName)
{
	BYTE *pbyBuffer = m_byBuffer;
	int nLen;
	BOOL bRet = FALSE;

	// ���ָ����ҳ��
	CInet::RESULTS ret;

	// ��INet����ļ�
	ret	= m_Inet.GetFile(strMedia,&pbyBuffer,nLen);

	if(ret == CInet::SUCCESS)
	{
		// �������������Ӹ������ļ���С
		m_nTotalBytes += nLen;

		// д���ļ�
		m_pProgress->SetActionTitle("Saving File: "+strMedia);
		m_Inet.SaveFile(strFileName,m_strDirectory,pbyBuffer,nLen);

		bRet = TRUE;
	}

	return(bRet);
}

//�Զ��Ƿ��ҳ���Ѿ����ع�������Ѿ����ع����򷵻�true
//����Ѿ����أ��򷵻�ָ��ָ��
BOOL CSnaggerDoc::ShouldGetPage(CString& strPage, MAP_FILES*& pMapEntry)
{
	// Page names shouldn't be case sensitive
	CString strNewPage = strPage;
	strNewPage.MakeLower();
	strNewPage = strNewPage.SpanExcluding("#");

	// �趨һ�����·��
	CString strExt = CInet::SplitFileName(strNewPage,CInet::EXT);
	if(strExt.IsEmpty() && strNewPage.Right(1) != "/")
		strNewPage += "/";

	// Did we find it??
	return(!m_arrPagesDone.Lookup(strNewPage,(CObject *&) pMapEntry));
}

//�ж��Ƿ����ض�ý���ļ��������ǰ�Ѿ����ع����򷵻�true
//����Ѿ����ع����򷵻�һ��ָ�������Ѿ����صĶ�ý���б�����
BOOL CSnaggerDoc::ShouldGetMedia(CString& strMedia, MAP_FILES*& pMapEntry)
{
	// Page names shouldn't be case sensitive
	CString strNewMedia = strMedia;
	strNewMedia.MakeLower();
	strNewMedia = strNewMedia.SpanExcluding("#");

	// Page names shouldn't be case sensitive
	return(!m_arrMediaDone.Lookup(strNewMedia,(CObject *&) pMapEntry));
}

//�ú�������true�����ָ����ҳ��Ҫ�����ӵ����ض�����
//��Ҫ��ȷ���Ƿ��ҳ���Ƿ���ǰһ���б�����
//ͬʱ��Ҫ����Ƿ��Ǳ�վҳ��
BOOL CSnaggerDoc::ShouldQueuePage(CString& strNewPage, BOOL bOffsite)
{
	MAP_FILES* pMapEntry;

	// �ж��Ƿ���ǰ���ع�
	if(ShouldGetPage(strNewPage,pMapEntry))
	{
		// �����û�У����ڵȴ��б���Ѱ���Ƿ��Ѿ���ǰһ��ĵȴ��б���
		for(int i = 0; i < m_nLevel; i++)
		{
			for(int j = 0; j < m_aLinks[i].arrLinks.GetSize(); j++)
			{
				if(strNewPage == m_aLinks[i].arrLinks.GetAt(j))
					return(FALSE);
			}
		}
	}
	else
	{
		// ����ﵽ���������򷵻�true 
		if(m_Options.nMaxDepth && m_nLevel >= pMapEntry->nMaxLevel)
			return(TRUE);
	}

	// �Ƿ���������վ���ҳ������
	if(bOffsite && !m_Options.bOffsiteLinks)
		return(FALSE);

	return(TRUE);
}

// ��ʼ��ָ�����ӵĶ�ջ���
void CSnaggerDoc::ResetLink(int nLevel)
{
	m_aLinks[nLevel].nIndex = 0;
	m_aLinks[nLevel].arrLinks.SetSize(0,100);
	m_aLinks[nLevel].arrMedia.SetSize(0,100);
	m_aLinks[nLevel].arrOffsite.SetSize(0,100);
}

// ���²�����ʾ
void CSnaggerDoc::UpdateStatus()
{
	// Does the statistics window exist?
	if(m_pProgress)
	{
		// Yep...update the info in its fields
		m_pProgress->SetQueuedFiles(m_nQueuedPageCount);
		m_pProgress->SetDownloadedPages(m_nGottenPageCount);
		m_pProgress->SetDownloadedFiles(m_nGottenFileCount);
		m_pProgress->SetKBDownloaded(m_nTotalBytes);
		m_pProgress->SetLevel(m_nLevel+1);
	}
}

// ������ҳ�Լ���ҳ�еĶ�ý��Ԫ��
UINT CSnaggerDoc::DownloadThread(LPVOID lpvData)
{
	HTREEITEM htreePage;

	CSnaggerDoc *pThis = (CSnaggerDoc *) lpvData;

	int nMaxDepth = pThis->m_Options.nMaxDepth-1;
	int nCount;
	CString strPage = pThis->m_strStartPage;
	CString strFileName;
	CString strLogData;
	CString strText;
	POSITION pos = pThis->GetFirstViewPosition();
	CSnaggerView* pView = (CSnaggerView *) pThis->GetNextView(pos);	
	BOOL bIsOffsite = FALSE;

	// ����WinInet�Ự
	try
	{
		pThis->m_Inet.OpenSession(pThis->m_Options.bUseProxy,pThis->m_Options.strProxyName);
	}
	catch(...)
	{
	}

	// ������־�ļ�
	pThis->m_fileLog.Open(pThis->m_strDirectory+"sitesnag.log",
						CFile::modeCreate|CFile::modeWrite);

	// ���������б��ļ�
	if(pThis->m_Options.bContents)
	{
		pThis->m_fileContents.Open(pThis->m_strDirectory+"SnagCon1.htm",
								CFile::modeCreate|CFile::modeWrite);

		// �������������ӵ������ļ��б���
		pThis->SetPageCacheEntry("snagcon1.htm","SnagCon1.htm",0);

		// �����б���뵽���οؼ���
		CString strTitle = "Contents Page 1 (SnagCon1.htm)";
		pView->AddTreeContent(strTitle);

		// д���һ�������б�Ҳ�Ŀ�ʼ
		strText = "<HTML>\r\n<HEAD>\r\n<TITLE>SiteSnagger Contents</TITLE>\r\n";
		strText += "</HEAD\r\n<BODY>\r\n";
		strText += "<H1><center>SiteSnagger Table of Contents</center><br><br></H1>\r\n<UL>\r\n";
		pThis->m_fileContents.Write(strText,strText.GetLength());
	}

	// ��ʼ����һ���ļ���������
	//�ӵ�һ�㿪ʼ
	pThis->m_nLevel = 0;
	pThis->m_aLinks[0].nIndex = 0;
	pThis->m_Inet.ResetUniqueCount();

	// ������������֪���Ѿ������������
	//���߱���Ϊֹͣ
	while(pThis->m_nLevel >= 0 && !pThis->m_pProgress->IsAborted())
	{
		// ��õڶ�������ҳ����
		if(pThis->m_aLinks[pThis->m_nLevel].nIndex > 0)
		{
			// ���汾ҳ��UR�����ж��Ƿ��Ǳ�վ��ҳ
			int nIndex = pThis->m_aLinks[pThis->m_nLevel].nIndex;
			strPage = pThis->m_aLinks[pThis->m_nLevel].arrLinks.GetAt(nIndex);
			bIsOffsite = pThis->m_aLinks[pThis->m_nLevel].arrOffsite.GetAt(nIndex);

			// ���ӵ���һ��
			pThis->m_nLevel++;
		}

		// Ϊ��ҳ����һ��Ψһ������
		pThis->m_Inet.GenerateUniqueFileName(strPage,strFileName,
						pThis->m_arrPagesDone,TRUE);
		pThis->m_pProgress->SetActionTitle("Getting Page: "+strPage);

		// Ϊ��ҳдһ����־��ڣ�����д����
		strLogData.Format("[%02d] Getting page %s ",pThis->m_nLevel+1,strPage);
		pThis->m_fileLog.Write(strLogData,strLogData.GetLength());

		CString strOrigPage = strPage;

		// ��ø�ҳ��Inet�����ļ�
		if(pThis->GetPage(strPage,strFileName,pThis->m_aLinks[pThis->m_nLevel]))
		{
			MAP_FILES *pMapEntry;
		
			// �����������
			nCount = pThis->m_aLinks[pThis->m_nLevel].arrLinks.GetSize();

			// �Ƿ�ֻ��Ҫ�����ļ�
			if(pThis->ShouldGetPage(strPage,pMapEntry))
			{
				// ���ӵ��Ѿ�����ҳ����б���
				pThis->SetPageCacheEntry(strPage,strFileName,pThis->m_nLevel);

				// ���ҳ�汻���¶�����
				//Ҳ���ԭʼ����
				if(strPage != strOrigPage && pThis->ShouldGetPage(strOrigPage,pMapEntry))
					pThis->SetPageCacheEntry(strOrigPage,strFileName,pThis->m_nLevel);

				// ���������վ�����ҳ������ǰ׺������������
				if(bIsOffsite)
					strText = strPage+" - ";
				else strText.Empty();

				// ����ҳ������ƺ��ļ���
				strText += pThis->m_strPageTitle+"  ("+
							strFileName.SpanExcluding("#")+")";

				htreePage = pView->AddTreePage(strText,bIsOffsite);
				strText.Format("<a href=%s><li> %s (%s - %s)<br>\r\n",strFileName,
									pThis->m_strPageTitle,
									strFileName.SpanExcluding("#"),strPage);
				if(pThis->m_Options.bContents)
					pThis->m_fileContents.Write(strText,strText.GetLength());

				// ���²���
				pThis->m_nGottenPageCount++;
				pThis->m_nGottenFileCount++;
				pThis->UpdateStatus();
			}
			else
			{
				// �����µ���������
				if(nMaxDepth)
				{
					// �Ƿ��Ѿ�����������
					if(pThis->m_nLevel >= pMapEntry->nMaxLevel)
						nCount = 0;
					else pMapEntry->nMaxLevel = pThis->m_nLevel;
				}
			}

			// �����д����־�ļ�
			pThis->m_fileLog.Write("[OK]\n",5);

			// �������վ�������
			if(bIsOffsite)
				nCount = 0;

			// �Ƿ����ض�ý���ļ�
			if(pThis->m_Options.bMultimedia)
			{
				// ������ý���ļ������б�
				CString strMedia;
				for(int j = 0; j < pThis->m_aLinks[pThis->m_nLevel].arrMedia.GetSize() &&
							!pThis->m_pProgress->IsAborted(); j++)
				{
					strMedia = pThis->m_aLinks[pThis->m_nLevel].arrMedia.GetAt(j);

					// �Ƿ��ȡ���ļ�
					if(pThis->ShouldGetMedia(strMedia,pMapEntry))
					{
						// ��֤��һ��Ψһ���ļ���
						pThis->m_Inet.GenerateUniqueFileName(strMedia,
										strFileName,pThis->m_arrMediaDone,FALSE);
						pThis->m_pProgress->SetActionTitle("Getting File: "+strFileName);

						// д����־�ļ�
						strLogData.Format("[%02d] Getting media %s ",pThis->m_nLevel,
														strMedia);
						pThis->m_fileLog.Write(strLogData,strLogData.GetLength());

						// ����EMAIL����
						BOOL bMail;
						if(strMedia.Left(7) == "mailto:")
						{
							bMail = TRUE;
							strFileName = strMedia;
						}
						else bMail = FALSE;

						//����ɹ�
						if(bMail || pThis->GetMedia(strMedia,strFileName))
						{
							//���ļ����ӵ��ļ��б���
							pThis->SetMediaCacheEntry(strMedia,strFileName);
							pView->AddTreeMedia(strFileName.SpanExcluding("#"),
											CTree::GetMediaType(strFileName));

							// ����̬��������
							if(!bMail)
								pThis->m_nGottenFileCount++;
							pThis->UpdateStatus();

							//�����д����־
							pThis->m_fileLog.Write("[OK]\n",5);
						}
						else
						{
							// ��ʧ�ܽ��д����־
							pThis->m_fileLog.Write("[FAILED] ",9);

							// д����ϸ�Ĵ�����־
							CString strError = pThis->m_Inet.GetErrorText();
							pThis->m_fileLog.Write(strError,strError.GetLength());
							pThis->m_fileLog.Write("\n",1);
						}
					}
				}
			}
		}
		else
		{
			// д����־
			pThis->m_fileLog.Write("[FAILED] ",9);

			// ��ʾ��ϸ�Ĵ���
			CString strError = pThis->m_Inet.GetErrorText();
			pThis->m_fileLog.Write(strError,strError.GetLength());
			pThis->m_fileLog.Write("\n",1);
			
			nCount = 0;
		}

		// ���²���
		pThis->UpdateStatus();

		// ����������趨�����ҳ�������˳�
		if(pThis->m_Options.nMaxPages > 0 && 
				pThis->m_nGottenPageCount >= pThis->m_Options.nMaxPages) 
			break;

		// ֻҪ�������ӣ����������
		if(pThis->m_nLevel < nMaxDepth && nCount > 0)
		{
			// �����һ��ҳ��
			strPage = pThis->m_aLinks[pThis->m_nLevel].arrLinks.GetAt(0);
			bIsOffsite = pThis->m_aLinks[pThis->m_nLevel].arrOffsite.GetAt(0);

			// ������һ�㣬����ʼ��
			pThis->m_nLevel++;
			pThis->ResetLink(pThis->m_nLevel);

			// Queue the links
			pThis->m_nQueuedPageCount += nCount;
			continue;
		}

		// ����������ӣ�����������Ϣ
		pThis->ResetLink(pThis->m_nLevel);

		// �Ƶ�ǰһ��
		pThis->m_nLevel--;

		// Ѱ����һҳ
		if(pThis->m_nLevel >= 0)
		{
			int nMaxCount;

			// 
			while(pThis->m_nLevel >= 0)
			{
				// ��õڶ�����������
				nMaxCount = pThis->m_aLinks[pThis->m_nLevel].arrLinks.GetSize();

				// �Ƿ��������Ϸ���ҳ������һ��
				if(pThis->m_aLinks[pThis->m_nLevel].nIndex < nMaxCount-1)
				{
					// �����һ��ҳ��
					pThis->m_aLinks[pThis->m_nLevel].nIndex++;
					pThis->m_nQueuedPageCount--;
					break;
				}
				else 
				{
					// �����οؼ��ϻص�ǰһ��
					pThis->m_nLevel--;
					pThis->m_nQueuedPageCount--;
				}
			}
		}
	}

	// Make sure the "stopping, please wait" message isn't displayed
	pView->EndWait();

	
	// ȷ��nNodeCount����0
	pThis->m_nLevel = pThis->m_nQueuedPageCount;

	// �Ƿ���Ҫ��������
	if(pThis->m_Options.bFixupLinks)
	{
		// �û�ȡ������
		if(pThis->m_pProgress->IsAborted() && 
			pView->ShowPrompt(IDS_FIXUP_PAGES,MB_ICONQUESTION|MB_YESNO) != IDYES)
			;
		else
		{
			// Everything was normal so just fixup the links
			pThis->m_pProgress->SetAborted(FALSE);
			pThis->FixupLinks();
		}
	}

	//  �ر���־�ļ�
	pThis->m_fileLog.Close();

	// Are we doing the TOC??
	if(pThis->m_Options.bContents)
	{
		// д���ļ���β��
		CString strText;
		strText = "</UL>\r\n</BODY>\r\n</HTML>\r\n";
		pThis->m_fileContents.Write(strText,strText.GetLength());
		pThis->m_fileContents.Close();
	}

	// �����Ϣ��ʾ�Ѿ����
	pThis->m_pProgress->SetActionTitle("********* ������� *********");
	pView->SetSnagging(FALSE);

	// �ͷ��ڴ�
	pThis->ClearCacheMaps();

	// Make sure the "stopping, please wait" message isn't displayed
	//pView->EndWait();

	// ������Զ�ģʽ���ر�ס����
	if(pThis->m_bAutoMode)
		AfxGetMainWnd()->PostMessage(WM_CLOSE);

	// �ر�WinInet�Ự
	try
	{
		pThis->m_Inet.CloseSession();
	}
	catch(...)
	{
	}

	pThis->m_nLevel = 0;

	return(0);
}

//�ͷ���Դ���������ļ�����Լ��ļ������б�
void CSnaggerDoc::ClearCacheMaps()
{
	MAP_FILES *pMapEntry;
	CString strPage;
	POSITION pos;
	int i;

	// ɾ��ҳ���б�
	pos = m_arrPagesDone.GetStartPosition();
	for(i = 0; i < m_arrPagesDone.GetCount(); i++)
	{
		m_arrPagesDone.GetNextAssoc(pos,strPage,(CObject *&) pMapEntry);
		delete pMapEntry;
	}
	m_arrPagesDone.RemoveAll();

	// ɾ����ý��ҳ���б�
	pos = m_arrMediaDone.GetStartPosition();
	for(i = 0; i < m_arrMediaDone.GetCount(); i++)
	{
		m_arrMediaDone.GetNextAssoc(pos,strPage,(CObject *&) pMapEntry);
		delete pMapEntry;
	}
	m_arrMediaDone.RemoveAll();
}

// ����һ����������ҳ���ļ��б�����
void CSnaggerDoc::SetPageCacheEntry(LPCTSTR lpszPage, LPCTSTR lpszFileName, int nLevel)
{
	CString strTempPage = lpszPage;
	MAP_FILES *pMapEntry = new MAP_FILES();

	// ����ҳ������Сд����
	pMapEntry->strFileName = lpszFileName;
	pMapEntry->nMaxLevel = nLevel;
	strTempPage.MakeLower();
	strTempPage = strTempPage.SpanExcluding("#");

	// ������ҳ����һ�����·��
	CString strExt = CInet::SplitFileName(strTempPage,CInet::EXT);
	if(strExt.IsEmpty() && strTempPage.Right(1) != "/")
		strTempPage += "/";

	// Assimilate it into the collective
	m_arrPagesDone.SetAt(strTempPage,(CObject *&) pMapEntry);
	TRACE("Pages Done: "+strTempPage+"\n");

	// �趨Ĭ������
	if(pMapEntry->strFileName == "index.htm" && strTempPage.Find("index.htm") == -1)
	{
		// ���õ�Ĭ��ҳ�ļ���
		char *aszDefPages[4] = { "index.htm", "index.html", "default.htm", 
										"default.html"};

		MAP_FILES *pTempMapEntry;

		CString strServer, strObject, strUser, strPassword;
		INTERNET_PORT nPort;
		DWORD dwServiceType;
		AfxParseURLEx(strTempPage,dwServiceType,strServer,strObject,nPort,
									strUser,strPassword,
									ICU_NO_ENCODE);

		// ����Ĭ���ʾ�����
		if(!strServer.IsEmpty())
		{
			strServer = "http://"+strServer+"/";

			for(int i = 0; i < 4; i++)
			{
				pTempMapEntry = new MAP_FILES();
				pTempMapEntry->strFileName = pMapEntry->strFileName;
				pTempMapEntry->nMaxLevel = pMapEntry->nMaxLevel;
				m_arrPagesDone.SetAt(strServer+aszDefPages[i],(CObject *&) pTempMapEntry);
				TRACE("Pages Done: "+CString(aszDefPages[i])+"\n");
			}
		}
	}	
}

// ����һ���µ��ļ��б�����
void CSnaggerDoc::SetMediaCacheEntry(LPCTSTR lpszMedia, LPCTSTR lpszFileName)
{
	CString strTempMedia = lpszMedia;
	MAP_FILES *pMapEntry = new MAP_FILES();

	// �ļ��������Сд
	pMapEntry->strFileName = lpszFileName;
	pMapEntry->nMaxLevel = -1;
	strTempMedia.MakeLower();
	strTempMedia = strTempMedia.SpanExcluding("#");

	// Assimilate it into the collective
	m_arrMediaDone.SetAt(strTempMedia,(CObject *&) pMapEntry);
}

//��ʼ������վ����
void CSnaggerDoc::RecursiveDownload(LPCTSTR lpszURL)
{
	// ��ʼ������
	m_nGottenPageCount = 0;
	m_nGottenFileCount = 0;
	m_nQueuedPageCount = 0;
	m_nTotalBytes = 0;
	m_strStartPage = lpszURL;
	ClearCacheMaps();

	// ��ʼ��������ʾ����
	POSITION pos = GetFirstViewPosition();
	CSnaggerView* pView = (CSnaggerView *) GetNextView(pos);
	m_pProgress = pView->GetProgress();

	m_pProgress->SetAborted(FALSE);
	UpdateStatus();

	// ���������߳�
	CWinThread *pThread = AfxBeginThread(DownloadThread,this);
}

// Fixs up each of the downloaded pages to allow local browsing.  It
// basically goes through the downloaded page list and opens each 
// file specified then parses the page and fixes up each of the 
// links found.
//
void CSnaggerDoc::FixupLinks()
{
	BYTE *pbyBuffer = m_byBuffer;
	int nLen;
	CFile fileIn, fileOut;
	CFileException ex;
	MAP_FILES* pMapEntry;
	int nIndex;
	int nBytesWritten;
	int nSectionLength;
	int nOffset;
	CString strFixupFileName;
	CString strPage;
	CString strFileName;
	CString strTempName = m_strDirectory+"$ssnag$.htm";
	BOOL bFixup;
	int nFiles = m_arrPagesDone.GetCount();
	BOOL bIndexDone = FALSE;

	//���²�����ʾ����ָʾ��δ������ļ�����
	m_pProgress->SetQueuedFiles(nFiles);

	// Iterate through the downloaded page list
	//�������ҳ�б�
	POSITION pos = m_arrPagesDone.GetStartPosition();
	for(int i = 0; i < m_arrPagesDone.GetCount() && !m_pProgress->IsAborted(); i++)
	{
		m_arrPagesDone.GetNextAssoc(pos,strPage,(CObject *&) pMapEntry);

		if(pMapEntry->strFileName == "index.htm" || 
						pMapEntry->strFileName == "index.html")
		{
			if(bIndexDone)
			{
				// ���²�����ʾ����
				m_pProgress->SetQueuedFiles(--nFiles);
				continue;
			}	
			else bIndexDone = TRUE;
		}

		//��ʾ����
		strFileName = m_strDirectory+pMapEntry->strFileName;
		m_pProgress->SetActionTitle("Fixing up links in file: "+pMapEntry->strFileName);

		// �򿪱����ļ�
		TRACE("Fixing up: "+strFileName+"\n");
		if(fileIn.Open(strFileName.SpanExcluding("#"),CFile::modeRead,&ex))
		{
			pbyBuffer = m_byBuffer;

			// ���ļ������ڴ�
			nLen = fileIn.Read(pbyBuffer,MAX_INET_BUFFER);
			fileIn.Close();

			// Parse it (tell the parser to look for only links, i.e. fixup mode)
			CSnaggerHtmlParser Parser;
			Parser.SetFixupMode(TRUE);
			Parser.ResetArrays();
			Parser.SetGetMedia(m_Options.bMultimedia);
			Parser.SetPageURL(strPage);
			Parser.ParseText((char *)pbyBuffer,nLen);

			CString strServer, strObject, strUser, strPassword;
			INTERNET_PORT nPort;
			DWORD dwServiceType;
			AfxParseURLEx(strPage,dwServiceType,strServer,strObject,nPort,
									strUser,strPassword,
									ICU_NO_ENCODE);
			CString strOrigHost = "http://"+strServer+"/";

			// �򿪸���ʱ�ļ�
			if(fileOut.Open(strTempName,CFile::modeCreate|CFile::modeWrite,&ex))
			{
				CString strURL;

				nIndex = 0;
				nBytesWritten = 0;
				nOffset = 0;

				// д���ļ�������(����б�Ҫ��
				for(int j = 0; j < Parser.GetFixups().arrURL.GetSize(); j++)
				{
					strURL = Parser.GetFixups().arrURL.GetAt(j);

					// �Ƿ��������ı��ļ�
					if(Parser.GetFixups().arrTextPage.GetAt(j))
					{
						// �Ƿ�Ҫ��������ļ�
						if(!ShouldGetPage(strURL,pMapEntry))
						{
							// ��ñ����ļ�
							strFixupFileName = pMapEntry->strFileName;
							bFixup = TRUE;
						}
						else
						{							
							AfxParseURLEx(strURL,dwServiceType,strServer,
									strObject,nPort,strUser,strPassword,
									ICU_NO_ENCODE);

							if(!strServer.IsEmpty())
							{
								strFixupFileName = strURL;
								bFixup = TRUE;
							}
							else bFixup = FALSE;
						}
					}
					else // ��ý���ļ�����
					{
						// �Ƿ����ض�ý���ļ�
						if(!ShouldGetMedia(strURL,pMapEntry))
						{
							// ��ȡ�ļ���
							strFixupFileName = pMapEntry->strFileName;
							bFixup = TRUE;
						}
						else bFixup = FALSE;
					}

					// д������
					nSectionLength = Parser.GetFixups().arrIndex.GetAt(j)-nIndex-nOffset;

					try
					{
						fileOut.Write(pbyBuffer,nSectionLength);
					}
					catch(...)
					{
					}

					nBytesWritten += nSectionLength;
					pbyBuffer += nSectionLength;
					nOffset = 0;

					// �Ƿ�����������
					if(bFixup)
					{
						// д���ļ�
						try
						{
							fileOut.Write(" \"",2);
							fileOut.Write(strFixupFileName,strFixupFileName.GetLength());
						}
						catch(...)
						{
						}

						// �ҵ�ԭ���ļ��Ŀ�ʼ��
						while(*pbyBuffer == ' ') 
						{
							pbyBuffer++;
							nBytesWritten++;
							nOffset++;
						}

						// Is the filename in quotes?
						if(*pbyBuffer == '"') 
						{
							// �ҵ����
							do
							{
								pbyBuffer++;
								nBytesWritten++;
								nOffset++;
							}
							while(*pbyBuffer != '"');

							pbyBuffer++;
							nBytesWritten++;
							nOffset++;
						}
						else
						{
							// Not in quotes, so just look for the first
							// trailing blank
							do
							{
								pbyBuffer++;
								nBytesWritten++;
								nOffset++;
							}
							while(*pbyBuffer != ' ');
						}

						// Write a trailing quote and blank, just to be safe
						fileOut.Write("\" ",2);
					}

					// ���濪ʼ�����λ��
					nIndex = Parser.GetFixups().arrIndex.GetAt(j);
				}

				// �������Ȼ��д��ʣ�������
				try
				{
					if(nLen-nBytesWritten > 0)
						fileOut.Write(pbyBuffer,nLen-nBytesWritten);
				}
				catch(...)
				{
				}

				fileOut.Close();
			}
		}

		// ����������ļ����ԭʼ�ļ�
		remove(strFileName.SpanExcluding("#"));
		rename(strTempName,strFileName.SpanExcluding("#"));

		// ���²�����ʾ����
		m_pProgress->SetQueuedFiles(--nFiles);
	}

	// ɾ����ʱ�ļ�����Ϊ�п����м��û�ҪӲ��ֹͣ
	remove(strTempName);
}

/////////////////////////////////////////////////////////////////////////////
// CSnaggerDoc diagnostics

#ifdef _DEBUG

// Check validity of object
void CSnaggerDoc::AssertValid() const
{
	CDocument::AssertValid();
}

// Dump the object
void CSnaggerDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

