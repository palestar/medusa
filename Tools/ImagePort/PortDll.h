/*
	PortDll.h
	(c)2005 Palestar, Richard Lyle
*/

//----------------------------------------------------------------------------

#include "Standard/Dll.h"

#undef DLL

#ifdef IMAGEPORT_DLL
#define DLL			DLL_EXPORT
#else
#define DLL			DLL_IMPORT

#ifdef _DEBUG
#pragma comment(lib,"ImagePortD.lib") 
#else
#pragma comment(lib,"ImagePort.lib") 
#endif

#endif

//----------------------------------------------------------------------------
// EOF
