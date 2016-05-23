/*
	Time.h
	(c)2005 Palestar, Richard Lyle
*/

#ifndef TIME_H
#define TIME_H

#include "Standard/Types.h"
#include "Standard/String.h"
#include "MedusaDll.h"

//----------------------------------------------------------------------------

class DLL Time
{
public:
	// Static
	static qword		CPU();							// returns CPU cycles
	static dword		seconds();						// returns the number of seconds since midnight (00:00:00), January 1, 1970
	static dword		milliseconds();					// returns the number of milliseconds since the machine was started

	static WideString	format( dword nSeconds, 
							const wchar * pFormat );	// wrapper for strftime()
	static CharString	format( dword nSeconds, 
							const char * pFormat );		// wrapper for strftime()
	static bool			isTime( dword nSeconds,			// returns the true if mask matches the provided time
							const char * pMask );		// mask is formated as "MON/DAY/YEAR HOUR:MIN:SEC ", * = Wildcard, + = interval (i.e. "*/*/* *:+5:0")
														

	static CharString	time( dword nSeconds );			// converts seconds to a ansi string
	static bool			setTime( dword nSeconds );		// sets the current time on this machine
};


//----------------------------------------------------------------------------



#endif

//----------------------------------------------------------------------------
// EOF

