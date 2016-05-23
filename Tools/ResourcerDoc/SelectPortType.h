#if !defined(AFX_SELECTPORTTYPE_H__87478F25_CC8B_11D2_B451_00C04F6FF8BD__INCLUDED_)
#define AFX_SELECTPORTTYPE_H__87478F25_CC8B_11D2_B451_00C04F6FF8BD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SelectPortType.h : header file
//

#include "Resource.h"

/////////////////////////////////////////////////////////////////////////////
// CSelectPortType dialog

class CSelectPortType : public CDialog
{
// Construction
public:
	CSelectPortType(CWnd* pParent = NULL);   // standard constructor

	int			m_SelectedPort;

// Dialog Data
	//{{AFX_DATA(CSelectPortType)
	enum { IDD = IDD_SELECT_PORT_TYPE };
	CListCtrl	m_PortList;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSelectPortType)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSelectPortType)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SELECTPORTTYPE_H__87478F25_CC8B_11D2_B451_00C04F6FF8BD__INCLUDED_)
