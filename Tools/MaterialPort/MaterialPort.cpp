// MaterialPort.cpp : Defines the initialization routines for the DLL.
//

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
		//TRACE0("MATERIALPORT.DLL Initializing!\n");
		
		if ( g_PortDLL.bInitialized )
			return 1;
		// Extension DLL one-time initialization
		//MessageBox( NULL, "AfxInitExtensionModule()", "MaterialPort", MB_OK );
		if (!AfxInitExtensionModule(g_PortDLL, hInstance))
			return 0;
		//MessageBox( NULL, "CDynLinkLibrary()", "MaterialPort", MB_OK );
		new CDynLinkLibrary(g_PortDLL);

		//MessageBox( NULL, "LoadIcon()", "MaterialPort", MB_OK );
		Icon = LoadIcon( hInstance, MAKEINTRESOURCE(IDR_PORT) );

		// register with resourcer
		Port::registerPort( CLASS_KEY( MaterialPort ), Icon );
		Port::registerImportExt( CLASS_KEY( MaterialPort ), ".TGA"  );
		Port::registerImportExt( CLASS_KEY( MaterialPort ), ".FLC"  );
	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
		//TRACE0("MATERIALPORT.DLL Terminating!\n");
		// Terminate the library before destructors are called
		AfxTermExtensionModule(g_PortDLL);

		// unregister with resourcer
		Port::unregisterPort( CLASS_KEY( MaterialPort ) );
		Port::unregisterImportExt( ".TGA" );
		Port::unregisterImportExt( ".FLC" );

		DeleteObject(Icon);
	}

	return 1;   // ok
}
