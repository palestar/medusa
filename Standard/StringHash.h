/*
	StringHash.h
	(c)2005 Palestar, Richard Lyle
*/

#ifndef STRING_HASH_H
#define STRING_HASH_H

#include "Standard/String.h"
#include "Standard/Types.h"
#include "MedusaDll.h"

//----------------------------------------------------------------------------

class StringHash
{
public:
	// Construction
	StringHash();
	StringHash( const StringHash & copy );
	StringHash( const char * pString );
	StringHash( const CharString & string );

	// Accessors
	operator		dword() const;

	bool			operator==(const StringHash &cmp ) const;
	bool			operator!=(const StringHash &cmp) const;
	bool			operator>(const StringHash &cmp ) const;
	bool			operator>=(const StringHash &cmp ) const;
	bool			operator<(const StringHash &cmp ) const;
	bool			operator<=(const StringHash &cmp) const;

	// Mutators
	void				setHash( const char * pString, dword nSeed = 0 );
	DLL static dword	makeHash( const char * pString, dword nSeed = 0 );

private:
	// Data
	dword				m_Hash;
};

//-------------------------------------------------------------------------------

inline StringHash::StringHash() : m_Hash( 0 ) 
{}

inline StringHash::StringHash( const StringHash &copy ) : m_Hash( copy.m_Hash )
{}

inline StringHash::StringHash( const char * pString ) : m_Hash( 0 ) 
{
	setHash( pString );
}

inline StringHash::StringHash( const CharString & string ) : m_Hash( 0 ) 
{
	setHash( string );
}

//-------------------------------------------------------------------------------

inline StringHash::operator dword() const
{
	return( m_Hash );
}

inline bool StringHash::operator==(const StringHash &cmp ) const
{
	return( m_Hash == cmp.m_Hash );
}

inline bool StringHash::operator!=(const StringHash &cmp) const
{
	return( m_Hash != cmp.m_Hash );
}

inline bool StringHash::operator>(const StringHash &cmp ) const
{
	return( m_Hash > cmp.m_Hash );
}

inline bool StringHash::operator>=(const StringHash &cmp ) const
{
	return( m_Hash >= cmp.m_Hash );
}

inline bool StringHash::operator<(const StringHash &cmp ) const
{
	return( m_Hash < cmp.m_Hash );
}

inline bool StringHash::operator<=(const StringHash &cmp) const
{
	return( m_Hash <= cmp.m_Hash );
}

//-------------------------------------------------------------------------------

inline void StringHash::setHash( const char * pString, dword nSeed /*= 0*/ )
{
	m_Hash = makeHash( pString, nSeed );
}

//-------------------------------------------------------------------------------



#endif

//-------------------------------------------------------------------------------
// EOF
