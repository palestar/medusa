// WindowViewPort.cpp : Defines the initialization routines for the DLL.
//

#define WINDOWVIEWPORT_DLL
#include "stdafx.h"
#include <afxdllx.h>

#include "Port.h"
#include "Resource.h"

AFX_EXTENSION_MODULE WindowViewPortDLL = { NULL, NULL };

extern "C" int APIENTRY
DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	static HICON						Icon;

	// Remove this if you use lpReserved
	UNREFERENCED_PARAMETER(lpReserved);

	if (dwReason == DLL_PROCESS_ATTACH)
	{
		//TRACE0("WINDOWVIEWPORT.DLL Initializing!\n");
		
		// Extension DLL one-time initialization
		if (!AfxInitExtensionModule(WindowViewPortDLL, hInstance))
			return 0;
		new CDynLinkLibrary(WindowViewPortDLL);

		Icon = LoadIcon( hInstance, MAKEINTRESOURCE(IDR_PORT) );

		// register with ScenePort
		BaseNodePort::registerPort( CLASS_KEY(WindowViewPort), Icon );

	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
		//TRACE0("WINDOWVIEWPORT.DLL Terminating!\n");
		// Terminate the library before destructors are called
		AfxTermExtensionModule(WindowViewPortDLL);

		// unregister port
		BaseNodePort::unregisterPort( CLASS_KEY(WindowViewPort) );

		DeleteObject(Icon);
	}
	return 1;   // ok
}
