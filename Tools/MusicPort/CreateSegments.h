#if !defined(AFX_CREATESEGMENTS_H__BA439721_EBB7_11D2_A4ED_00C04F6FF8CE__INCLUDED_)
#define AFX_CREATESEGMENTS_H__BA439721_EBB7_11D2_A4ED_00C04F6FF8CE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CreateSegments.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CCreateSegments dialog

#include "Resource.h"

class CCreateSegments : public CDialog
{
// Construction
public:
	CCreateSegments(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CCreateSegments)
	enum { IDD = IDD_CREATESEGMENTS };
	CEdit	m_SamplesCtrl;
	UINT	m_Samples;
	BOOL	m_UseCue;
	BOOL	m_KeepBranches;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCreateSegments)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CCreateSegments)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnUseCue();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CREATESEGMENTS_H__BA439721_EBB7_11D2_A4ED_00C04F6FF8CE__INCLUDED_)
