// NodeComplexMeshPort.cpp : Defines the initialization routines for the DLL.
//

#define NODECOMPLEXMESHPORT_DLL
#include "stdafx.h"
#include <afxdllx.h>

#include "Port.h"
#include "Resource.h"


static AFX_EXTENSION_MODULE NodeComplexMeshPortDLL = { NULL, NULL };

extern "C" int APIENTRY
DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	static HICON						Icon;

	// Remove this if you use lpReserved
	UNREFERENCED_PARAMETER(lpReserved);

	if (dwReason == DLL_PROCESS_ATTACH)
	{
		//TRACE0("NODECOMPLEXMESHPORT.DLL Initializing!\n");
		
		// Extension DLL one-time initialization
		if (!AfxInitExtensionModule(NodeComplexMeshPortDLL, hInstance))
			return 0;
		new CDynLinkLibrary(NodeComplexMeshPortDLL);

		Icon = LoadIcon( hInstance, MAKEINTRESOURCE(IDR_PORT) );

		// register with ScenePort
		BaseNodePort::registerPort( CLASS_KEY(NodeComplexMeshPort), Icon );
		//BaseNodePort::registerImportExt( CLASS_KEY(NodeComplexMeshPort), ".LWO" );		// LightWave Object
		//BaseNodePort::registerImportExt( CLASS_KEY(NodeComplexMeshPort), ".DAE" );		// Collada
	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
		//TRACE0("NODECOMPLEXMESHPORT.DLL Terminating!\n");
		// Terminate the library before destructors are called
		AfxTermExtensionModule(NodeComplexMeshPortDLL);

		// unregister port
		BaseNodePort::unregisterPort( CLASS_KEY(NodeComplexMeshPort) );
		//BaseNodePort::unregisterImportExt( ".LWO" );
		//BaseNodePort::unregisterImportExt( ".DAE" );

		DeleteObject(Icon);
	}
	return 1;   // ok
}
