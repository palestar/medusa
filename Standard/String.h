/*
	StringTemplate.h

	StringTemplate Class is used to contain a string in ansi or wide format.
	(c)2005 Palestar, Richard Lyle
*/

#ifndef STRING_H
#define STRING_H

#include "StringHelpers.h"
#include "Types.h"
#include "Hash.h"
#include "Queue.h"
#include "Limits.h"
#include "Exception.h"
#include "Array.h"

#include <stdarg.h>						// va_list
#include <stdio.h>
#include <memory.h>						// memset
#include <stdlib.h>

#pragma warning( disable : 4251 )		// Dll export warning on template classes
#pragma warning( disable : 4996 )		// This function or variable may be unsafe. Consider using strncpy_s instead. To disable deprecation, use _CRT_SECURE_NO_WARNINGS. See online help for details.

#include "MedusaDll.h"

//----------------------------------------------------------------------------

// The following macro can be used for all text contained in code that is used by the StringTemplate() class
#define STR( string )		L##string  

// Types
enum {
	FORMAT_BUFFER_SIZE	= 128 * 1024,
};

//----------------------------------------------------------------------------

class DLL StringBase
{
public:
	DECLARE_EXCEPTION( CharBufferOverflow );
	DECLARE_EXCEPTION( CharOutOfRange );
	DECLARE_EXCEPTION( ResourceNotFound );

	// Types
	typedef Hash< dword, wchar * >	StringHash;
	typedef Queue< void * >			TableQueue;

	// Data
	static void *			s_pNULL;

	// Static
	static StringHash &		stringIdHash();							// get the string Id hash
	static TableQueue &		stringTableQueue();						// queue of all the added string tables

	static bool				addStringTable( void * pTable );		// add string table, pInstance should be the HINSTANCE of the module with the string table
	static bool				deleteStringTable( void * pTable );		// remove string table

	static const wchar *	loadString( dword stringId );
	static int				loadString( void * pInstance, dword nID, wchar * pBuffer, int nBufferMax );
	static int				loadString( void * pInstance, dword nID, char * pBuffer, int nBufferMax );

	static wchar *			slash( const wchar * pIn );
	static char *			slash( const char * pIn );
	static wchar *			unslash( const wchar * pIn );
	static char *			unslash( const char * pIn );
	
	static void *			malloc( unsigned int nBytes );
	static void *			realloc( void * p, unsigned int nBytes );
	static void				free( void * p );

	static void				makeUNICODE( const char * pSource, wchar * pDest, int nMax = -1 );
	static void				makeANSI( const wchar * pSource, char * pDest, int nMax = -1 );
	
	static int				format( wchar * pBuffer, int nBufferSize, const wchar * pFormat, ... );
	static int				format( char * pBuffer, int nBufferSize, const char * pFormat, ... );
	static int				format( wchar * pBuffer, int nBufferSize, const wchar * pFormat, va_list va );
	static int				format( char * pBuffer, int nBufferSize, const char * pFormat, va_list va );

	static int				strint( const wchar * p, int nBase = 10 );
	static int				strint( const char * p, int nBase = 10 );
	static dword			strdword( const wchar * p, int nBase = 10 );
	static dword			strdword( const char * p, int nBase = 10 );
	static qword			strqword( const wchar * p, int nBase = 10 );
	static qword			strqword( const char * p, int nBase = 10 );
	static float			strfloat( const wchar * p );
	static float			strfloat( const char * p );
	static double			strdouble( const wchar * p );
	static double			strdouble( const char * p );
};

//----------------------------------------------------------------------------

template<typename C>
class StringTemplate : public StringBase
{
public:
	// Construction
	StringTemplate();
	StringTemplate( const char * p );
	StringTemplate( const wchar * p );
	StringTemplate( const StringTemplate< char > & s );
	StringTemplate( const StringTemplate< wchar > & s );

	~StringTemplate();

	// Accessors
	int					length() const;								// number of characters in the string
	C *					buffer() const;								// get the string buffer
	const C *			cstr() const;
	
