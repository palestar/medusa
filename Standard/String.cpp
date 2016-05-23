/*
	String.cpp
	(c)2005 Palestar Inc, Richard Lyle
*/

#include "Standard/String.h"
#include "Standard/Limits.h"
#include "Standard/Hash.h"

#include <assert.h>			// We cannot use Debug/Assert.h because that library is dependent on this
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdarg.h>

#if defined(_WIN32)
#include <windows.h>
#pragma warning( disable : 4996 )			// warning C4996: '_vsnwprintf' was declared deprecated
#endif

//----------------------------------------------------------------------------

dword					s_NULL = 0;
void *					StringBase::s_pNULL = &s_NULL;

//----------------------------------------------------------------------------

StringBase::StringHash & StringBase::stringIdHash()
{
	static StringHash stringHash;	
	return stringHash;
}

StringBase::TableQueue & StringBase::stringTableQueue()
{
	static TableQueue queue;
	return queue;
}


bool StringBase::addStringTable( void * pTable )
{
	if ( pTable == NULL )
		return false;		// invalid handle

	stringTableQueue().reset();
	while( stringTableQueue().valid() )
	{
		if ( *stringTableQueue() == pTable )
			return false;
		stringTableQueue().next();
	}

	// add table to the queue
	stringTableQueue().insert( pTable );
	return true;
}

bool StringBase::deleteStringTable( void * pTable )
{
	if ( pTable == NULL )
		return false;		// invalid handle

	stringTableQueue().reset();
	while( stringTableQueue().valid() )
	{
		if ( *stringTableQueue() == pTable )
		{
			stringTableQueue().pop();
			return true;
		}

		stringTableQueue().next();
	}

	// failed to find the table in our queue
	return false;
}

//----------------------------------------------------------------------------

const int MAX_RESOURCE_SIZE = 4 * 1024;

const wchar * StringBase::loadString( dword nId )
{
	// look for the string already in the hash
	StringHash::Iterator it = stringIdHash().find( nId );
	if (! it.valid() )
	{
		wchar sBuffer[ MAX_RESOURCE_SIZE ];

		stringTableQueue().reset();
		while( stringTableQueue().valid() )
		{
			int nSize = StringBase::loadString( *stringTableQueue(), nId, sBuffer, MAX_RESOURCE_SIZE );
			if ( nSize > 0 )
			{
				// insert the loaded string into our hash, call unslash() to remove any escapped characters
				it = stringIdHash().insert( nId, unslash( sBuffer ) );
				// string found, stop while
				break;
			}
			else
				stringTableQueue().next();		// try the next table
		}

		// if string not found in any table, create a dummy string then
		if (! it.valid() )
		{
			StringBase::format( sBuffer, sizeof(sBuffer), L"##%d", nId );
			it = stringIdHash().insert( nId, strdup( sBuffer ) );
		}
	}

	return *it;
}

int StringBase::loadString( void * pInstance, dword nID, wchar * pBuffer, int nBufferMax )
{
#if defined(_WIN32) || defined(_XBOX)
	return LoadStringW( (HINSTANCE)pInstance,nID, pBuffer, nBufferMax ); 
#else
	return 0;
#endif
}

int StringBase::loadString( void * pInstance, dword nID, char * pBuffer, int nBufferMax )
{
#if defined(_WIN32) || defined(_XBOX)
	return LoadStringA( (HINSTANCE)pInstance,nID, pBuffer, nBufferMax ); 
#else
	return 0;
#endif
}

//----------------------------------------------------------------------------

wchar * StringBase::slash( const wchar * pIn )
{
	int nLen = strlen( pIn );

	// allocate a buffer twice the size of the string we are slashing
	wchar * pBuffer = (wchar *)StringBase::malloc( ((nLen * 2) + 1) * sizeof(wchar) );
	wchar * pOutput = pBuffer;

	for(int i=0;i<nLen;i++)
	{
		wchar c = pIn[i];
		switch( c )
		{
		case ('\0'):
			*pOutput++ = ('\\');
			*pOutput++ = ('0');
			break;
		case ('\n'):
			*pOutput++ = ('\\');
			*pOutput++ = ('n');
			break;
		case ('\r'):
			*pOutput++ = ('\\');
			*pOutput++ = ('r');
			break;
		case ('\\'):
			*pOutput++ = ('\\');
			*pOutput++ = ('\\');
			break;
		case ('\''):
			*pOutput++ = ('\\');
			*pOutput++ = ('\'');
			break;
		case ('\"'):
			*pOutput++ = ('\\');
			*pOutput++ = ('\"');
			break;
		case ('\t'):
			*pOutput++ = ('\\');
			*pOutput++ = ('t');
			break;
		case ('\032'):	// EOF
			*pOutput++ = ('\\');
			*pOutput++ = ('z');
			break;
		default:
			*pOutput++ = c;
			break;
		}
	}

	// null terminate the output
	*pOutput = 0;
	// copy the slashed version into this string
	return (wchar *)StringBase::realloc( pBuffer, (strlen( pBuffer ) + 1) * sizeof(wchar) );
}

