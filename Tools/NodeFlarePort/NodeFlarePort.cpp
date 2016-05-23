// NodeFlarePort.cpp : Defines the initialization routines for the DLL.
//

#define NODEFLAREPORT_DLL
#include "stdafx.h"
#include <afxdllx.h>

#include "Port.h"
#include "Resource.h"


static AFX_EXTENSION_MODULE NodeFlarePortDLL = { NULL, NULL };

extern "C" int APIENTRY
DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	static HICON						Icon;

	// Remove this if you use lpReserved
	UNREFERENCED_PARAMETER(lpReserved);

	if (dwReason == DLL_PROCESS_ATTACH)
	{
		//TRACE0("NODEFLAREPORT.DLL Initializing!\n");
		
		// Extension DLL one-time initialization
		if (!AfxInitExtensionModule(NodeFlarePortDLL, hInstance))
			return 0;
		new CDynLinkLibrary(NodeFlarePortDLL);

		Icon = LoadIcon( hInstance, MAKEINTRESOURCE(IDR_PORT) );

		// register with ScenePort
		BaseNodePort::registerPort( CLASS_KEY(NodeFlarePort), Icon );

	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
		//TRACE0("NODEFLAREPORT.DLL Terminating!\n");
		// Terminate the library before destructors are called
		AfxTermExtensionModule(NodeFlarePortDLL);

		// unregister port
		BaseNodePort::unregisterPort( CLASS_KEY(NodeFlarePort) );

		DeleteObject(Icon);
	}
	return 1;   // ok
}
