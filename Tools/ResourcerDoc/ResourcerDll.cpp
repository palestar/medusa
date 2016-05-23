// ResourcerDoc.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include <afxdllx.h>

static AFX_EXTENSION_MODULE ResourcerDocDLL = { NULL, NULL };

extern "C" int APIENTRY
DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	// Remove this if you use lpReserved
	UNREFERENCED_PARAMETER(lpReserved);

	if (dwReason == DLL_PROCESS_ATTACH)
	{
		TRACE0("RESOURCERDOC.DLL Initializing!\n");
		
		// Extension DLL one-time initialization
		if (!AfxInitExtensionModule(ResourcerDocDLL, hInstance))
			return 0;
		new CDynLinkLibrary(ResourcerDocDLL);
	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
		TRACE0("RESOURCERDOC.DLL Terminating!\n");
		// Terminate the library before destructors are called
		AfxTermExtensionModule(ResourcerDocDLL);
	}
	return 1;   // ok
}
