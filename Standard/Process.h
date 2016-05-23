/*
	Process.h

	This object is used to start a child-process
	(c)2005 Palestar, Richard Lyle
*/

#ifndef PROCESS_H
#define PROCESS_H

#include "Standard/Hash.h"
#include "Standard/CriticalSection.h"
#include "MedusaDll.h"

#pragma warning( disable:4251 )		// needs to have dll-interface

//-------------------------------------------------------------------------------

class DLL Process
{
public:
	static void *	open( dword processId );
	static void *	start( const char * pCommand );
	static void *	start( const char * pCommand, const char * pDirectory, dword * pID = NULL );
	static void		stop( void * handle );
	static int		wait( void * handle );
	static bool		active( void * handle );		// returns true if process is still running, false if not
	static int		exitCode( void * handle );		// returns the exit code for stopped process
	static void		close( void * handle );			// release the handle for a process

	static dword	getProcessId( void * handle );	// returns 0 on failure, otherwise the process id of a process
	static void *	getCurrentProcess();			// get the handle for the calling process
	static dword	getCurrentProcessId();			// get the calling process id
};

//----------------------------------------------------------------------------



#endif

//-------------------------------------------------------------------------------
// EOF
