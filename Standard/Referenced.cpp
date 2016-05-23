/*
	Referenced.cpp
	(c)2005 Palestar, Richard Lyle
*/

#include "Referenced.h"
#include "Atomic.h"
#include "CharString.h"

#include "Debug/ExceptionHandler.h"
#include "Debug/Trace.h"
#include "Debug/Assert.h"

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT    0x0400        
#include <windows.h>
#include <Excpt.h>
#include <dbghelp.h>
#endif

#pragma warning( disable : 4355 )	// warning C4355: 'this' : used in base member initializer list

//---------------------------------------------------------------------------------------------------

#if ENABLE_REF_TRACKING
CriticalSection		Referenced::sm_TrackingLock;
bool				Referenced::sm_bEnableTracking = false;
#endif

//-------------------------------------------------------------------------------------------

void Referenced::dumpTracking( CharString & a_sDump )
{
#if ENABLE_REF_TRACKING
	if ( m_pTrackingMap != NULL )
	{
		for( TrackingMap::iterator iTracking = (*m_pTrackingMap).begin(); iTracking != (*m_pTrackingMap).end(); ++iTracking )
		{
			Stack & stack = iTracking->second;
			DumpStack( a_sDump, stack.nStack, sizeof(stack.nStack)/sizeof(stack.nStack[0]) );
		}
	}
#endif
}

//-------------------------------------------------------------------------------
// EOF
