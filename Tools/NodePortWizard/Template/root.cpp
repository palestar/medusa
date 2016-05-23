// $$root$$.cpp : Defines the initialization routines for the DLL.
//

#define $$ROOT$$_DLL
#include "stdafx.h"
#include <afxdllx.h>

#include "Port.h"
#include "Resource.h"

static AFX_EXTENSION_MODULE $$safe_root$$DLL = { NULL, NULL };

extern "C" int APIENTRY
DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	static HICON						Icon;

	// Remove this if you use lpReserved
	UNREFERENCED_PARAMETER(lpReserved);

	if (dwReason == DLL_PROCESS_ATTACH)
	{
		TRACE0("$$ROOT$$.DLL Initializing!\n");
		
		// Extension DLL one-time initialization
		if (!AfxInitExtensionModule($$safe_root$$DLL, hInstance))
			return 0;
		new CDynLinkLibrary($$safe_root$$DLL);

		Icon = LoadIcon( hInstance, MAKEINTRESOURCE(IDR_PORT) );

		// register with ScenePort
		BaseNodePort::registerPort( CLASS_KEY($$root$$), Icon );

	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
		TRACE0("$$ROOT$$.DLL Terminating!\n");
		// Terminate the library before destructors are called
		AfxTermExtensionModule($$safe_root$$DLL);

		// unregister port
		BaseNodePort::unregisterPort( CLASS_KEY($$root$$) );

		DeleteObject(Icon);
	}
	return 1;   // ok
}
