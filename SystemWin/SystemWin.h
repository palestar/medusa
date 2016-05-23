/*
	SystemWin.h
	(c)2005 Palestar Inc, Richard Lyle
*/

//----------------------------------------------------------------------------
// NOTE: no include guards, this is because DLL will be defined multiple times

#include "Standard/Dll.h"

#undef DLL

#ifdef SYSTEMWIN_DLL
#define DLL			DLL_EXPORT
#else
#define DLL			DLL_IMPORT
#endif

//----------------------------------------------------------------------------
// EOF
