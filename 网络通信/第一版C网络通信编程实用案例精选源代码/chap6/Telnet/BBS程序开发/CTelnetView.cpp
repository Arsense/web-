// CTelnetView.cpp : implementation of the CTelnetView class
//

#include "stdafx.h"
#include "CTelnet.h"

#include "CTelnetDoc.h"
#include "CTelnetView.h"
#include "MainFrm.h"
#include "ClientSocket.h"
#include "Process.h"

#include "HostDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern CMultiDocTemplate * pDocTemplate;


/////////////////////////////////////////////////////////////////////////////
// CTelnetView

IMPLEMENT_DYNCREATE(CTelnetView, CScrollView)

BEGIN_MESSAGE_MAP(CTelnetView, CScrollView)
	//{{AFX_MSG_MAP(CTelnetView)
	ON_WM_CHAR()
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_WM_KEYDOWN()

	//}}AFX_MSG_MAP
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CScrollView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CScrollView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CScrollView::OnFilePrintPreview)
	ON_COMMAND(ID_FILE_NEW, OnFileNew)

END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTelnetView construction/destruction

CTelnetView::CTelnetView()
{
	cTextColor = RGB(0,200,000);
	cBackgroundColor = RGB(000,000,000);
	cSock = NULL;
	bOptionsSent = FALSE;
	TempCounter = 0;
	cCursX = 0;
	CurrentXX=0;
	CurrentYY=0;

	IfOutput=false;
	dtX=8;
	dtY=20;

//	OffsetNum=0;
	for(int x = 0; x < 80; x++)
	{
		for(int y = 0; y < bufferLines; y++)
		{
			cText[x][y] = ' ';
		}
	}
}

CTelnetView::~CTelnetView()
{
	if(cSock != NULL)
		delete cSock;
	cSock = NULL;
}

BOOL CTelnetView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CScrollView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CTelnetView drawing

void CTelnetView::OnDraw(CDC* pDC)
{
	CTelnetDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

//	pDC->SelectObject(GetStockObject(ANSI_FIXED_FONT));
	/*
	for(int x = 0; x < 80; x++)
	{
		for(int y = 0; y < bufferLines; y++)
		{
			cText[x][y] = ' ';
		}
		pDC->TextOut(0,Text);
	}
	*/
//	OnEraseBkgnd(pDC);
	CRect rect;
	GetClientRect(&rect);
	pDC->FillSolidRect(rect, cBackgroundColor);
//	ClearWindows(pDC);
	DrawCursor(pDC,FALSE);
	DoDraw(pDC);
	DrawCursor(pDC,TRUE);
}

void CTelnetView::DoDraw(CDC* pDC)
{
	CString strLine;
	BOOL bSkip = FALSE;
	CRect clip;
	pDC->GetClipBox(clip);
	clip.top -= dtY;

	pDC->SetTextColor(cTextColor);
	CFont font;

	static BOOL isinit=TRUE;
	if(isinit==TRUE)
	{
		CFont * t=pDC->GetCurrentFont();
		t->GetLogFont(&m_lplf);
		isinit=FALSE;
	}
	font.CreateFontIndirect(&m_lplf);
	pDC->SelectObject(&font);


//	CurrentXX=0;
	char text[2] = {0x00, 0x00};

	for(int y = 0; y < bufferLines; y++)
	{
		//if(y * dtY >= clip.top)
		//{
			for(int x = 0; x < 80; x++)
			{
				text[0] = cText[x][y];
				if(text[0] == 27)
					bSkip = TRUE;
				if(!bSkip)
					strLine += text[0];
				if(text[0] == 'm' && bSkip)
					bSkip = FALSE;
			}
			pDC->TextOut(0, y * dtY, strLine);
			strLine.Empty();
		//}
	}
}

void CTelnetView::OnInitialUpdate()
{
	CSize sizeTotal;
	

	// TODO: calculate the total size of this view
	sizeTotal.cx = dtX * 80 + 3;
	sizeTotal.cy = dtY * bufferLines + 3;
	SetScrollSizes(MM_TEXT, sizeTotal);
	//SetWindowPos(NULL,0,0,sizeTotal.cx,sizeTotal.cy,SWP_NOMOVE);

//	ScrollToPosition(CPoint(0, bufferLines * 1000)); //go way past the end


	CScrollView::OnInitialUpdate();

}

