#if !defined(AFX_EDITSEGMENT_H__C61AE365_EB8E_11D2_B451_00C04F6FF8BD__INCLUDED_)
#define AFX_EDITSEGMENT_H__C61AE365_EB8E_11D2_B451_00C04F6FF8BD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EditSegment.h : header file
//
#include "Resource.h"

/////////////////////////////////////////////////////////////////////////////
// CEditSegment dialog

class CEditSegment : public CDialog
{
// Construction
public:
	CEditSegment( CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CEditSegment)
	enum { IDD = IDD_EDITSEGMENT };
	UINT	m_Begin;
	UINT	m_End;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditSegment)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CEditSegment)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EDITSEGMENT_H__C61AE365_EB8E_11D2_B451_00C04F6FF8BD__INCLUDED_)
