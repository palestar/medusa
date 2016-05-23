// ProfilerClient.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#pragma warning( disable : 4146 )

#include "ProfilerClient.h"
#include "ProfilerClientDlg.h"
#include "Standard/CommandLine.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CProfilerClientApp

BEGIN_MESSAGE_MAP(CProfilerClientApp, CWinApp)
	//{{AFX_MSG_MAP(CProfilerClientApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CProfilerClientApp construction

CProfilerClientApp::CProfilerClientApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CProfilerClientApp object

CProfilerClientApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CProfilerClientApp initialization

BOOL CProfilerClientApp::InitInstance()
{
	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

	SetRegistryKey( _T("Palestar") );

	CProfilerClientDlg().DoModal();

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}
