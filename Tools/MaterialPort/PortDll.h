/*
	PortDll.h
	(c)2005 Palestar, Richard Lyle
*/

//----------------------------------------------------------------------------

#include "Standard/Dll.h"

#undef DLL

#ifdef MATERIALPORT_DLL
#define DLL			DLL_EXPORT
#else
#define DLL			DLL_IMPORT

#ifdef _DEBUG
#pragma comment(lib,"MaterialPortD.lib") 
#else
#pragma comment(lib,"MaterialPort.lib") 
#endif

#endif

//----------------------------------------------------------------------------
// EOF