/////////////////////////////////////////////////////////////////////////////
// CTelnetView printing

BOOL CTelnetView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CTelnetView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CTelnetView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

/////////////////////////////////////////////////////////////////////////////
// CTelnetView diagnostics

#ifdef _DEBUG
void CTelnetView::AssertValid() const
{
	CScrollView::AssertValid();
}

void CTelnetView::Dump(CDumpContext& dc) const
{
	CScrollView::Dump(dc);
}

CTelnetDoc* CTelnetView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CTelnetDoc)));
	return (CTelnetDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CTelnetView message handlers


//���շ�������
void CTelnetView::ProcessMessage(CClientSocket * pSock)
{
	if(!IsWindow(m_hWnd)) return;
	if(!IsWindowVisible()) return;
	//�������ݵ�m_bBuf
	int nBytes = pSock->Receive(m_bBuf ,ioBuffSize );
	if(nBytes != SOCKET_ERROR)
	{
		int ndx = 0;
		//ÿ�ζ���һ������
		while(GetLine(m_bBuf, nBytes, ndx) != TRUE);
		//����Э��
		ProcessOptions();
		MessageReceived(m_strNormalText);
	}
	m_strLine.Empty();
	m_strResp.Empty();
}

//����Э��
void CTelnetView::ProcessOptions()
{
	CString m_strTemp;
	CString m_strOption;
	unsigned char ch;
	int ndx;
	int ldx;
	BOOL bScanDone = FALSE;

	m_strTemp = m_strLine;

	while(!m_strTemp.IsEmpty() && bScanDone != TRUE)
	{
		ndx = m_strTemp.Find(IAC);
		if(ndx != -1)
		{
			m_strNormalText += m_strTemp.Left(ndx);
			ch = m_strTemp.GetAt(ndx + 1);
			switch(ch)
			{
			case DO:
			case DONT:
			case WILL:
			case WONT:
				m_strOption		= m_strTemp.Mid(ndx, 3);
				m_strTemp		= m_strTemp.Mid(ndx + 3);
				m_strNormalText	= m_strTemp.Left(ndx);
				m_ListOptions.AddTail(m_strOption);
				break;
			case IAC:
				m_strNormalText	= m_strTemp.Left(ndx);
				m_strTemp		= m_strTemp.Mid(ndx + 1);
				break;
			case SB:
				m_strNormalText = m_strTemp.Left(ndx);
				ldx = Find(m_strTemp, SE);
				m_strOption		= m_strTemp.Mid(ndx, ldx);
				m_ListOptions.AddTail(m_strOption);
				m_strTemp		= m_strTemp.Mid(ldx);
				//AfxMessageBox(m_strOption,MB_OK);
				break;
			default:
				bScanDone = TRUE;
			}
		}
		else
		{
			m_strNormalText = m_strTemp;
			bScanDone = TRUE;
		}
	} 
	
	RespondToOptions();
}


void CTelnetView::RespondToOptions()
{
	CString strOption;
	
	while(!m_ListOptions.IsEmpty())
	{
		strOption = m_ListOptions.RemoveHead();

		ArrangeReply(strOption);
	}

	DispatchMessage(m_strResp);
	m_strResp.Empty();
}

void CTelnetView::ArrangeReply(CString strOption)
{

	unsigned char Verb;
	unsigned char Option;
	unsigned char Modifier;
	unsigned char ch;
	BOOL bDefined = FALSE;

	if(strOption.GetLength() < 3) return;

	Verb = strOption.GetAt(1);
	Option = strOption.GetAt(2);

	switch(Option)
	{
	case 1:	//����
	case 3: // Suppress Go-Ahead
		bDefined = TRUE;
		break;
	}

	m_strResp += IAC;

	if(bDefined == TRUE)
	{
		switch(Verb)
		{
		case DO:
			ch = WILL;
			m_strResp += ch;
			m_strResp += Option;
			break;
		case DONT:
			ch = WONT;
			m_strResp += ch;
			m_strResp += Option;
			break;
		case WILL:
			ch = DO;
			m_strResp += ch;
			m_strResp += Option;
			break;
		case WONT:
			ch = DONT;
			m_strResp += ch;
			m_strResp += Option;
			break;
		case SB:
			Modifier = strOption.GetAt(3);
			if(Modifier == SEND)
			{
				ch = SB;
				m_strResp += ch;
				m_strResp += Option;
				m_strResp += IS;
				m_strResp += IAC;
				m_strResp += SE;
			}
			break;
		}
	}

	else
	{
		switch(Verb)
		{
		case DO:
			ch = WONT;
			m_strResp += ch;
			m_strResp += Option;
			break;
		case DONT:
			ch = WONT;
			m_strResp += ch;
			m_strResp += Option;
			break;
		case WILL:
			ch = DONT;
			m_strResp += ch;
			m_strResp += Option;
			break;
		case WONT:
			ch = DONT;
			m_strResp += ch;
			m_strResp += Option;
			break;
		}
	}
}

//��������
void CTelnetView::DispatchMessage(CString strText)
{
	if(cSock!=NULL)
		cSock->Send(strText, strText.GetLength());
}

//���һ������
BOOL CTelnetView::GetLine( unsigned char * bytes, int nBytes, int& ndx )
{
	BOOL bLine = FALSE;
	while ( bLine == FALSE && ndx < nBytes )
	{
		unsigned char ch = bytes[ndx];
		//ԭ����Ƶ�ʱ��Ҫȥ���س����еģ����Ǻ������ֲ���ȥ��
		switch( ch )
		{
		case '\r': // 
			m_strLine += "\r"; //�س�
			break;
		case '\n': // �н�β
			m_strLine += "\n";
			break;
		default:   // ��������
			m_strLine += ch;
			break;
		} 

		ndx ++;


		if (ndx == nBytes)
		{
			bLine = TRUE;
		}
	}
	return bLine;
}

//���ݴ���
void CTelnetView::MessageReceived(LPCSTR pText)
{
	BOOL bSkip = FALSE;
	int loop=0;
	CString tempStr="0123456789;";
	CString tempStr2;
	int ColorVal;
	int tempY=0;

	CDC * pDC = GetDC();
	OnPrepareDC(pDC);
	DrawCursor(pDC,FALSE);

	CMainFrame * frm = (CMainFrame*)GetTopLevelFrame();
	//������ɫ
	pDC->SetTextColor(cTextColor);
	pDC->SetBkColor(cBackgroundColor);

	CFont font;
	font.CreateFontIndirect(&m_lplf);
	pDC->SelectObject(&font);

//	pDC->SelectObject(GetStockObject(ANSI_FIXED_FONT));
	int length = strlen(pText);
	char text[2] = {0x00, 0x00};
	while(loop < length)
	{
		switch(pText[loop])
		{
		case 8: //ɾ��
			CurrentXX--;
			if(CurrentXX < 0) CurrentXX = 0;
			loop++;
			break;
		case 9: //TAB��
			CurrentXX++; //TBD make this smarter
			loop++;
			break;
		case 13: //����CR
			m_strline.Empty();
			CurrentXX = 0;
			loop++;
			break;
		case 27:
			loop++;
			//����������27���ַ��Ƿ���91���������91�����������ַ������������ˣ�ֱ������
			if (pText[loop]!=91) 
			{
				loop++;
				break;
			}
			//�����91�������������������ϵͳ�������
			else
			{
				loop++;
				while (tempStr.Find(pText[loop])!=-1) 
				{
					tempStr2+=pText[loop];
					loop++;
				}
				if (pText[loop]=='m')//�����������������m�������ǰ���õ��ַ���
				{

					//ѭ������ַ����е�ֵ�������ַ����е�ֵ�����ԷֺŸ�����
					while (tempStr2!="")
					{
						if (tempStr2.Find(";")!=-1)
						{
							ColorVal=atoi(tempStr2.Mid(0,tempStr2.Find(";")));
							tempStr2=tempStr2.Mid(tempStr2.Find(";")+1);

						}
						else
						{
							ColorVal=atoi(tempStr2);
							tempStr2.Empty();
						}

						//���һ��ֵ
						//�ı�ǰ����ɫ�������ɫ���԰���һ���Ĺ������Զ���
						
						if (ColorVal>29 && ColorVal<38)
							//cTextColor=RGB(0,0,255);
						//���ñ�����ɫ
						if (ColorVal>39 && ColorVal<48)
							//cBackgroundColor=RGB(0,255,ColorVal);
						//�ָ���������
						if (ColorVal==0)
						{
							//cBackgroundColor=RGB(0,0,0);
							//cTextColor=RGB(255,255,255);
						}
						//���Ϊ1,������ǰ��ɫ
						//if ColorVal==1
						//��ʾҪ��ɫ
						//if ColorVal==7

					}
				}
				//���Ϊ�ַ�K����ʾҪ��һ���Ա���ɫ�ľ�������
				if (pText[loop]=='K')
				{

					int x,y;
					CString myStr;
					//����ԭ��������

					//��������������Ϊ�Ա���ɫ���������൱���ƶ����
					//����������ı䵽Ŀǰ��λ��
					x=CurrentXX;
					y=CurrentYY;
					for (int l=CurrentXX;l<80;l++)
					{
						cText[l][CurrentYY]=' ';
						myStr+=' ';
					}
					pDC->TextOut(x*dtX, y * dtY, myStr);

					CurrentXX=x;
					CurrentYY=y;

				}
	            //����ַ�ΪC����ʾҪ�ı䵱ǰ�ĺ�����
				if (pText[loop]=='C')
				{
	            //��ú�����ĸı���
					if (tempStr2.Find(";")!=-1)
					{
						ColorVal=atoi(tempStr2.Mid(0,tempStr2.Find(";")));
						tempStr2=tempStr2.Mid(tempStr2.Find(";")+1);
					}
					else
					{
						ColorVal=atoi(tempStr2);
						tempStr2.Empty();
					}
					//Ȼ����������ֵ��ע������Ҫ�����ַ����
					CurrentXX=CurrentXX+ColorVal;
				}
				//����ַ�ΪH����ʾ�������ú������������
				if (pText[loop]=='H')
				{
                //���������ֵ���ڷ��������͵Ĺ����У��ȷ���������ֵ
				TRACE0("H");

				int tX=0,tY=0;
				//char buffer3[20];
					tY=atoi(tempStr2.Mid(0,tempStr2.Find(";")));
					tempStr2=tempStr2.Mid(tempStr2.Find(";")+1);
					//��ú�����ֵ��ע��������ֵ��û�м��ַ���ȵ�
					tX=atoi(tempStr2);
					if (tX>0 && tY>0)
					{
						CurrentYY=tY-1;
						CurrentXX=tX-1;
					}
				}

				//���Ϊ�ַ�J����ʾҪ���������Ļ
				if (pText[loop]=='J')
				{
					ClearWindows(pDC);
				}
			}
			loop++;
			IfOutput=false;
			break;
		case 0:
			loop++;
		case 10: //����
			{
				
				CurrentYY=CurrentYY+1;
				if (CurrentYY>=bufferLines)
				{
					for(int row = 0; row < bufferLines; row++)
					{
						for(int  col = 0; col < 80; col++)
						{
							cText[col][row] = cText[col][row+1];
						}
					}
					for(int  col = 0; col < 80; col++)
					{
						cText[col][bufferLines-1] = ' ';
					}
					CurrentYY=CurrentYY-1;
					DoDraw(pDC);
				}
			}
			loop++;
			break;

		default://�������
			{
				cText[CurrentXX][CurrentYY] = pText[loop];
				m_strline.Empty();
				for (int i=0;i<80;i++)
				{
					if (cText[i][CurrentYY]!=27)
						m_strline += cText[i][CurrentYY];
					else
						break;
				}
				pDC->TextOut(0, CurrentYY * dtY, m_strline);
				CurrentXX++;
			}
			tempStr2.Empty();
			loop++;

			break;
		}
	}
	DrawCursor(pDC,TRUE);
	ReleaseDC(pDC);
}

//��������
void CTelnetView::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	//�����س���
	if (nChar == VK_RETURN)
	{
//		CClientDC dc(this);
//		OnEraseBkgnd(&dc);
//		Invalidate();
		DispatchMessage("\r\n");
	}
	else
	{
		DispatchMessage(nChar);
	}
}

