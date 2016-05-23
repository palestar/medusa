// NodePortWizardaw.cpp : implementation file
//

#include "stdafx.h"
#include "NodePortWizard.h"
#include "NodePortWizardaw.h"

#include <atlbase.h>

#ifdef _PSEUDO_DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// This is called immediately after the custom AppWizard is loaded.  Initialize
//  the state of the custom AppWizard here.
void CNodePortWizardAppWiz::InitCustomAppWiz()
{
	// There are no steps in this custom AppWizard.
	SetNumberOfSteps(0);

	// Add build step to .hpj if there is one
	m_Dictionary[_T("HELP")] = _T("1");

	// Inform AppWizard that we're making a DLL.
	m_Dictionary[_T("PROJTYPE_DLL")] = _T("1");

	// TODO: Add any other custom AppWizard-wide initialization here.
}

// This is called just before the custom AppWizard is unloaded.
void CNodePortWizardAppWiz::ExitCustomAppWiz()
{
	// TODO: Add code here to deallocate resources used by the custom AppWizard
}

// This is called when the user clicks "Create..." on the New Project dialog
CAppWizStepDlg* CNodePortWizardAppWiz::Next(CAppWizStepDlg* pDlg)
{
	ASSERT(pDlg == NULL);	// By default, this custom AppWizard has no steps

	// Set template macros based on the project name entered by the user.

	// Get value of $$root$$ (already set by AppWizard)
	CString strRoot;
	m_Dictionary.Lookup(_T("root"), strRoot);
	
	// Set value of $$Doc$$, $$DOC$$
	CString strDoc = strRoot.Left(6);
	m_Dictionary[_T("Doc")] = strDoc;
	strDoc.MakeUpper();
	m_Dictionary[_T("DOC")] = strDoc;

	// Set value of $$MAC_TYPE$$
	strRoot = strRoot.Left(4);
	int nLen = strRoot.GetLength();
	if (strRoot.GetLength() < 4)
	{
		CString strPad(_T(' '), 4 - nLen);
		strRoot += strPad;
	}
	strRoot.MakeUpper();
	m_Dictionary[_T("MAC_TYPE")] = strRoot;

	// Return NULL to indicate there are no more steps.  (In this case, there are
	//  no steps at all.)
	return NULL;
}

void CNodePortWizardAppWiz::CustomizeProject(IBuildProject* pProject)
{
	// TODO: Add code here to customize the project.  If you don't wish
	//  to customize project, you may remove this virtual override.
	
	// This is called immediately after the default Debug and Release
	//  configurations have been created for each platform.  You may customize
	//  existing configurations on this project by using the methods
	//  of IBuildProject and IConfiguration such as AddToolSettings,
	//  RemoveToolSettings, and AddCustomBuildStep. These are documented in
	//  the Developer Studio object model documentation.

	// WARNING!!  IBuildProject and all interfaces you can get from it are OLE
	//  COM interfaces.  You must be careful to release all new interfaces
	//  you acquire.  In accordance with the standard rules of COM, you must
	//  NOT release pProject, unless you explicitly AddRef it, since pProject
	//  is passed as an "in" parameter to this function.  See the documentation
	//  on CCustomAppWiz::CustomizeProject for more information.

	long lNumConfigs;

	CComPtr<IConfigurations> pConfigs;
	pProject->get_Configurations(&pConfigs);
	pConfigs->get_Count(&lNumConfigs);
	for (long j = 1 ; j < lNumConfigs + 1 ; j++)
	{
		CComVariant Varj = j;
		//Get each individual configuration
		CComPtr<IConfiguration> pConfig;
		pConfigs->Item(Varj, &pConfig);
		
		// determine if this is a debug or release build
		bool debug = false;
		{
			CComBSTR configurationName( 256 );
			pConfig->get_Name( &configurationName );
			if ( CString( configurationName ).Find( _T("Debug") ) != -1 )
				debug = true;
		}

		// set the compiler settings
		CComVariant reserved;
		pConfig->AddToolSettings( CComBSTR(L"cl.exe"),CComBSTR(L"/GR") , reserved);		// turn on RTTI
		pConfig->AddToolSettings( CComBSTR(L"cl.exe"),CComBSTR(L"/I..\\..\\") , reserved);		// set the include path
		pConfig->RemoveToolSettings( CComBSTR(L"cl.exe"),CComBSTR(L"/D_USRDLL"), reserved);	
		pConfig->AddToolSettings( CComBSTR(L"cl.exe"),CComBSTR(L"/D_AFXEXT"), reserved);	

		pConfig->AddToolSettings( CComBSTR(L"link.exe"), CComBSTR(L"/libpath:..\\..\\Bin"), reserved);
		pConfig->AddToolSettings( CComBSTR(L"link.exe"), CComBSTR(L"/libpath:..\\Bin"), reserved);
		if ( debug )
			pConfig->AddToolSettings( CComBSTR(L"link.exe"), CComBSTR(L"DebugD.lib FactoryD.lib ResourceD.lib StandardD.lib FileD.lib Render3dD.lib"), reserved);
		else
			pConfig->AddToolSettings( CComBSTR(L"link.exe"), CComBSTR(L"Debug.lib Factory.lib Resource.lib Standard.lib File.lib Render3d.lib"), reserved);

		// get the project name
		CString strRoot;
		m_Dictionary.Lookup(_T("root"), strRoot);

		// set the output
		if ( debug )
		{
			CString out( CString("/out:..\\Bin\\") + strRoot + CString("D.dll") );
			pConfig->AddToolSettings( CComBSTR(L"link.exe"), out.AllocSysString() , reserved);
		}
		else
		{
			CString out( CString("/out:..\\Bin\\") + strRoot + CString(".dll") );
			pConfig->AddToolSettings( CComBSTR(L"link.exe"), out.AllocSysString() , reserved);
		}
	}
}


// Here we define one instance of the CNodePortWizardAppWiz class.  You can access
//  m_Dictionary and any other public members of this class through the
//  global NodePortWizardaw.
CNodePortWizardAppWiz NodePortWizardaw;

