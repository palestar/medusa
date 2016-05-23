/*
	WorldDll.h
	(c)2000 Palestar Inc, Richard Lyle
*/

#include "Standard/Dll.h"

//----------------------------------------------------------------------------

#undef DLL

#ifdef WORLD_DLL
#define DLL			DLL_EXPORT
#else

#define DLL			DLL_IMPORT

#ifdef _DEBUG
#pragma comment(lib,"WorldD.lib") 
#else
#pragma comment(lib,"World.lib") 
#endif

#endif

//----------------------------------------------------------------------------
// EOF