//�����
void CTelnetView::DrawCursor(CDC * pDC, BOOL pDraw)
{
	COLORREF color;
	CMainFrame * frm = (CMainFrame*)GetTopLevelFrame();
	if(pDraw) //draw
	{
		color = cTextColor;
	}
	else //erase
	{
		color = cBackgroundColor;
	}
	CRect rect(CurrentXX * dtX + 2, CurrentYY * dtY + 1, 
		CurrentXX * dtX + dtX - 2, CurrentYY * dtY + dtY -1);
	pDC->FillSolidRect(rect, color);
}

void CTelnetView::OnSize(UINT nType, int cx, int cy) 
{
	CScrollView::OnSize(nType, cx, cy);
	if(IsWindow(m_hWnd))
	{
		if(IsWindowVisible())
		{
			//ScrollToPosition(CPoint(0, bufferLines * 1000)); //go way past the end
		}
	}
}

BOOL CTelnetView::OnEraseBkgnd(CDC* pDC) 
{
	CRect clip;
	GetClientRect(&clip);
	CMainFrame * frm = (CMainFrame*)GetTopLevelFrame();
	pDC->FillSolidRect(clip,cBackgroundColor);
	return TRUE;
}

//�����ַ�
int CTelnetView::Find(CString str, char ch)
{
	char* data = str.GetBuffer(0);
	int len = str.GetLength();
	int i = 0;
	for(i = 0; i < len; i++)
	{
		if(data[i] == ch)
			break;
	}
	str.ReleaseBuffer();
	return i;
}

