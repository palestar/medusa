#if !defined(AFX_PORTVIEW_H__1A974376_8F42_11D2_A4ED_00C04F6FF8CE__INCLUDED_)
#define AFX_PORTVIEW_H__1A974376_8F42_11D2_A4ED_00C04F6FF8CE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PortView.h : header file
//

#include "resource.h"

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
	CButton	m_cEditButton;
	CButton	m_cPlayButton;
	CString	m_sRate;
	CString	m_sChannels;
	CString	m_sBytes;
	CString	m_sTime;
	CString	m_sBits;
	//}}AFX_DATA

	void	UpdateTempFile();
	CString	m_sTempFile;

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
	afx_msg void OnPlay();
	afx_msg void OnImport();
	afx_msg void OnExport();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PORTVIEW_H__1A974376_8F42_11D2_A4ED_00C04F6FF8CE__INCLUDED_)