char * StringBase::slash( const char * pIn )
{
	int nLen = strlen( pIn );

	// allocate a buffer twice the size of the string we are slashing
	char * pBuffer = (char *)StringBase::malloc( ((nLen * 2) + 1) * sizeof(char) );
	char * pOutput = pBuffer;

	for(int i=0;i<nLen;i++)
	{
		char c = pIn[i];
		switch( c )
		{
		case ('\0'):
			*pOutput++ = ('\\');
			*pOutput++ = ('0');
			break;
		case ('\n'):
			*pOutput++ = ('\\');
			*pOutput++ = ('n');
			break;
		case ('\r'):
			*pOutput++ = ('\\');
			*pOutput++ = ('r');
			break;
		case ('\\'):
			*pOutput++ = ('\\');
			*pOutput++ = ('\\');
			break;
		case ('\''):
			*pOutput++ = ('\\');
			*pOutput++ = ('\'');
			break;
		case ('\"'):
			*pOutput++ = ('\\');
			*pOutput++ = ('\"');
			break;
		case ('\t'):
			*pOutput++ = ('\\');
			*pOutput++ = ('t');
			break;
		case ('\032'):	// EOF
			*pOutput++ = ('\\');
			*pOutput++ = ('z');
			break;
		default:
			*pOutput++ = c;
			break;
		}
	}

	// null terminate the output
	*pOutput = 0;
	// copy the slashed version into this string
	return (char *)StringBase::realloc( pBuffer, (strlen( pBuffer ) + 1) * sizeof(char) );
}

wchar * StringBase::unslash( const wchar * pIn )
{
	int nLen = strlen( pIn );

	// allocate a buffer twice the size of the string we are slashing
	wchar * pBuffer = (wchar *)StringBase::malloc( (nLen + 1) * sizeof(wchar) );
	wchar * pOutput = pBuffer;

	for(int i=0;i<nLen;i++)
	{
		wchar c = pIn[i];
		if ( c == ('\\') )
		{
			// skip to the next character
			i++;
			
			wchar code = pIn[ i ];
			switch( code )
			{
			case ('0'):
				*pOutput++ = ('\0');
				break;
			case ('n'):
				*pOutput++ = ('\n');
				break;
			case ('r'):
				*pOutput++ = ('\r');
				break;
			case ('\\'):
				*pOutput++ = ('\\');
				break;
			case ('\''):
				*pOutput++ = ('\'');
				break;
			case ('\"'):
				*pOutput++ = ('\"');
				break;
			case ('t'):
				*pOutput++ = ('\t');
				break;
			case ('z'):
				*pOutput++ = ('\032');
				break;
			default:
				// unknown escape code, just keep in the string intact
				*pOutput++ = ('\\');
				*pOutput++ = c;
				break;
			}
		}
		else
			*pOutput++ = c;
	}

	// null terminate the output
	*pOutput = 0;
	return (wchar *)StringBase::realloc( pBuffer, (strlen( pBuffer ) + 1) * sizeof(wchar) );
}

char * StringBase::unslash( const char * pIn )
{
	int nLen = strlen( pIn );

	// allocate a buffer twice the size of the string we are slashing
	char * pBuffer = (char *)StringBase::malloc( (nLen + 1) * sizeof(char) );
	char * pOutput = pBuffer;

	for(int i=0;i<nLen;i++)
	{
		char c = pIn[i];
		if ( c == ('\\') )
		{
			// skip to the next character
			i++;
			
			wchar code = pIn[ i ];
			switch( code )
			{
			case ('0'):
				*pOutput++ = ('\0');
				break;
			case ('n'):
				*pOutput++ = ('\n');
				break;
			case ('r'):
				*pOutput++ = ('\r');
				break;
			case ('\\'):
				*pOutput++ = ('\\');
				break;
			case ('\''):
				*pOutput++ = ('\'');
				break;
			case ('\"'):
				*pOutput++ = ('\"');
				break;
			case ('t'):
				*pOutput++ = ('\t');
				break;
			case ('z'):
				*pOutput++ = ('\032');
				break;
			default:
				// unknown escape code, just keep in the string intact
				*pOutput++ = ('\\');
				*pOutput++ = c;
				break;
			}
		}
		else
			*pOutput++ = c;
	}

	// null terminate the output
	*pOutput = 0;
	return (char *)StringBase::realloc( pBuffer, (strlen( pBuffer ) + 1) * sizeof(char) );
}

