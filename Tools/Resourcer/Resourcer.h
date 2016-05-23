// Resourcer.h : main header file for the RESOURCER application
//

#if !defined(AFX_RESOURCER_H__F7307288_6F06_11D2_B451_00C04F6FF8BD__INCLUDED_)
#define AFX_RESOURCER_H__F7307288_6F06_11D2_B451_00C04F6FF8BD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

/////////////////////////////////////////////////////////////////////////////
// CResourcerApp:
// See Resourcer.cpp for the implementation of this class
//

class CResourcerApp : public CWinApp
{
public:
	CResourcerApp();

	// Mutators
	bool		m_ConfirmDelete;		// confirm before deleting objects

private:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CResourcerApp)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	//}}AFX_VIRTUAL

// Implementation
	//{{AFX_MSG(CResourcerApp)
	afx_msg void OnAppAbout();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	virtual int Run();
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_RESOURCER_H__F7307288_6F06_11D2_B451_00C04F6FF8BD__INCLUDED_)