	int					compare( const C * pCmp ) const;			// strcmp()
	int					compareNoCase( const C * pCmp ) const;		// stricmp()
	int					compareCommand( const C * pCmp ) const;		// strnicmp(), uses the length of the shorter string

	int					find( C ch, int nOffset = 0, 
							bool bIgnoreCase = false ) const;		// find character
	int					find( const C * pFind, int n = 0, 
							bool bIgnoreCase = false ) const;		// find string
	int					reverseFind( C ch, int n = -1, 
							bool bIgnoreCase = false ) const;		// find from the end

	bool				beginsWith( const C * pCmp ) const;
	bool				endsWith( const C * pCmp ) const;

	bool				operator==( const C * pCmp ) const;			// case-senstive compare
	bool				operator^=( const C * pCmp ) const;			// case-insenstive compare
	bool				operator!=( const C * pCmp ) const;
	bool				operator>( const C * pCmp ) const;
	bool				operator>=( const C * pCmp ) const;
	bool				operator<( const C * pCmp ) const;
	bool				operator<=( const C * pCmp ) const;

	C					operator[]( int ch ) const;
	operator			const C *() const;		

	int					split( Array< StringTemplate > & parts,
							const StringTemplate & w ) const;		// seperate this string and store the parts into the given array

	// Mutators
	StringTemplate &	format( const char * pFormat, ... );
	StringTemplate &	format( const wchar * pFormat, ... );
	StringTemplate & 	format( dword stringId, ... );

	bool				tokenize( StringTemplate & t, 
							const StringTemplate & w );				// get next token from string into t, seperated by w
	StringTemplate &	allocate( C * pString );					// give the allocated string to this object, previous string is freed
	StringTemplate &	allocate( int nSize );
	StringTemplate &	allocate( C fill, int nSize );
	StringTemplate &	realloc( int nSize );
	void				release();

	StringTemplate &	append( C ch );								// append another character

	StringTemplate &	left(int n);								// keep the left n characters
	StringTemplate &	right(int n);								// keep the right n characters
	StringTemplate &	mid(int begin, int n = -1);					// keep the n characters at begin 
	StringTemplate &	insert( C ch, int index );					// insert character
	StringTemplate &	insert( const StringTemplate & s, int n );			// insert string at index n
	StringTemplate &	remove( int index );						// remove character
	StringTemplate &	remove( int index, int len );				// remove characters

	StringTemplate &	replace( int i, int n, 
							const StringTemplate & s );				// replace characters starting at i of len n with string s
	StringTemplate &	replace( const StringTemplate & f, 
							const StringTemplate & s,
							bool bIgnoreCase = false );				// replace string f with string s
	StringTemplate &	replace( C s, C r );						// replace all occurrences of the character s with r

	StringTemplate &	slash();									// escape special characters in the string
	StringTemplate &	unslash();									// remove escaped characters

	StringTemplate &	trimLeft();									// remove leading whitespace
	StringTemplate &	trimRight();								// remove trailing whitespace
	StringTemplate &	trim();										// trim left and right

	void				empty();									// release the contents
	StringTemplate &	lower();									// make all lower case
	StringTemplate &	upper();									// make all upper case
	StringTemplate &	pretty();									// make first character upper, the rest lower

	C &					operator[](int ch);							// allocates additional space if character is out of range
	StringTemplate &	operator=( const char * p );
	StringTemplate &	operator=( const wchar * p );
	StringTemplate &	operator=( const StringTemplate< char > & s );
	StringTemplate &	operator=( const StringTemplate< wchar > & s );
	StringTemplate &	operator+=( C ch );
	StringTemplate &	operator+=( const char * p );
	StringTemplate &	operator+=( const wchar * p );
	StringTemplate &	operator+=( const StringTemplate< char > & s );
	StringTemplate &	operator+=( const StringTemplate< wchar > & s );

	//----------------------------------------------------------------------------

