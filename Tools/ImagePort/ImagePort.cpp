// ImagePort.cpp : Defines the initialization routines for the DLL.
//

#define IMAGEPORT_DLL
#include "stdafx.h"
#include <afxdllx.h>

#include "Port.h"
#include "PortView.h"
#include "ImageView.h"
#include "ChildFrame.h"
#include "Resource.h"

//-------------------------------------------------------------------------------

AFX_EXTENSION_MODULE						g_PortDLL = { NULL, NULL };

//-------------------------------------------------------------------------------

extern "C" int APIENTRY
DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	static HICON						Icon;

	// Remove this if you use lpReserved
	UNREFERENCED_PARAMETER(lpReserved);

	if (dwReason == DLL_PROCESS_ATTACH)
	{
		//TRACE0("IMAGEPORT.DLL Initializing!\n");
		
		// Extension DLL one-time initialization
		if (!AfxInitExtensionModule(g_PortDLL, hInstance))
			return 0;
		new CDynLinkLibrary(g_PortDLL);

		Icon = LoadIcon( hInstance, MAKEINTRESOURCE(IDR_PORT) );

#ifdef _DEBUG
		static Library		LIB_DISPLAYD3D( "DisplayD3DD.dll" );		// Direct3D
#else
		static Library		LIB_DISPLAYD3D( "DisplayD3D.dll" );			// Direct3D
#endif
		// register with resourcer
		Port::registerPort( CLASS_KEY( ImagePort ), Icon );
		Port::registerImportExt( CLASS_KEY( ImagePort ), ".TGA"  );
		Port::registerImportExt( CLASS_KEY( ImagePort ), ".FLC"  );
		Port::registerImportExt( CLASS_KEY( ImagePort ), ".PCX"  );
		Port::registerImportExt( CLASS_KEY( ImagePort ), ".BMP"  );
		Port::registerImportExt( CLASS_KEY( ImagePort ), ".JPG"  );
		Port::registerImportExt( CLASS_KEY( ImagePort ), ".JPEG"  );
		Port::registerImportExt( CLASS_KEY( ImagePort ), ".TIF"  );
		Port::registerImportExt( CLASS_KEY( ImagePort ), ".TIFF"  );
		Port::registerImportExt( CLASS_KEY( ImagePort ), ".ICO"  );
		Port::registerImportExt( CLASS_KEY( ImagePort ), ".PCD"  );
		Port::registerImportExt( CLASS_KEY( ImagePort ), ".PNG"  );
		Port::registerImportExt( CLASS_KEY( ImagePort ), ".AVI"  );
	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
		//TRACE0("IMAGEPORT.DLL Terminating!\n");
		// Terminate the library before destructors are called
		AfxTermExtensionModule(g_PortDLL);

		// unregister with resourcer
		Port::unregisterPort( CLASS_KEY( ImagePort ) );
		Port::unregisterImportExt( ".TGA" );
		Port::unregisterImportExt( ".FLC" );
		Port::unregisterImportExt( ".PCX" );
		Port::unregisterImportExt( ".BMP" );
		Port::unregisterImportExt( ".JPG" );
		Port::unregisterImportExt( ".JPEG" );
		Port::unregisterImportExt( ".TIF" );
		Port::unregisterImportExt( ".TIFF" );
		Port::unregisterImportExt( ".ICO" );
		Port::unregisterImportExt( ".PCD" );
		Port::unregisterImportExt( ".PNG" );
		Port::unregisterImportExt( ".AVI" );

		DeleteObject(Icon);
	}

	return 1;   // ok
}
