/*
	LocalizedString.h

	A LocalizedString object is used to hold text that needs to be localized to other languages. 

	Each object has a StringKey (UniqueNumber) assigned to it, which is used to hash to the correct
	string for the installed language package. If the key doesn't exist in the currently loaded locale file, 
	then it is added automatically so the text can be translated for that package.

	(c)2005 Palestar Inc, Richard Lyle
*/

#ifndef LOCALIZEDSTRING_H
#define LOCALIZEDSTRING_H

#include "File/Stream.h"
#include "System/Locale.h"
#include "MedusaDll.h"

#define LSTR( q )		LocalizedString( q ).string()

//----------------------------------------------------------------------------

class DLL LocalizedString 
{
public:
	// Construction
	LocalizedString();
	LocalizedString( const LocalizedString & copy );				// copy constructor
	LocalizedString( const wchar * pString );						// calls setString()
	LocalizedString( const char * pString );						// calls setString()
	LocalizedString( dword nStringId );						

	// Accessors
	const StringKey &		stringKey() const;						// get the StringKey
	const wchar *			string() const;							// get the translated version of this string
	operator				WideString() const;

	// Mutators
	void					setString( const wchar * pString );		// generate a new key, writes to the locale file if key not found
	void					setString( const char * pString );	

	// Static
	static Locale &			locale();								// get the locale object

protected:
	// Data
	StringKey				m_nKey;
}; 

IMPLEMENT_RAW_STREAMING( LocalizedString );

//----------------------------------------------------------------------------

inline LocalizedString::LocalizedString() : m_nKey( (qword)0 )
{}

inline LocalizedString::LocalizedString( const LocalizedString & copy ) : m_nKey( copy.m_nKey )
{}

inline LocalizedString::LocalizedString( const wchar * pString ) : m_nKey( (qword)0 )
{
	setString( pString );
}

inline LocalizedString::LocalizedString( const char * pString ) : m_nKey( (qword)0 )
{
	setString( pString );
}

inline LocalizedString::LocalizedString( dword nStringId ) : m_nKey( (qword)0 )
{
	setString( StringBase::loadString( nStringId ) );
}

//----------------------------------------------------------------------------

inline const StringKey & LocalizedString::stringKey() const
{
	return m_nKey;
}

inline const wchar * LocalizedString::string() const
{
	return locale().string( m_nKey );
}

inline LocalizedString::operator WideString() const
{
	return WideString( locale().string( m_nKey ) );
}

//----------------------------------------------------------------------------

inline void LocalizedString::setString( const wchar * pString )
{
	m_nKey = locale().localizeString( pString );
}

inline void LocalizedString::setString( const char * pString )
{
	m_nKey = locale().localizeString( String( pString ) );
}

//----------------------------------------------------------------------------



#endif

//----------------------------------------------------------------------------
//EOF
