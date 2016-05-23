// MusicPort.cpp : Defines the initialization routines for the DLL.
//

#define MUSICPORT_DLL
#include "stdafx.h"
#include <afxdllx.h>

#include "Port.h"
#include "PortView.h"
#include "ChildFrame.h"
#include "Resource.h"

//-------------------------------------------------------------------------------

AFX_EXTENSION_MODULE						g_PortDLL = { NULL, NULL };

//-------------------------------------------------------------------------------

extern "C" int APIENTRY
DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	static HICON						Icon;

	// Remove this if you use lpReserved
	UNREFERENCED_PARAMETER(lpReserved);

	if (dwReason == DLL_PROCESS_ATTACH)
	{
		//TRACE0("MUSICPORT.DLL Initializing!\n"); 
		
		// Extension DLL one-time initialization
		if (!AfxInitExtensionModule(g_PortDLL, hInstance))
			return 0;
		new CDynLinkLibrary(g_PortDLL);

		Icon = LoadIcon( hInstance, MAKEINTRESOURCE(IDR_PORT) );

		// register with resourcer
		Port::registerPort( CLASS_KEY( MusicPort ), Icon );
		Port::registerImportExt( CLASS_KEY( MusicPort ), ".WAV" );
	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
		//TRACE0("MUSICPORT.DLL Terminating!\n");
		// Terminate the library before destructors are called
		AfxTermExtensionModule(g_PortDLL);

		// unregister with resourcer
		Port::unregisterPort( CLASS_KEY( MusicPort ) );
		Port::unregisterImportExt( ".WAV" );			

		DeleteObject(Icon);
	}

	return 1;   // ok
}
