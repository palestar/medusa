// SoundPort.cpp : Defines the initialization routines for the DLL.
//

#define SOUNDPORT_DLL
#include "stdafx.h"
#include <afxdllx.h>

#include "Port.h"
#include "PortView.h"
#include "ChildFrame.h"
#include "Resource.h"

//-------------------------------------------------------------------------------

AFX_EXTENSION_MODULE						g_PortDLL = { NULL, NULL };
CMultiDocTemplate *							g_Template = NULL;

//-------------------------------------------------------------------------------

extern "C" int APIENTRY
DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	static HICON						Icon;

	// Remove this if you use lpReserved
	UNREFERENCED_PARAMETER(lpReserved);

	if (dwReason == DLL_PROCESS_ATTACH)
	{
		//TRACE0("SOUNDPORT.DLL Initializing!\n");
		
		// Extension DLL one-time initialization
		if (!AfxInitExtensionModule(g_PortDLL, hInstance))
			return 0;
		new CDynLinkLibrary(g_PortDLL);

		Icon = LoadIcon( hInstance, MAKEINTRESOURCE(IDR_PORT) );

		// create the template(s)
		g_Template = new CMultiDocTemplate( IDR_PORT, 
			RUNTIME_CLASS(CDocument), 
			RUNTIME_CLASS(CChildFrame),
			RUNTIME_CLASS(CPortView));	

		// register with resourcer
		Port::registerPort( CLASS_KEY( SoundPort ), Icon );
		Port::registerImportExt( CLASS_KEY( SoundPort ), ".WAV" );
	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
		//TRACE0("SOUNDPORT.DLL Terminating!\n");
		// Terminate the library before destructors are called
		AfxTermExtensionModule(g_PortDLL);

		// unregister with resourcer
		Port::unregisterPort( CLASS_KEY( SoundPort ) );
		Port::unregisterImportExt( ".WAV" );		

		// release the template
		delete g_Template;

		DeleteObject(Icon);
	}

	return 1;   // ok
}
