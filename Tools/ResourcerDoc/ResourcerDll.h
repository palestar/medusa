/*
	ResourcerDll.h
	(c)2005 Palestar, Richard Lyle
*/

//----------------------------------------------------------------------------

#include "Standard/Dll.h"

#undef DLL

#ifdef RESOURCERDOC_DLL
#define DLL			DLL_EXPORT
#else

#define DLL			DLL_IMPORT
#ifdef _DEBUG
#pragma comment(lib,"ResourcerDocD.lib") 
#else
#pragma comment(lib,"ResourcerDoc.lib") 
#endif

#endif

//----------------------------------------------------------------------------
// EOF

