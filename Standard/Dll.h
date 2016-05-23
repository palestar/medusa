/*
	Dll.h
	(c)2005 Palestar, Richard Lyle
*/

//----------------------------------------------------------------------------

#if defined(_WIN32)

// WIN32
#define	DLL_EXPORT			__declspec(dllexport)
#define DLL_IMPORT			__declspec(dllimport)

#else

// GNU  
#define DLL_EXPORT
#define DLL_IMPORT

#endif

//----------------------------------------------------------------------------
// EOF