	template<typename K>
	StringTemplate & copy( const StringTemplate<K> & s )
	{
		int nSize = s.length();
		if ( nSize > 0 )
		{
			// KEEP: allocate the memory first, the assigment may be from it's own buffer.
			C * pString = (C *)StringBase::malloc( (nSize + 1) * sizeof(C) );

			int i = 0;
			for(;i<nSize;i++)
				pString[i] = (C)s[i];
			pString[i] = 0;

			allocate( pString );
		}
		else
			release();

		return *this;
	}
	template<typename K>
	StringTemplate & copy( const K * p )
	{
		// handle cases when the string is assigned to itself...
		if ( m_pString == (const C *)p )
			return *this;

		int nSize = (int)strlen( p );
		if ( nSize > 0 )
		{
			// KEEP: allocate the memory first, the assigment may be from it's own buffer.
			C * pString = (C *)StringBase::malloc( (nSize + 1) * sizeof(C) );

			int i = 0;
			for(;i<nSize;i++)
				pString[i] = (C)p[i];
			pString[i] = 0;

			allocate( pString );
		}
		else
			release();

		return *this;
	}
	template<typename K>
	StringTemplate & append( const K * p )
	{
		int nLen = length();
		int nSize = (int)strlen( p );

		realloc( nLen + nSize );
		int i = 0;
		for(;i<nSize;i++)
			m_pString[nLen + i] = (C)p[i];
		m_pString[nLen + i] = 0;

		return *this;
	}
	template<typename K>
	StringTemplate &	append( const StringTemplate<K> & s )
	{
		int nLen = length();
		int nSize = s.length();

		realloc( nLen + nSize );
		int i = 0;
		for(;i<nSize;i++)
			m_pString[nLen + i] = (C)s[i];
		m_pString[nLen + i] = 0;

		return *this;
	}

private:
	// data
	C *					m_pString;									// string data, NULL terminated at all times
};

//----------------------------------------------------------------------------

template<typename C>
inline StringTemplate<C>::StringTemplate() : m_pString((C *)StringBase::s_pNULL)
{}

template<typename C>
inline StringTemplate<C>::StringTemplate( const char * p ) : m_pString((C *)StringBase::s_pNULL)
{
	copy( p );
}

template<typename C>
inline StringTemplate<C>::StringTemplate( const wchar * p ) : m_pString((C *)StringBase::s_pNULL)
{
	copy( p );
}

template<typename C>
inline StringTemplate<C>::StringTemplate( const StringTemplate< char > & s ) : m_pString((C *)StringBase::s_pNULL)
{
	copy( s );
}

template<typename C>
inline StringTemplate<C>::StringTemplate( const StringTemplate< wchar > & s ) : m_pString((C *)StringBase::s_pNULL)
{
	copy( s );
}

template<typename C>
inline StringTemplate<C>::~StringTemplate()
{
	// release the string, if any
	release();
}

//----------------------------------------------------------------------------

template<typename C>
inline int StringTemplate<C>::length() const
{
	return (int)strlen( m_pString );
}

template<typename C>
inline C * StringTemplate<C>::buffer() const
{ 
	return m_pString; 
}

template<typename C>
inline const C * StringTemplate<C>::cstr() const
{
	return m_pString;
}

template<typename C>
inline int StringTemplate<C>::compare(  const C * pCmp  ) const
{
	return strcmp( m_pString, pCmp );
}

template<typename C>
inline int StringTemplate<C>::compareNoCase(  const C * pCmp  ) const
{
	return stricmp<C>( m_pString, pCmp );
}

template<typename C>
inline int StringTemplate<C>::compareCommand(  const C * pCmp  ) const
{
	return strnicmp<C>( m_pString, pCmp, Min<int>( length(), strlen( pCmp ) ) );
}

template<typename C>
inline int StringTemplate<C>::find( C ch, int nOffset /*= 0*/, bool bIgnoreCase /*= false*/ ) const
{
	int nLen = length();
	if ( nOffset < 0 )
		nOffset = 0;
	if ( nOffset >= nLen )
		return -1;
	C * pFound = bIgnoreCase ? strichr( m_pString + nOffset, ch ) : ((char*) strchr( m_pString + nOffset, ch));
	if ( pFound == NULL )
		return -1;
	return pFound - m_pString;
}

