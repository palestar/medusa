#if !defined(AFX_PORTVIEW_H__233B8C38_7ED5_493B_B045_EC7C4D3BA474__INCLUDED_)
#define AFX_PORTVIEW_H__233B8C38_7ED5_493B_B045_EC7C4D3BA474__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PortView.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPortView view

class CPortView : public CEditView
{
protected:
	CPortView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CPortView)

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPortView)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CPortView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CPortView)
	afx_msg void OnChange();
	afx_msg void OnKillfocus();
	afx_msg void OnEditCopy();
	afx_msg void OnEditCut();
	afx_msg void OnEditPaste();
	afx_msg void OnEditUndo();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PORTVIEW_H__233B8C38_7ED5_493B_B045_EC7C4D3BA474__INCLUDED_)
