/*
	ExceptionHandler.h

	This class provides a SEH handler that will dump stack, processor, 
	and register information into ProcessTrace() (see Trace.h). Usually, you would setup the 
	trace to dump it's output to a file for debugging purposes.

	(c)2005 Palestar Inc, Richard Lyle
*/

#ifndef EXCEPTIONHANDLER_H
#define EXCEPTIONHANDLER_H

#include "Standard/Types.h"
#include "Standard/CharString.h"
#include "MedusaDll.h"

//----------------------------------------------------------------------------

DLL void			LoadProgramVersion();			// attempt to load the version & build number from the version.ini file
DLL dword &			GetProgramVersion();
DLL dword &			GetProgramBuild();

DLL int				GetStack( void * pContext, dword * pStack, int nMax );
DLL int				GetStack( dword * pStack, int nMax );
DLL bool			GetLogicalAddress( void * addr,char * szModule, dword len,dword & section, dword & offset );
DLL void			DumpStack( CharString & a_sDump, dword * pStack, int nMax );
DLL void			DumpContextStack( CharString & a_sDump, void * pContext );

DLL int				ProcessException( void * pException );
DLL int				HandleException( int (* pFunction)( void * pArg ), void * pArg );
DLL void			DumpThreadStack( void * pThread );

//----------------------------------------------------------------------------

#endif

//----------------------------------------------------------------------------
//EOF
