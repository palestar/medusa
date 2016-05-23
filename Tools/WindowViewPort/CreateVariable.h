#if !defined(AFX_CREATEVARIABLE_H__910E9A04_FFF5_11D3_BA92_00C0DF22DE85__INCLUDED_)
#define AFX_CREATEVARIABLE_H__910E9A04_FFF5_11D3_BA92_00C0DF22DE85__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CreateVariable.h : header file
//

#include "Resource.h"

/////////////////////////////////////////////////////////////////////////////
// CCreateVariable dialog

class CCreateVariable : public CDialog
{
// Construction
public:
	CCreateVariable(CWnd* pParent = NULL);   // standard constructor

	CString m_PrototypeTemplate;

// Dialog Data
	//{{AFX_DATA(CCreateVariable)
	enum { IDD = IDD_CREATE_VARIABLE };
	CString	m_Prototype;
	CString	m_VariableName;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCreateVariable)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CCreateVariable)
	virtual BOOL OnInitDialog();
	afx_msg void OnUpdatePrototype();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CREATEVARIABLE_H__910E9A04_FFF5_11D3_BA92_00C0DF22DE85__INCLUDED_)
