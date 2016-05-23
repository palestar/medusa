// NodeParticleSystemPort.cpp : Defines the initialization routines for the DLL.
//

#define NODEPARTICLESYSTEMPORT_DLL
#include "stdafx.h"
#include <afxdllx.h>

#include "Port.h"
#include "Resource.h"

static AFX_EXTENSION_MODULE NodeParticleSystemPortDLL = { NULL, NULL };

extern "C" int APIENTRY
DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	static HICON						Icon;

	// Remove this if you use lpReserved
	UNREFERENCED_PARAMETER(lpReserved);

	if (dwReason == DLL_PROCESS_ATTACH)
	{
		//TRACE0("NODEPARTICLESYSTEMPORT.DLL Initializing!\n");
		
		// Extension DLL one-time initialization
		if (!AfxInitExtensionModule(NodeParticleSystemPortDLL, hInstance))
			return 0;
		new CDynLinkLibrary(NodeParticleSystemPortDLL);

		Icon = LoadIcon( hInstance, MAKEINTRESOURCE(IDR_PORT) );

		// register with ScenePort
		BaseNodePort::registerPort( CLASS_KEY(NodeParticleSystemPort), Icon );

	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
		//TRACE0("NODEPARTICLESYSTEMPORT.DLL Terminating!\n");
		// Terminate the library before destructors are called
		AfxTermExtensionModule(NodeParticleSystemPortDLL);

		// unregister port
		BaseNodePort::unregisterPort( CLASS_KEY(NodeParticleSystemPort) );

		DeleteObject(Icon);
	}
	return 1;   // ok
}
