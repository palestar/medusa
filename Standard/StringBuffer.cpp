/*
	StringBuffer.cpp

	StringBuffer Class
	(c)2005 Palestar
*/

#include "Standard/StringHelpers.h"
#include "Standard/StringBuffer.h"
#include "Standard/CriticalSection.h"

#include <stdio.h>

//----------------------------------------------------------------------------

wchar				StringBuffer::s_Empty = 0;				// empty strings point to this data

//----------------------------------------------------------------------------

StringBuffer::StringBuffer() : m_String(&s_Empty), m_Capacity(0)
{}

StringBuffer::StringBuffer( dword initialCapacity )
{
	m_Capacity = initialCapacity;
	m_String = new wchar[ m_Capacity ];
	m_String[0] = 0;
}

StringBuffer::~StringBuffer()
{
	// release the string, if any
	release();
}

//----------------------------------------------------------------------------

StringBuffer & StringBuffer::copy( const wchar * pString, int length )
{
	ensureCapacity( length + 1 );

	strncpy( m_String, pString, length );
	m_String[length] = 0;

	return *this;
}

StringBuffer & StringBuffer::append(const wchar * append)
{
	int thisStringLength = strlen( m_String );
	int appendStringLength = strlen( append );

	ensureCapacity( thisStringLength + appendStringLength + 1 );
	strcat( m_String, append );

	return *this;
}

StringBuffer & StringBuffer::append( wchar ch )
{
	dword currLen = length();
	ensureCapacity( currLen + 2 );

	m_String[ currLen ] = ch;
	m_String[ currLen + 1 ] = 0;

	return *this;
}

StringBuffer & StringBuffer::appendFormatted( const wchar * pFormat, ... )
{
	va_list va;
	va_start(va, pFormat);

	appendFormatVA(pFormat, va);

	va_end(va);

	return *this;
}

//----------------------------------------------------------------------------

// Only use this to grow, never shrink
StringBuffer & StringBuffer::ensureCapacity( dword capacity )
{
	if( capacity > m_Capacity )
	{
		wchar * pNew = new wchar[ 2 * capacity ];
		strcpy( pNew, m_String );
		if( m_String != &s_Empty)
			delete [] m_String;
		m_String = pNew;
		m_Capacity = 2 * capacity;
	}

	return *this;
}

void StringBuffer::release()
{
	if ( m_String != &s_Empty )
	{
		delete [] m_String;
		m_String = &s_Empty;
		m_Capacity = 0;
	}
}

//----------------------------------------------------------------------------

StringBuffer & StringBuffer::lower()
{
	strlwr( m_String );
	return *this;
}

//-------------------------------------------------------------------------------

const int CHAR_BUFFER_SIZE = 128 * 1024;

void StringBuffer::appendFormatVA( const wchar * pFormat, va_list va)
{
	assert( pFormat );

	wchar sBuffer[ CHAR_BUFFER_SIZE ];
	StringBase::format( sBuffer, sizeof(sBuffer), pFormat, va );

	// validated then assign the string
	int stringLength = strlen( sBuffer );
	if (stringLength >= CHAR_BUFFER_SIZE)
		throw CharBufferOverflow();

	int thisLength = length();
	ensureCapacity( thisLength + stringLength + 1 );
	strcat<wchar>( m_String, sBuffer );
}


//-------------------------------------------------------------------------------
// EOF