template<typename C>
inline int StringTemplate<C>::find( const C * p, int n /*= 0*/, bool bIgnoreCase /*= false*/ ) const
{
	int nLen = strlen( p );
	if ( nLen <= 0 )
		return -1;
	if ( n < 0 )
		n = 0;
	if ( n >= length() )
		return -1;
	if ( nLen == 1 )
		return find(p[0],n);

	C * pFound = bIgnoreCase ? stristr( m_pString + n, p ) : ((char*) strstr( m_pString + n, p ));
	if ( pFound == NULL )
		return -1;

	return pFound - m_pString;
}

template<typename C>
inline int StringTemplate<C>::reverseFind( C ch, int n /*= -1*/, bool bIgnoreCase /*= false*/ ) const
{
	int nLength = (int)strlen( m_pString );
	if ( n < 0 || n > nLength )
		n = nLength;

	if ( bIgnoreCase )
	{
		ch = tolower( ch );
		for(int k=(n - 1);k >= 0; k-- )
		{
			if ( tolower( m_pString[k] ) == ch )
				return k;
		}
	}
	else
	{
		for(int k=(n - 1);k >= 0; k-- )
		{
			if ( m_pString[k] == ch )
				return k;
		}
	}

	return -1;
}

template<typename C>
inline bool StringTemplate<C>::beginsWith( const C * p ) const
{
	return strncmp( m_pString, p, strlen(p) ) == 0;
}

template<typename C>
inline bool StringTemplate<C>::endsWith( const C * p ) const
{
	int k = length();
	if ( k <= 0 )
		return false;
	int n = (int)strlen( p );
	if ( n <= 0 )
		return false;

	return strncmp( m_pString + k - n, p, n ) == 0;
}

template<typename C>
inline bool StringTemplate<C>::operator==( const C * p   ) const
{ 
	return compare( p ) == 0; 
}

template<typename C>
inline bool StringTemplate<C>::operator^=( const C * p   ) const
{
	return compareNoCase( p ) == 0;
}

template<typename C>
inline bool	StringTemplate<C>::operator!=( const C * p  ) const		
{ 
	return compare( p ) != 0; 
}

template<typename C>
inline bool StringTemplate<C>::operator>( const C * p ) const		
{ 
	return compare( p ) > 0; 
}

template<typename C>
inline bool StringTemplate<C>::operator>=( const C * p ) const	
{ 
	return compare( p ) >= 0; 
}

template<typename C>
inline bool StringTemplate<C>::operator<( const C * p ) const		
{ 
	return compare( p ) < 0; 
}

template<typename C>
inline bool StringTemplate<C>::operator<=( const C * p ) const		
{ 
	return compare( p ) <= 0; 
}

template<typename C>
inline C StringTemplate<C>::operator[]( int n ) const
{
	return m_pString[ n ];
}

template<typename C>
inline StringTemplate<C>::operator const C *() const
{ 
	return m_pString; 
}	

template<typename C>
inline int StringTemplate<C>::split( Array< StringTemplate > & parts,
						const StringTemplate & w ) const
{
	StringTemplate<C> copy( *this );

	StringTemplate<C> token;
	while( copy.tokenize( token, w ) )
		parts.push( token );

	return parts.size();
}

//-------------------------------------------------------------------------------

template<typename C>
inline StringTemplate<C> & StringTemplate<C>::format( const char * pFormat, ... )
{
	va_list va;
	va_start(va, pFormat);

	char sBuffer[ FORMAT_BUFFER_SIZE ];
	if ( StringBase::format( sBuffer, FORMAT_BUFFER_SIZE, pFormat, va ) < 0 )
		throw StringBase::CharBufferOverflow();
	va_end(va);

	return copy( sBuffer );
}

template<typename C>
inline StringTemplate<C> & StringTemplate<C>::format( const wchar * pFormat, ... )
{
	va_list va;
	va_start(va, pFormat);

	wchar sBuffer[ FORMAT_BUFFER_SIZE ];
	if ( StringBase::format( sBuffer, FORMAT_BUFFER_SIZE, pFormat, va ) < 0 )
		throw StringBase::CharBufferOverflow();

	va_end(va);

	return copy( sBuffer );
}

