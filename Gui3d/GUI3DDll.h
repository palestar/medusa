/*
	Guid3dDll.h
	(c)2005 Palestar, Richard Lyle
*/

//----------------------------------------------------------------------------
// NOTE: no include guards, this is because DLL will be defined multiple times

#include "Standard/Dll.h"

#undef DLL

#ifdef GUI3D_DLL
#define DLL			DLL_EXPORT
#else

#define DLL			DLL_IMPORT

#ifdef _DEBUG
#pragma comment(lib,"GUI3DD.lib") 
#else
#pragma comment(lib,"GUI3D.lib") 
#endif

#endif

//----------------------------------------------------------------------------
// EOF
