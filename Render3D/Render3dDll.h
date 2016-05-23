/*
	Render3dDll.h
	(c)2005 Palestar, Richard Lyle
*/

//----------------------------------------------------------------------------

#include "Standard/Dll.h"

#undef DLL

#ifdef RENDER3D_DLL
#define DLL			DLL_EXPORT
#else

#define DLL			DLL_IMPORT

#ifdef _DEBUG
#pragma comment(lib,"Render3DD.lib") 
#else
#pragma comment(lib,"Render3D.lib") 
#endif

#endif

//----------------------------------------------------------------------------
// EOF
