/*
	StringHash.cpp
	(c)2005 Palestar Inc, Richard Lyle
*/

#include "Standard/StringHash.h"
#include "Standard/StringHash64.h"



//----------------------------------------------------------------------------

dword StringHash::makeHash( const char * pString, dword nSeed /*= 0*/ )
{
	dword nHash = nSeed;

	byte c = *pString++;
	while( c != 0 )
	{
		if ( c >= 'a' && c <='z' )
			c += 'A' - 'a';				// convert to upper case
		else if ( c == '/' )
			c = '\\';					// convert all slashes to DOS style

		nHash ^= c;
		nHash = (nHash << 5) | ((nHash & 0xf8000000) >> 27);					// rotate left 5

		c = *pString++;
	}

	return nHash;
}

//----------------------------------------------------------------------------

qword StringHash64::makeHash( const char * pString, qword nSeed /*= 0*/ )
{
	dword nHashLow = (dword)(nSeed & 0xffffffff);
	dword nHashHigh = (dword)(nSeed >> 32);

	byte c = *pString++;
	while( c != 0 )
	{
		if ( c >= 'a' && c <='z' )
			c += 'A' - 'a';				// convert to upper case
		else if ( c == '/' )
			c = '\\';					// convert all slashes to DOS style

		nHashLow ^= c;
		nHashLow = (nHashLow << 5) | ((nHashLow & 0xf8000000) >> 27);			// rotate left 5 
		nHashHigh ^= c;
		nHashHigh = (nHashHigh << 3) | ((nHashHigh & 0xe0000000) >> 29);		// rotate left 3

		c = *pString++;
	}

	return (((qword)nHashHigh) << 32) | nHashLow;
}

//----------------------------------------------------------------------------
//EOF
