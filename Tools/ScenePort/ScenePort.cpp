// ScenePort.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include <afxdllx.h>

#include "Port.h"
#include "NodePort.h" 
#include "NodeView.h"
#include "SegmentView.h"
#include "ChildFrame.h"
#include "Resource.h"

//-------------------------------------------------------------------------------

AFX_EXTENSION_MODULE						g_PortDLL = { NULL, NULL };

//-------------------------------------------------------------------------------

extern "C" int APIENTRY
DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	static HICON						Icon;
	static HICON						BaseNodeIcon;
	static HICON						NodeIcon;
	
	// Remove this if you use lpReserved
	UNREFERENCED_PARAMETER(lpReserved);

	if (dwReason == DLL_PROCESS_ATTACH)
	{
		//TRACE0("SCENEPORT.DLL Initializing!\n");
		
		// Extension DLL one-time initialization
		if (!AfxInitExtensionModule(g_PortDLL, hInstance))
			return 0;
		new CDynLinkLibrary(g_PortDLL);

		Icon = LoadIcon( hInstance, MAKEINTRESOURCE(IDR_PORT) );
		BaseNodeIcon = LoadIcon( hInstance, MAKEINTRESOURCE(IDR_BASENODE) );
		NodeIcon = LoadIcon( hInstance, MAKEINTRESOURCE(IDR_NODE) );

		// register with resourcer
		Port::registerPort( CLASS_KEY( ScenePort ), Icon );
		Port::registerImportExt( CLASS_KEY( ScenePort ), ".DAE" );	
		Port::registerImportExt( CLASS_KEY( ScenePort ), ".MAX" );	
		Port::registerImportExt( CLASS_KEY( ScenePort ), ".3DS" );	
		Port::registerImportExt( CLASS_KEY( ScenePort ), ".OBJ" );	
		Port::registerImportExt( CLASS_KEY( ScenePort ), ".FBX" );	
		Port::registerImportExt( CLASS_KEY( ScenePort ), ".DFX" );	

		BaseNodePort::registerPort( CLASS_KEY(BaseNodePort), BaseNodeIcon );
		BaseNodePort::registerPort( CLASS_KEY(NodePort), NodeIcon );

#ifdef _DEBUG
		// dynamically load the correct DLL's
		static Library LIB_IMPORTDAE( "ImportFBXD.dll" );
#else
		static Library LIB_IMPORTDAE( "ImportFBX.dll" );
#endif
	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
		//TRACE0("SCENEPORT.DLL Terminating!\n");
		// Terminate the library before destructors are called
		AfxTermExtensionModule(g_PortDLL);

		// unregister with resourcer
		Port::unregisterPort( CLASS_KEY( ScenePort ) );
		Port::unregisterImportExt( ".DAE" );
		Port::unregisterImportExt( ".MAX" );
		Port::unregisterImportExt( ".3DS" );
		Port::unregisterImportExt( ".OBJ" );
		Port::unregisterImportExt( ".FBX" );
		Port::unregisterImportExt( ".DFX" );

		BaseNodePort::unregisterPort( CLASS_KEY(BaseNodePort) );
		BaseNodePort::unregisterPort( CLASS_KEY(NodePort) );

		DeleteObject(Icon);
		DeleteObject(BaseNodeIcon);
		DeleteObject(NodeIcon);
	}

	return 1;   // ok
}