template<typename C>
inline StringTemplate<C> &  StringTemplate<C>::format( dword stringId, ... )
{
	va_list va;
	va_start(va, stringId);

	wchar sBuffer[ FORMAT_BUFFER_SIZE ];
	if ( StringBase::format( sBuffer, FORMAT_BUFFER_SIZE, StringBase::loadString( stringId ), va ) < 0 )
		throw StringBase::CharBufferOverflow();

	va_end(va);

	return copy( sBuffer );
}

template<typename C>
inline bool StringTemplate<C>::tokenize( StringTemplate<C> & t, const StringTemplate<C> & w )
{
	int nLen = length();
	if ( nLen > 0 )
	{
		C * pBegin = m_pString + strcspn( m_pString, w.buffer() );
		C * pEnd = pBegin;

		// parse past whitespace
		while( strlen(pEnd) > 0 && strcspn(pEnd, w.buffer()) == 0 )
			pEnd++;

		int nTokenLen = pBegin - m_pString;

		C * pToken = (C *)StringBase::malloc( (nTokenLen + 1) * sizeof(C) );
		strncpy( pToken, m_pString, nTokenLen );
		pToken[ nTokenLen ] = 0;

		t.allocate( pToken );

		right(nLen - (pEnd - m_pString));
		return true;
	}

	return false;
}

template<typename C>
inline StringTemplate<C> & StringTemplate<C>::allocate( C * pString )
{
	if ( m_pString != pString )
	{
		release();
		m_pString = pString;
	}
	return *this;
}

template<typename C>
inline StringTemplate<C> & StringTemplate<C>::allocate( int nSize )
{
	release();

	m_pString = (C *)StringBase::malloc( (nSize + 1) * sizeof(C) );
	m_pString[ nSize ] = 0;

	return *this;
}

template<typename C>
inline StringTemplate<C> & StringTemplate<C>::allocate( C fill, int nSize )
{
	allocate( nSize );
	int i = 0;
	for(;i<nSize;i++)
		m_pString[i] = fill;

	return *this;
}

template<typename C>
inline StringTemplate<C> & StringTemplate<C>::realloc( int nSize )
{
	if ( m_pString == (C *)StringBase::s_pNULL )
		return allocate( nSize );

	m_pString = (C *)StringBase::realloc( m_pString, (nSize + 1) * sizeof(C) );
	m_pString[ nSize ] = 0;

	return *this;
}

template<typename C>
inline void StringTemplate<C>::release()
{
	if ( m_pString != (C *)StringBase::s_pNULL )
	{
		StringBase::free( m_pString );
		m_pString = (C *)StringBase::s_pNULL;
	}
}

template<typename C>
inline StringTemplate<C> & StringTemplate<C>::append( C ch )
{
	int n = length();
	
	realloc( n + 1 );
	m_pString[ n ] = ch;
	m_pString[ n + 1 ] = 0;

	return *this;
}

template<typename C>
inline StringTemplate<C> & StringTemplate<C>::left(int n)
{
	if ( n == 0 )
	{
		release();
		return *this;
	}

	int nLength = (int)strlen( m_pString );
	if (n >= nLength)
		return *this;
	realloc( n );
	m_pString[ n ] = 0;

	return *this;
}

template<typename C>
inline StringTemplate<C> & StringTemplate<C>::right(int n)
{
	if ( n == 0 )
	{
		release();
		return *this;
	}

	int nLength = strlen( m_pString );
	if ( n >= nLength )
		return *this;

	C * pString = (C *)StringBase::malloc( (n + 1) * sizeof(C) );
	memcpy( pString, m_pString + (nLength - n), sizeof(C) * n );
	pString[ n ] = 0;

	allocate( pString );
	return *this;
}

