/*
	MedusaDll.h
	(c)2006 Palestar, Richard Lyle
*/

#ifdef _WIN32

#include "Standard/Dll.h"

#undef DLL

#ifdef MEDUSA_DLL
#define DLL			DLL_EXPORT
#else

#define DLL			DLL_IMPORT

#ifdef _DEBUG
#pragma comment(lib,"MedusaD.lib") 
#else
#pragma comment(lib,"Medusa.lib") 
#endif

#endif

//---------------------------------------------------------------------------------------------------

#ifndef TARGETSXS
#define TARGETSXS
// force a certain version of MSRT to be linked against to avoid SXS problems. This fixes 
// many of the SXS problems.

#ifndef __midl
#define _SXS_ASSEMBLY_VERSION "8.0.50727.762"
#undef _CRT_ASSEMBLY_VERSION
#define _CRT_ASSEMBLY_VERSION _SXS_ASSEMBLY_VERSION
#undef _MFC_ASSEMBLY_VERSION
#define _MFC_ASSEMBLY_VERSION _SXS_ASSEMBLY_VERSION
#undef _ATL_ASSEMBLY_VERSION
#define _ATL_ASSEMBLY_VERSION _SXS_ASSEMBLY_VERSION
#ifdef __cplusplus
extern "C" {
#endif
__declspec(selectany) int _forceCRTManifest;
__declspec(selectany) int _forceMFCManifest;
__declspec(selectany) int _forceAtlDllManifest;
__declspec(selectany) int _forceCRTManifestRTM;
__declspec(selectany) int _forceMFCManifestRTM;
__declspec(selectany) int _forceAtlDllManifestRTM;
#ifdef __cplusplus
}
#endif
#endif

#endif
//---------------------------------------------------------------------------------------------------

#else

// Linux/FreeBSD doesn't use DLL_EXPORT/DLL_IMPORT
#undef DLL
#define DLL

#endif

//----------------------------------------------------------------------------
// EOF
