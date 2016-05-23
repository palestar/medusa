/*
	SelfUpdateDll.h
	(c)2004 Palestar, Richard Lyle
*/

//----------------------------------------------------------------------------
// NOTE: no include guards, this is because DLL will be defined multiple times

#include "Standard/Dll.h"

#undef DLL

#ifdef SELFUPDATE_DLL
#define DLL			DLL_EXPORT
#else

#define DLL			DLL_IMPORT

#ifdef _DEBUG
#pragma comment(lib,"SelfUpdateD.lib") 
#else
#pragma comment(lib,"SelfUpdate.lib") 
#endif

#endif

//----------------------------------------------------------------------------
// EOF
