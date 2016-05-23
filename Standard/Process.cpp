/*
	Process.cpp

	This object is used to start a child-process
	- Most functionality moved into CoreSystem.cpp where it belongs
	(c)2005 Palestar, Richard Lyle
*/

#include "Debug/Log.h"
#include "Types.h"
#include "Process.h"
#include "StringHelpers.h"
#include "CommandLine.h"

#if defined(_WIN32)
#include <windows.h>
#else
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#endif

//----------------------------------------------------------------------------

#if defined(_WIN32)
struct ProcessHandle {
	ProcessHandle( dword nPID, HANDLE a_hHandle ) : m_nPID( nPID ), m_hHandle( a_hHandle )
	{}
	~ProcessHandle()
	{
		CloseHandle( m_hHandle );
	}
	dword		m_nPID;			// process ID
	HANDLE		m_hHandle;		// handle to the process
};
#else
struct ProcessHandle {
	ProcessHandle( dword nPID ) : m_nPID( nPID ), m_bActive( true ), m_nExitCode( 0 )
	{}
	~ProcessHandle()
	{}

	dword		m_nPID;
	bool		m_bActive;
	int			m_nExitCode;
};
#endif

//-------------------------------------------------------------------------------

void * Process::open( dword processId )
{
#if defined(_WIN32)
	HANDLE hProcess = OpenProcess( PROCESS_ALL_ACCESS, FALSE, processId );
	if ( hProcess == NULL )
		return NULL;

	return new ProcessHandle( processId, hProcess );
#else
	return new ProcessHandle( processId );
#endif
}

void * Process::start( const char * pCommand )
{
	return start( pCommand, NULL, NULL );
}

void * Process::start( const char * pCommand, const char * pDirectory, dword * pID /*= NULL*/ )
{
#if defined(_WIN32)
    STARTUPINFO			si;
    PROCESS_INFORMATION pi;

	memset( &si, 0, sizeof(si) );
    si.cb = sizeof(si);

	char sCommandLine[ MAX_PATH ];
	strncpy<char>( sCommandLine, pCommand, MAX_PATH );

	if ( !CreateProcessA( NULL,					// No module name (use command line). 
        sCommandLine,							// Command line. 
        NULL,									// Process handle not inheritable. 
        NULL,									// Thread handle not inheritable. 
        FALSE,									// Set handle inheritance to FALSE. 
        CREATE_NEW_PROCESS_GROUP,				// Creation flags. 
        NULL,									// Use parent's environment block. 
        pDirectory,						
        &si,									// Pointer to STARTUPINFO structure.
        &pi ))									// Pointer to PROCESS_INFORMATION structure.
	{
		// failed
		return NULL;
	}

	if ( pID != NULL )
		*pID = pi.dwProcessId;

	return new ProcessHandle( pi.dwProcessId, pi.hProcess );
#else
	CommandLine commands( pCommand );
	if ( commands.argumentCount() < 1 )
		return NULL;

	dword nPID = (dword)fork();
	if ( nPID == 0 )
	{
		// switch to the directory specified
		if ( pDirectory != NULL )
			chdir( pDirectory );

		// put all arguments into another array to pass into execv()..
		int argc = commands.argumentCount() + 1;
		char ** argv = new char *[ argc ];

		for(int i=0;i<commands.argumentCount();++i)
			argv[ i ] = strdup( commands.argument( i ) );
		argv[ argc - 1 ] = NULL;		// null terminate the list..

		int nExitCode = execv( argv[0], argv );

		for(int i=0;i<argc;++i)
			free( argv[i] );
		delete [] argv;

		exit( nExitCode );
	}

	return new ProcessHandle( nPID );
#endif
}

