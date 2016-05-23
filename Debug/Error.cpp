/**
	@file Error.cpp

	(c)2006 Palestar Inc
	@author Richard Lyle @date 7/19/2006 8:30:55 PM
*/

#include "Standard/List.h"
#include "Standard/String.h"
#include "Log.h"
#include "Error.h"

#if defined(_WIN32)
#define _WIN32_WINNT    0x0400        
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#endif

//---------------------------------------------------------------------------------------------------

struct ErrorLocation
{
   const char *      m_pFile;
   int               m_nLine;

   ErrorLocation( const char * a_pFile, int a_nLine ) : m_pFile( a_pFile ), m_nLine( a_nLine )
   {}
};

typedef List< ErrorLocation >		IgnoreList;
static IgnoreList                   s_IgnoreList;
static ErrorMode					s_nErrorMode = EM_DEFAULT;

//---------------------------------------------------------------------------------------------------

void SetProcessErrorMode( ErrorMode nMode )
{
	s_nErrorMode = nMode;
}

void ProcessError( const char * a_pMessage, const char * a_pFile, int a_nLine )
{
	if ( s_nErrorMode == EM_DISABLED )
		return;		// all errors are ignored

	// check the ignore list
	bool bIgnore = false;
	if ( a_pFile && a_nLine >= 0 )
	{
		for( IgnoreList::Iterator i = s_IgnoreList.head(); i.valid(); i++ )
			bIgnore = (*i).m_pFile == a_pFile && (*i).m_nLine == a_nLine;
	}

	if (! bIgnore )
	{
		Logging::report( a_pFile, a_nLine, LL_ERROR, "MEDUSA", a_pMessage );

#if defined(_WIN32)
		if ( s_nErrorMode == EM_DIALOG )
		{
			// Show message box and allow user to ignore, retry or abort...
			int nChoice = ::MessageBox( NULL, a_pMessage, "Error", MB_ABORTRETRYIGNORE|MB_SYSTEMMODAL );
			if ( nChoice == IDIGNORE )
			{
				// ignore selected, add to static ignore list and return...
				s_IgnoreList.insert( ErrorLocation( a_pFile, a_nLine ) );
				return;
			}
			else if ( nChoice == IDRETRY )
				return;

			// give debugger a chance to debug the application before exiting...
			if ( nChoice == IDABORT && IsDebuggerPresent() )
				__asm { int 3 };
		}
		else
		{
			if ( IsDebuggerPresent() )
				__asm { int 3 };
		}
#endif
		exit( 1 );
	}
}

//---------------------------------------------------------------------------------------------------
//EOF
