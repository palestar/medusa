// ProfilerClient.h : main header file for the PROFILERCLIENT application
//

#if !defined(AFX_PROFILERCLIENT_H__951D95C4_03C6_409E_8263_F2B9B21CF799__INCLUDED_)
#define AFX_PROFILERCLIENT_H__951D95C4_03C6_409E_8263_F2B9B21CF799__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CProfilerClientApp:
// See ProfilerClient.cpp for the implementation of this class
//

class CProfilerClientApp : public CWinApp
{
public:
	CProfilerClientApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CProfilerClientApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CProfilerClientApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PROFILERCLIENT_H__951D95C4_03C6_409E_8263_F2B9B21CF799__INCLUDED_)
