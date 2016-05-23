/*
	StringHash64.h
	(c)2005 Palestar, Richard Lyle
*/

#ifndef STRING_HASH_64_H
#define STRING_HASH_64_H

#include "Standard/Types.h"
#include "MedusaDll.h"

//----------------------------------------------------------------------------

class StringHash64
{
public:
	// Construction
	StringHash64();
	StringHash64( const StringHash64 & copy );
	StringHash64( const char * pString, qword nSeed = 0 );

	// Accessors
	operator		qword() const;
	qword			hash() const;

	bool			operator==(const StringHash64 &cmp ) const;
	bool			operator!=(const StringHash64 &cmp) const;
	bool			operator>(const StringHash64 &cmp ) const;
	bool			operator>=(const StringHash64 &cmp ) const;
	bool			operator<(const StringHash64 &cmp ) const;
	bool			operator<=(const StringHash64 &cmp) const;

	// Mutators
	void			setHash( const char * pString, qword nSeed = 0 );
	void			setHash( const wchar * pString, qword nSeed = 0 );
	// Static
	DLL static qword	
					makeHash( const char * pString, qword nSeed = 0 );

private:
	// Data
	qword			m_Hash;
};

//-------------------------------------------------------------------------------

inline StringHash64::StringHash64() : m_Hash(0) 
{}

inline StringHash64::StringHash64( const StringHash64 &copy )
{
	m_Hash = copy.m_Hash;
}

inline StringHash64::StringHash64( const char * pString, qword nSeed /*= 0*/ )
{
	setHash( pString, nSeed );
}

//-------------------------------------------------------------------------------

inline StringHash64::operator qword() const
{
	return( m_Hash );
}

inline qword StringHash64::	hash() const
{
	return m_Hash;
}

inline bool StringHash64::operator==(const StringHash64 &cmp ) const
{
	return( m_Hash == cmp.m_Hash );
}

inline bool StringHash64::operator!=(const StringHash64 &cmp) const
{
	return( m_Hash != cmp.m_Hash );
}

inline bool StringHash64::operator>(const StringHash64 &cmp ) const
{
	return( m_Hash > cmp.m_Hash );
}

inline bool StringHash64::operator>=(const StringHash64 &cmp ) const
{
	return( m_Hash >= cmp.m_Hash );
}

inline bool StringHash64::operator<(const StringHash64 &cmp ) const
{
	return( m_Hash < cmp.m_Hash );
}

inline bool StringHash64::operator<=(const StringHash64 &cmp) const
{
	return( m_Hash <= cmp.m_Hash );
}

//-------------------------------------------------------------------------------

inline void StringHash64::setHash( const char * pString, qword nSeed /*= 0*/ )
{
	m_Hash = makeHash( pString, nSeed );
}

//-------------------------------------------------------------------------------



#endif

//-------------------------------------------------------------------------------
// EOF