//----------------------------------------------------------------------------

void * StringBase::malloc( unsigned int nBytes )
{
	return ::malloc( nBytes );
}

void * StringBase::realloc( void * p, unsigned int nBytes )
{
	return ::realloc( p, nBytes );
}

void StringBase::free( void * p )
{
	::free( p );
}

//---------------------------------------------------------------------------------------------------

void StringBase::makeUNICODE( const char * pSource, wchar * pDest, int nMax /*= -1*/ )
{
#if defined(_WIN32)
	MultiByteToWideChar( CP_UTF8, 0, pSource, strlen(pSource), pDest, nMax );
#else
	if ( pSource != NULL )
	{
		while( *pSource != 0 && (nMax < 0 || nMax-- > 0) )
			*pDest++ = (wchar)*pSource++;
	}
	*pDest = 0;
#endif
}

void StringBase::makeANSI( const wchar * pSource, char * pDest, int nMax /*= -1*/ )
{
#if defined(_WIN32)
	WideCharToMultiByte( CP_UTF8, 0, pSource, strlen(pSource), pDest, nMax, 0, 0 );
#else
	if ( pSource != NULL )
	{
		while( *pSource != 0 && (nMax < 0 || nMax-- > 0) )
			*pDest++ = (char)*pSource++;
	}
	*pDest = 0;
#endif
}

int StringBase::format( wchar * pBuffer, int nBufferSize, const wchar * pFormat, ... )
{
	va_list va;
	va_start(va, pFormat);
	int nOut = format( pBuffer, nBufferSize, pFormat, va );
	va_end(va);

	return nOut;
}

int StringBase::format( char * pBuffer, int nBufferSize, const char * pFormat, ... )
{
	va_list va;
	va_start(va, pFormat);
	int nOut = format( pBuffer, nBufferSize, pFormat, va );
	va_end(va);

	return nOut;
}

int StringBase::format( wchar * pBuffer, int nBufferSize, const wchar * pFormat, va_list va )
{
#if defined(_WIN32) || defined(_XBOX)
	return _vsnwprintf( pBuffer, nBufferSize, pFormat, va );
#else
	return vswprintf( pBuffer, nBufferSize, pFormat, va );
#endif
}

int StringBase::format( char * pBuffer, int nBufferSize, const char * pFormat, va_list va )
{
#if defined(_WIN32) || defined(_XBOX)
	return _vsnprintf( pBuffer, nBufferSize, pFormat, va );
#else
	return vsnprintf( pBuffer, nBufferSize, pFormat, va );
#endif
}

int	StringBase::strint( const wchar * p, int nBase /*= 10*/ )
{
	return wcstol( p, NULL, nBase );
}

int StringBase::strint( const char * p, int nBase /*= 10*/ )
{
	return strtol( p, NULL, nBase );
}

dword StringBase::strdword( const wchar * p, int nBase /*= 10*/ )
{
	return wcstoul( p, NULL, nBase );
}

dword StringBase::strdword( const char * p, int nBase /*= 10*/ )
{
	return strtoul( p, NULL, nBase );
}

qword StringBase::strqword( const wchar * p, int nBase /*= 10*/ )
{
	qword n = 0;
	if ( nBase == 10 )
		swscanf( p, L"%llu", &n );
	else if ( nBase == 16 )
		swscanf( p, L"%llx", &n );
	return n;
}

qword StringBase::strqword( const char * p, int nBase /*= 10*/ )
{
	qword n = 0;
	if ( nBase == 10 )
		sscanf( p, "%llu", &n );
	else if ( nBase == 16 )
		sscanf( p, "%llx", &n );
	return n;
}

float StringBase::strfloat( const wchar * p )
{
	return (float)wcstod( p, NULL );
}

float StringBase::strfloat( const char * p )
{
	return (float)strtod( p, NULL );
}

double StringBase::strdouble( const wchar * p )
{
	return wcstod( p, NULL );
}

double StringBase::strdouble( const char * p )
{
	return strtod( p, NULL );
}

//-------------------------------------------------------------------------------
// EOF
