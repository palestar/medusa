/*
	GCQDll.h
	(c)1998 Palestar, Richard Lyle
*/

//----------------------------------------------------------------------------
// NOTE: no include guards, this is because DLL will be defined multiple times

#include "Standard/Dll.h"

#undef DLL

#ifdef GCQ_DLL
#define DLL			DLL_EXPORT
#else

#define DLL			DLL_IMPORT

#ifdef _DEBUG
#pragma comment(lib,"GCQD.lib") 
#else
#pragma comment(lib,"GCQ.lib") 
#endif

#endif

//----------------------------------------------------------------------------
// EOF
