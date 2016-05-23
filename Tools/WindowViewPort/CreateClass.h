#if !defined(AFX_CREATECLASS_H__3814BE47_FF26_11D3_BA92_00C0DF22DE85__INCLUDED_)
#define AFX_CREATECLASS_H__3814BE47_FF26_11D3_BA92_00C0DF22DE85__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CreateClass.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CCreateClass dialog

#include "Resource.h"

class CCreateClass : public CDialog
{
// Construction
public:
	CCreateClass(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CCreateClass)
	enum { IDD = IDD_CREATE_CLASS };
	CString	m_Directory;
	CString	m_ClassName;
	CString	m_CPP;
	CString	m_H;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCreateClass)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CCreateClass)
	afx_msg void OnUpdateClassName();
	afx_msg void OnBrowseDirectory();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CREATECLASS_H__3814BE47_FF26_11D3_BA92_00C0DF22DE85__INCLUDED_)
