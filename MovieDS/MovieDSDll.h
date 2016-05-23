// ============================================================================
//             File Name: MovieDSDll.h
//  Associated File Name: None
// Date of File Creation: 4/1/99, 3:37:13 PM
//                Author: Erik Dieckert
// ============================================================================

//----------------------------------------------------------------------------
// NOTE: no include guards, this is because DLL will be defined multiple times

#undef DLL

#include "Standard/Dll.h"

#ifdef MOVIEDS_DLL
#define DLL			DLL_EXPORT
#else
#define DLL			DLL_IMPORT
#endif
							   
//----------------------------------------------------------------------------
// EOF
