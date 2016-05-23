// NodeScenePort.cpp : Defines the initialization routines for the DLL.
//

#define NODESCENEPORT_DLL
#include "stdafx.h"
#include <afxdllx.h>

#include "Port.h"
#include "Resource.h"

static AFX_EXTENSION_MODULE NodeScenePortDLL = { NULL, NULL };

extern "C" int APIENTRY
DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	static HICON						Icon;

	// Remove this if you use lpReserved
	UNREFERENCED_PARAMETER(lpReserved);

	if (dwReason == DLL_PROCESS_ATTACH)
	{
		//TRACE0("NODESCENEPORT.DLL Initializing!\n");
		
		// Extension DLL one-time initialization
		if (!AfxInitExtensionModule(NodeScenePortDLL, hInstance))
			return 0;
		new CDynLinkLibrary(NodeScenePortDLL);

		Icon = LoadIcon( hInstance, MAKEINTRESOURCE(IDR_PORT) );

		// register with ScenePort
		NodePort::registerPort( CLASS_KEY(NodeScenePort), Icon );

	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
		//TRACE0("NODESCENEPORT.DLL Terminating!\n");
		// Terminate the library before destructors are called
		AfxTermExtensionModule(NodeScenePortDLL);

		// unregister port
		NodePort::unregisterPort( CLASS_KEY(NodeScenePort) );

		DeleteObject(Icon);
	}
	return 1;   // ok
}