void Process::stop( void * handle )
{
	if (! handle )
		return;
#if defined(_WIN32)
	TerminateProcess( ((ProcessHandle *)handle)->m_hHandle, 0 );
#else
	ProcessHandle * pHandle = (ProcessHandle *)handle;
	if ( pHandle->m_bActive )
	{
		// kill the process by ID first..
		kill( pHandle->m_nPID, SIGTERM );

		// ... then waitpid() to clean up the process..
		int nStatus = 0;
		int nWait = waitpid( pHandle->m_nPID, &nStatus, 0 );
		if ( nWait < 0 )
			LOG_ERROR( "Process", "Process::stop() - waitpid failed for process %u", pHandle->m_nPID );
		pHandle->m_nExitCode = WEXITSTATUS(nStatus);
		pHandle->m_bActive = false;
	}
#endif
}

int Process::wait( void * handle )
{
	if (! handle )
		return -1;
#if defined(_WIN32)
	WaitForSingleObject( ((ProcessHandle *)handle)->m_hHandle, INFINITE );
	return exitCode( handle );
#else
	ProcessHandle * pHandle = (ProcessHandle *)handle;
	if ( pHandle->m_bActive )
	{
		int nStatus = 0;
		int nWait = waitpid( pHandle->m_nPID, &nStatus, 0 );
		if ( nWait < 0 )
		{
			LOG_ERROR( "Process", "Process::wait() - waitpid failed for process %u", pHandle->m_nPID );
			return -1;
		}

		pHandle->m_nExitCode = WEXITSTATUS(nStatus);
		pHandle->m_bActive = false;
	}

	return pHandle->m_nExitCode;
#endif
}

bool Process::active( void * handle )
{
	if (! handle )
		return false;
#if defined(_WIN32)
	dword exitCode;
	if ( GetExitCodeProcess( ((ProcessHandle *)handle)->m_hHandle, &exitCode ) )
		return exitCode == STILL_ACTIVE ? true : false;

	return false;
#else
	ProcessHandle * pHandle = (ProcessHandle *)handle;

	if (! pHandle->m_bActive )
		return false;		// we already know it's not active..
	// signal the process to see if it's still alive..
	if ( kill( pHandle->m_nPID, 0 ) < 0 && errno == ESRCH )
		return false;		// no such PID, so not active..

	// check if the child exited or crashed, return false if either has occured..
	int nStatus = 0;
	int nWait = waitpid( pHandle->m_nPID, &nStatus, WNOHANG );
	if ( nWait < 0 )
	{
		LOG_ERROR( "Process", "Process::active() - waitpid failed for process %u", pHandle->m_nPID );
		return false;
	}
	else if ( nWait > 0 )
	{
		pHandle->m_nExitCode = WEXITSTATUS(nStatus);
		pHandle->m_bActive = false;
		return false;		// waitpid returns a positive process ID if the child has exited..
	}

	// child still active..
	return true;
#endif
}

int Process::exitCode( void * handle )
{
	if (! handle )
		return - 1;

#if defined(_WIN32)
	dword exitCode = -1;
	GetExitCodeProcess( ((ProcessHandle *)handle)->m_hHandle, &exitCode );
	return (int)exitCode;
#else
	return ((ProcessHandle *)handle)->m_nExitCode;
#endif
}

void Process::close( void * handle )
{
	if (! handle )
		return;

#if !defined(_WIN32)
	if (! active( handle ) )
	{
		ProcessHandle * pHandle = (ProcessHandle *)handle;
		if ( pHandle->m_bActive )
		{
			// clean up the zombie process if not active..
			int nStatus = 0;
			int nWait = waitpid( pHandle->m_nPID, &nStatus, 0 );
			if ( nWait < 0 )
				LOG_ERROR( "Process", "Process::close() - waitpid failed for process %u", pHandle->m_nPID );
		}
	}
#endif

	delete (ProcessHandle *)handle;
}

dword Process::getProcessId( void * handle )
{
	return ((ProcessHandle *)handle)->m_nPID;
}

void * Process::getCurrentProcess()
{
#if defined(_WIN32)
	return new ProcessHandle( GetCurrentProcessId(), GetCurrentProcess() );
#else
	return new ProcessHandle( (dword)getpid() );
#endif
}

dword Process::getCurrentProcessId()
{
#if defined(_WIN32)
	return GetCurrentProcessId();
#else
	return (dword)getpid();
#endif
}

//-------------------------------------------------------------------------------
// EOF
