// NodeParticleSystem2Port.cpp : Defines the initialization routines for the DLL.
//

#define NODEPARTICLESYSTEM2PORT_DLL
#include "stdafx.h"
#include <afxdllx.h>

#include "Port.h"
#include "Resource.h"


static AFX_EXTENSION_MODULE NodeParticleSystem2PortDLL = { NULL, NULL };

extern "C" int APIENTRY
DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	static HICON						Icon;

	// Remove this if you use lpReserved
	UNREFERENCED_PARAMETER(lpReserved);

	if (dwReason == DLL_PROCESS_ATTACH)
	{
		//TRACE0("NODEPARTICLESYSTEM2PORT.DLL Initializing!\n");
		
		// Extension DLL one-time initialization
		if (!AfxInitExtensionModule(NodeParticleSystem2PortDLL, hInstance))
			return 0;
		new CDynLinkLibrary(NodeParticleSystem2PortDLL);

		Icon = LoadIcon( hInstance, MAKEINTRESOURCE(IDR_PORT) );

		// register with ScenePort
		BaseNodePort::registerPort( CLASS_KEY(NodeParticleSystem2Port), Icon );

	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
		//TRACE0("NODEPARTICLESYSTEM2PORT.DLL Terminating!\n");
		// Terminate the library before destructors are called
		AfxTermExtensionModule(NodeParticleSystem2PortDLL);

		// unregister port
		BaseNodePort::unregisterPort( CLASS_KEY(NodeParticleSystem2Port) );

		DeleteObject(Icon);
	}
	return 1;   // ok
}