template<typename C>
inline StringTemplate<C> & StringTemplate<C>::mid(int nBegin,int n /*= -1*/)
{
	int nLength = strlen( m_pString );
	if ( nBegin < 0 )
		nBegin = 0;
	if ( n < 0 || ( nBegin + n ) > nLength )
		n = nLength - nBegin;

	if ( n <= 0 )
	{
		release();
		return *this;
	}

	C * pString = (C *)StringBase::malloc( (n + 1) * sizeof(C) );
	memcpy( pString, m_pString + nBegin, sizeof(C) * n);
	pString[ n ] = 0;

	allocate( pString );
	return *this;
}

template<typename C>
inline StringTemplate<C> & StringTemplate<C>::insert( C ch, int nIndex )
{
	if (nIndex < 0 )
		return *this;

	int nLength = length();
	int nNewSize = Max(nLength + 1,nIndex + 1);
	C * pString = (C *)StringBase::malloc( (nNewSize + 1) * sizeof(C) );

	memset( pString, 0, sizeof(C) * (nNewSize + 1) );
	memcpy( pString, m_pString, sizeof(C) * Min(nIndex,nLength) );
	pString[ nIndex ] = ch;
	if ( nIndex < nLength )
		memcpy( pString + nIndex + 1, m_pString + nIndex, sizeof(C) * (nLength - nIndex) );

	allocate( pString );
	return *this;
}

template<typename C>
inline StringTemplate<C> & StringTemplate<C>::insert( const StringTemplate & s, int nIndex )
{
	replace( nIndex, 0, s );
	return *this;
}

template<typename C>
inline StringTemplate<C> & StringTemplate<C>::remove( int nIndex )
{
	remove( nIndex, 1 );
	return *this;
}

template<typename C>
inline StringTemplate<C> & StringTemplate<C>::remove( int nIndex, int nLen )
{
	replace( nIndex, nLen, "" );
	return *this;
}

template<typename C>
inline StringTemplate<C> & StringTemplate<C>::replace( int nIndex, int nLen, const StringTemplate & s )
{
	int nLength = length();
	if (nIndex < 0 || nLen < 0 || nIndex >= nLength  || nIndex + nLen > nLength )
		return *this;
	
	int nInsertLength = s.length();
	int nNewSize = (nLength + nInsertLength) - nLen;
	C * pString = (C *)StringBase::malloc( (nNewSize + 1) * sizeof(C) );

	memcpy( pString, m_pString, sizeof(C) * nIndex );
	memcpy( pString + nIndex, s.buffer(), sizeof(C) * nInsertLength );
	memcpy( pString + nIndex + nInsertLength, m_pString + nIndex + nLen, sizeof(C) * (nLength - (nIndex + nLen)) );
	pString[ nNewSize ] = 0;	

	allocate( pString );
	return *this;
}

template<typename C>
inline StringTemplate<C> & StringTemplate<C>::replace( const StringTemplate & f, const StringTemplate & r, bool bIgnoreCase /*= false*/ )
{
	int nLen = f.length();
	if( nLen == 0 )
		return *this;
	int nReplaceLen = r.length();

	for(int n = find( f, 0, bIgnoreCase );n >= 0; n = find( f, n + nReplaceLen + 1, bIgnoreCase ) )
		replace( n , nLen, r );

	return *this;
}

template<typename C>
inline StringTemplate<C> & StringTemplate<C>::replace( C s, C r )
{
	int nLen = length();
	if ( nLen > 0 )
	{
		for(int n=0;n<nLen;n++)
		{
			if ( m_pString[n] == s )
				m_pString[n] = r;
		}
	}

	return *this;
}

template<typename C>
inline StringTemplate<C> & StringTemplate<C>::slash()
{
	return allocate( StringBase::slash( m_pString ) );
}

template<typename C>
inline StringTemplate<C> & StringTemplate<C>::unslash()
{
	return allocate( StringBase::unslash( m_pString ) );
}

template<typename C>
inline StringTemplate<C> & StringTemplate<C>::trimLeft()
{
	int i=0;
	while( strchr( L" \r\n\t", m_pString[i] ) != NULL )
		i++;

	return right( length() - i );
}

template<typename C>
inline StringTemplate<C> & StringTemplate<C>::trimRight()
{
	int i= length();
	while( i > 0 && strchr( L" \r\n\t", m_pString[i - 1] ) != NULL )
		i--;
		
	return left( i );
}

