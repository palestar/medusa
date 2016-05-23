/*
	NetworkDLL.h
	(c)2005 Palestar, Erik Dieckert
*/

//----------------------------------------------------------------------------
// NOTE: no include guards, this is because DLL will be defined multiple times

#ifdef _WIN32

#include "Standard/Dll.h"

#undef DLL

#ifdef NETWORK_DLL
#define DLL			DLL_EXPORT
#else

#define DLL			DLL_IMPORT

#ifdef _DEBUG
#pragma comment(lib,"NetworkD.lib") 
#else
#pragma comment(lib,"Network.lib") 
#endif

#endif

#else

// Linux/FreeBSD doesn't use DLL_EXPORT/DLL_IMPORT
#undef DLL
#define DLL

#endif

//----------------------------------------------------------------------------
// EOF
