// MedusaExport.h : main header file for the MEDUSAEXPORT DLL
//

#if !defined(AFX_MEDUSAEXPORT_H__80981FBD_629B_4B90_8940_D32AD4F46BAC__INCLUDED_)
#define AFX_MEDUSAEXPORT_H__80981FBD_629B_4B90_8940_D32AD4F46BAC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CMedusaExportApp
// See MedusaExport.cpp for the implementation of this class
//

class CMedusaExportApp : public CWinApp
{
public:
	CMedusaExportApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMedusaExportApp)
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CMedusaExportApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MEDUSAEXPORT_H__80981FBD_629B_4B90_8940_D32AD4F46BAC__INCLUDED_)
