/*
	StringBuffer.h

	StringBuffer Class
	(c)2005 Palestar
*/

#ifndef STRINGBUFFER_H
#define STRINGBUFFER_H

#include "Standard/Types.h"
#include "Standard/String.h"
#include "MedusaDll.h"

//----------------------------------------------------------------------------

// NOTE: The internal format is maintained as 8 bit-character

class DLL StringBuffer
{
public:
	// Exceptions
	class CharBufferOverflow {};
	class CharOutOfRange {};
	class ResourceNotFound {};
	
	// Construction
	StringBuffer();
	StringBuffer( dword initialCapacity );

	~StringBuffer();

	// Accessors
	int						length() const;
	wchar *					buffer() const;
	operator				const wchar *() const;

	wchar					operator[]( int ch ) const;

	// Mutators
	StringBuffer &			copy( const wchar * pString );		// copy string into this string
	StringBuffer &			copy( const wchar * pString, int length );
	StringBuffer &			append( const wchar * append );		// append another string onto this
	StringBuffer &			append( wchar ch );					// append another character
	StringBuffer &			appendFormatted( const wchar * pFormat, ... );

	StringBuffer &			lower();

	wchar &					operator[](int ch);				
	StringBuffer &			operator=( const wchar * src);
	StringBuffer &			operator=( const String & src);
	StringBuffer &			operator+=( const wchar * append);
	StringBuffer &			operator+=( wchar ch );

private:
	// data
	wchar *					m_String;							// string data, NULL terminated at all times
	// static
	static wchar			s_Empty;							// empty strings point to this data
	dword 					m_Capacity;							// capacity this string has

	// mutators
	StringBuffer &			ensureCapacity( dword capacity );
	void					appendFormatVA( const wchar *format,va_list va );
	void					release();
};

//----------------------------------------------------------------------------

inline int StringBuffer::length() const
{
	return( strlen( m_String ) );
}

inline wchar * StringBuffer::buffer() const
{ 
	return( m_String ); 
}

inline StringBuffer::operator const wchar *() const
{ 
	return( m_String ); 
}		

inline wchar StringBuffer::operator[]( int ch ) const
{
	if (ch < 0 || ch >= length() )
		throw CharOutOfRange();
	return( m_String[ch] );
}

//-------------------------------------------------------------------------------

inline StringBuffer & StringBuffer::operator=(const wchar * pSrc)
{
	copy( pSrc );
	return( *this );
}

inline StringBuffer & StringBuffer::operator=(const String & src )
{
	copy( src );
	return( *this );
}

inline StringBuffer & StringBuffer::operator+=(const wchar * src)
{
	append( src );
	return( *this );
}

inline StringBuffer & StringBuffer::operator+=( wchar ch )
{
	append( ch );
	return( *this );
}

//----------------------------------------------------------------------------

inline StringBuffer & StringBuffer::copy( const wchar * pString )
{
	return copy( pString, strlen( pString ) );
}



#endif

//-------------------------------------------------------------------------------
// EOF
