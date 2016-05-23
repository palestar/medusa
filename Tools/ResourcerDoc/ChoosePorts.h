#if !defined(AFX_CHOOSEPORTS_H__C5C9BAE1_D86E_11D2_A4ED_00C04F6FF8CE__INCLUDED_)
#define AFX_CHOOSEPORTS_H__C5C9BAE1_D86E_11D2_A4ED_00C04F6FF8CE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ChoosePorts.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CChoosePorts dialog

//#include "Resource.h"

#define IDD_CHOOSE_PORTS                1032

#include "Tools/ResourcerDoc/ResourcerDll.h"

class DLL CChoosePorts : public CDialog
{
// Construction
public:
	CChoosePorts(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CChoosePorts)
	enum { IDD = IDD_CHOOSE_PORTS };
	CListCtrl	m_PortList;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CChoosePorts)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CChoosePorts)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnAdd();
	afx_msg void OnRemove();
	afx_msg void OnRemoveAll();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CHOOSEPORTS_H__C5C9BAE1_D86E_11D2_A4ED_00C04F6FF8CE__INCLUDED_)
