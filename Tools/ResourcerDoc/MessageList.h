#if !defined(AFX_MESSAGELIST_H__FA3B20B4_810B_11D5_BA96_00C0DF22DE85__INCLUDED_)
#define AFX_MESSAGELIST_H__FA3B20B4_810B_11D5_BA96_00C0DF22DE85__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MessageList.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMessageList view

class CMessageList : public CListView
{
protected:
	CMessageList();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CMessageList)

// Attributes
public:
	int				m_LastMessageId;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMessageList)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CMessageList();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CMessageList)
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDblclk(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MESSAGELIST_H__FA3B20B4_810B_11D5_BA96_00C0DF22DE85__INCLUDED_)
