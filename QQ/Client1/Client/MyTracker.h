#pragma once





#define CRIT_RECTTRACKER    5

void AFXAPI AfxUnlockGlobals(int nLockType);



class CMyTracker
{
public:
	//��ǰ������ɫ
	COLORREF m_rectColor;
	//���õ������
	void SetResizeCursor(UINT nID_N_S,UINT nID_W_E,UINT nID_NW_SE,
		UINT nID_NE_SW,UINT nIDMiddle);
		CRect m_rectLast;
		void GetTrueRect(LPRECT lpTrueRect) ;
		//��������ˢ,�ڲ�����,ɶ��˼
		void CMyTracker::CreatePen();
		//���þ�����ɫ
		void SetRectColor(COLORREF rectColor);
		virtual int GetHandleSize(LPCRECT lpRect = NULL) ;
		void CMyTracker::Draw(CDC* pDC) ;

	// Style Flags
	enum StyleFlags
	{
		solidLine = 1, dottedLine = 2, hatchedBorder = 4,
		resizeInside = 8, resizeOutside = 16, hatchInside = 32,
		resizeMiddle =80 //�����м�
	};
	// Attributes
	UINT m_nStyle;      // current state
	CRect m_rect;       // current position (always in pixels)
	int m_nHandleSize;  // size of resize handles (default from WIN.INI)
	


	//**************************************************************

	CMyTracker(void);
	~CMyTracker(void);
};