template<typename C>
inline StringTemplate<C> & StringTemplate<C>::trim()
{
	trimLeft();
	trimRight();
	return *this;
}

template<typename C>
inline void StringTemplate<C>::empty()
{
	release();
}

template<typename C>
inline StringTemplate<C> & StringTemplate<C>::lower()
{
	strlwr<C>( m_pString );
	return *this;
}

template<typename C>
inline StringTemplate<C> & StringTemplate<C>::upper()
{
	strupr<C>( m_pString );
	return *this;
}

template<typename C>
inline StringTemplate<C> & StringTemplate<C>::pretty()
{
	strlwr<C>( m_pString );
	m_pString[0] = (C)towupper( m_pString[0] );
	return *this;
}

//----------------------------------------------------------------------------

template<typename C>
inline C & StringTemplate<C>::operator[](int n)
{
	if (n < 0 )
		n = 0;
	if ( n >= length() )
	{
		realloc( n + 1 );
		m_pString[ n ] = 0;
	}

	return m_pString[n];
}

template<typename C>
inline StringTemplate<C> & StringTemplate<C>::operator=( const char * p )
{
	return copy( p );
}

template<typename C>
inline StringTemplate<C> & StringTemplate<C>::operator=( const wchar * p )
{
	return copy( p );
}

template<typename C>
inline StringTemplate<C> & StringTemplate<C>::operator=( const StringTemplate< char > & s )
{
	return copy( s );
}

template<typename C>
inline StringTemplate<C> & StringTemplate<C>::operator=( const StringTemplate< wchar > & s )
{
	return copy( s );
}

template<typename C>
inline StringTemplate<C> & StringTemplate<C>::operator+=( C ch )
{
	return append( ch );
}

template<typename C>
inline StringTemplate<C> & StringTemplate<C>::operator+=( const char * p )
{
	return append( p );
}

template<typename C>
inline StringTemplate<C> & StringTemplate<C>::operator+=( const wchar * p )
{
	return append( p );
}

template<typename C>
inline StringTemplate<C> & StringTemplate<C>::operator+=( const StringTemplate< char > & s )
{
	return append( s );
}

template<typename C>
inline StringTemplate<C> & StringTemplate<C>::operator+=( const StringTemplate< wchar > & s )
{
	return append( s );
}

//----------------------------------------------------------------------------

template<typename C,typename K>
StringTemplate<C> operator+( const StringTemplate<C> & s, const StringTemplate<K> & a )
{
	StringTemplate<C> r;
	r.copy( s );
	r.append( a );
	return r;
}

template<typename C,typename K>
StringTemplate<C> operator+( const StringTemplate<C> & s, const K * p )
{
	StringTemplate<C> r;
	r.copy( s );
	r.append( p );
	return r;
}

template<typename C,typename N>
inline StringTemplate<C> FormatNumber( N nInteger )
{
	if ( nInteger == 0 )
		return StringTemplate<C>( "0" );

	bool bNegative = false;
	if ( nInteger < 0 )
	{
		bNegative = true;
		nInteger = -nInteger;
	}
	
	// convert the number into a readable format with comma's
	StringTemplate<C> sNumber;
	while( nInteger > 0)
	{
		N nThousands = nInteger % 1000;
		nInteger /= 1000;

		StringTemplate<C> sThousands;
		if (nInteger > 0)
			sThousands.format("%3.3u", nThousands);
		else
			sThousands.format("%u", nThousands );

		if (sNumber.length() > 0)
			sNumber = sThousands + StringTemplate<C>(",") + sNumber;
		else
			sNumber = sThousands;
	}

	if ( bNegative )
		sNumber = StringTemplate<C>("-") + sNumber;
	return sNumber;
}

//----------------------------------------------------------------------------

typedef StringTemplate<char>	CharString;
typedef StringTemplate<wchar>	WideString;
typedef StringTemplate<wchar>	String;										// depreciated, use WideString instead..

//----------------------------------------------------------------------------



#endif

//-------------------------------------------------------------------------------
// EOF
