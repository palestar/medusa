// NodeParticleField.cpp : Defines the initialization routines for the DLL.
//

#define NODEPARTICLEFIELD_DLL
#include "stdafx.h"
#include <afxdllx.h>

#include "Port.h"
#include "Resource.h"


static AFX_EXTENSION_MODULE NodeParticleFieldDLL = { NULL, NULL };

extern "C" int APIENTRY
DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	static HICON						Icon;

	// Remove this if you use lpReserved
	UNREFERENCED_PARAMETER(lpReserved);

	if (dwReason == DLL_PROCESS_ATTACH)
	{
		//TRACE0("NODEPARTICLEFIELD.DLL Initializing!\n");
		
		// Extension DLL one-time initialization
		if (!AfxInitExtensionModule(NodeParticleFieldDLL, hInstance))
			return 0;
		new CDynLinkLibrary(NodeParticleFieldDLL);

		Icon = LoadIcon( hInstance, MAKEINTRESOURCE(IDR_PORT) );

		// register with ScenePort
		BaseNodePort::registerPort( CLASS_KEY(NodeParticleFieldPort), Icon );

	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
		//TRACE0("NODEPARTICLEFIELD.DLL Terminating!\n");
		// Terminate the library before destructors are called
		AfxTermExtensionModule(NodeParticleFieldDLL);

		// unregister port
		BaseNodePort::unregisterPort( CLASS_KEY(NodeParticleFieldPort) );

		DeleteObject(Icon);
	}
	return 1;   // ok
}
