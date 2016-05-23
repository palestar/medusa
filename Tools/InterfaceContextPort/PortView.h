#if !defined(AFX_PORTVIEW_H__C4E41A01_0852_11D3_A4EF_00C04F6FF8CE__INCLUDED_)
#define AFX_PORTVIEW_H__C4E41A01_0852_11D3_A4EF_00C04F6FF8CE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PortView.h : header file
//

#include "Resource.h"

/////////////////////////////////////////////////////////////////////////////
// CPortView form view

#ifndef __AFXEXT_H__
#include <afxext.h>
#endif

class CPortView : public CFormView
{
protected:
	CPortView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CPortView)

// Form Data
public:
	//{{AFX_DATA(CPortView)
	enum { IDD = IDD_FORMVIEW };
	CListCtrl	m_Scenes;
	CComboBox	m_StartingScene;
	CString	m_DocumentClass;
	CString	m_Cursor;
	CString	m_Style;
	//}}AFX_DATA

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
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
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
	//{{AFX_MSG(CPortView)
	afx_msg void OnAddScene();
	afx_msg void OnRemoveScene();
	afx_msg void OnOpenScene();
	afx_msg void OnRenameScene(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnChangeStart();
	afx_msg void OnExecute();
	afx_msg void OnBrowseDocumentClass();
	afx_msg void OnListOpenScene(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnOpenCursor();
	afx_msg void OnBrowseCursor();
	afx_msg void OnOpenStyle();
	afx_msg void OnBrowseStyle();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PORTVIEW_H__C4E41A01_0852_11D3_A4EF_00C04F6FF8CE__INCLUDED_)
