/*
	ScenePortDll.h
	(c)2005 Palestar, Richard Lyle
*/

//----------------------------------------------------------------------------

#include "Standard/Dll.h"

#undef DLL

#ifdef SCENE_PORT_DLL
#define DLL			DLL_EXPORT
#else

#define DLL			DLL_IMPORT

#ifdef _DEBUG
#pragma comment(lib,"ScenePortD.lib") 
#else
#pragma comment(lib,"ScenePort.lib") 
#endif


#endif

//----------------------------------------------------------------------------
// EOF
