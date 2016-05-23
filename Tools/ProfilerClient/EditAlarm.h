#if !defined(AFX_EDITALARM_H__37259AD5_938C_4044_9F31_2AE9AA9884A0__INCLUDED_)
#define AFX_EDITALARM_H__37259AD5_938C_4044_9F31_2AE9AA9884A0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EditAlarm.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CEditAlarm dialog

class CEditAlarm : public CDialog
{
// Construction
public:
	CEditAlarm(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CEditAlarm)
	enum { IDD = IDD_EDIT_ALARM };
	BOOL	m_bAudibleAlaram;
	CString	m_sNameFilter;
	int		m_nPercent;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditAlarm)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CEditAlarm)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EDITALARM_H__37259AD5_938C_4044_9F31_2AE9AA9884A0__INCLUDED_)
