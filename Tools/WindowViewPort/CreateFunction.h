#if !defined(AFX_CREATEFUNCTION_H__67D98F02_FF64_11D3_B451_00C04F6FF8BD__INCLUDED_)
#define AFX_CREATEFUNCTION_H__67D98F02_FF64_11D3_B451_00C04F6FF8BD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CreateFunction.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CCreateFunction dialog

#include "Resource.h"

class CCreateFunction : public CDialog
{
// Construction
public:
	CCreateFunction(CWnd* pParent = NULL);   // standard constructor

	CString	m_Template;

// Dialog Data
	//{{AFX_DATA(CCreateFunction)
	enum { IDD = IDD_CREATE_FUNCTION };
	CString	m_FunctionName;
	CString	m_Prototype;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCreateFunction)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CCreateFunction)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	afx_msg void OnUpdatePrototype();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CREATEFUNCTION_H__67D98F02_FF64_11D3_B451_00C04F6FF8BD__INCLUDED_)