//���������
void CTelnetView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	unsigned char myChar[3];

	// TODO: Add your message handler code here and/or call default
	//�����
	if (nChar==37)
	{
		myChar[0]=27;
		myChar[1]=91;
		myChar[2]=68;

		DispatchMessage(myChar);
	}
	//�ҷ����
	else if (nChar==39)
	{
		myChar[0]=27;
		myChar[1]=91;
		myChar[2]=67;

		DispatchMessage(myChar);
	}
	//�Ϸ����
	else if (nChar==38)
	{
		myChar[0]=27;
		myChar[1]=91;
		myChar[2]=65;

		DispatchMessage(myChar);
	}
	//�·����
	else if (nChar==40)
	{
		myChar[0]=27;
		myChar[1]=91;
		myChar[2]=66;

		DispatchMessage(myChar);
	}
	CView::OnKeyDown(nChar, nRepCnt, nFlags);
	//MessageBox((char*)nChar);
}

//�����Ļ
void CTelnetView::ClearWindows(CDC *pDc)
{
	for(int x = 0; x < 80; x++)
	{
		for(int y = 0; y < bufferLines; y++)
		{
			cText[x][y] = ' ';
		}
	}
	DoDraw(pDc);
	CurrentYY=0;
	CurrentXX=0;
}

void CTelnetView::OnFileNew() 
{
	BOOL bOK;

	// TODO: Add your command handler code here
	//�����趨�������Ի���
	CHostDialog host;
	if(host.DoModal()!=IDOK)
		return;
	cHostName = host.m_HostName;

	//����socket
	if(cSock!=NULL)
	{
		delete cSock;
	}

	cSock = new CClientSocket(this);
	if(cSock != NULL)
	{
		bOK = cSock->Create();
		if(bOK == TRUE)
		{
			//���ӷ�����
			cSock->Connect(cHostName, host.m_port);
			//�趨����
			GetDocument()->SetTitle(cHostName);
			Sleep(90);
		}
		else
		{
			AfxMessageBox("���ܴ���socket!");
		}
	}
	else
	{
		AfxMessageBox("���ܴ���socket!");
	}
}

