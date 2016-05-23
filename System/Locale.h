/*
	Locale.h
	(c)2005 Palestar Inc, Richard Lyle
*/

#ifndef LOCALE_H
#define LOCALE_H

#include "Standard/UniqueNumber.h"
#include "Standard/String.h"
#include "MedusaDll.h"

#pragma warning(disable:4251 )		// class 'Tree<unsigned long,class Hash<class UniqueNumberBase,class String,32> >' needs to have dll-interface to be used by clients of class '

//----------------------------------------------------------------------------

// StringKey is the unique ID used for all LocalizedStrings
typedef UniqueNumber			StringKey;

//----------------------------------------------------------------------------

class DLL Locale 
{
public:
	// Constructor
	Locale();
	virtual ~Locale();

	// Accessors
	const char *			localeFile() const;			// get the locale filename
	dword					LCID() const;				// get the locale ID

	// find string with current LCID by key
	const wchar *			string( const StringKey & nKey );
	// find string with the specified LCID by key
	const wchar *			string( dword LCID, const StringKey & nKey );

	// Mutators
	void					setLocaleFile( const char * pFile );
	void					setLCID( dword LCID );		// set the current locale (language)

	// add the provided string to the string tree, returns the key for the provided string which 
	// is a hash of the string.
	StringKey				localizeString( 
								const wchar * pString );

	// set the string for the specified LCID and key to the provided string
	void					setString( dword LCID, const StringKey & nKey,
								const wchar * pString );

	bool					load( const char * pFile, 
								bool bClearStrings = true );
	bool					save( const char * pFile );

	// Static
	static const wchar *	language( dword LCID );		// get the language name
	static dword			getDefaultLCID();			// get the default LCID
	// The static instance
	static Locale &			locale();					// get the singleton object

protected:
	// Types
	typedef Hash< StringKey, String >	StringHash;
	typedef Tree< dword, StringHash >	StringTree;

	// Data
	CharString				m_sLocaleFile;				// file containing all strings
	dword					m_LCID;						// locale ID
	StringTree				m_Strings;					// key to string hash
};

//----------------------------------------------------------------------------

inline const char * Locale::localeFile() const
{
	return m_sLocaleFile;
}

inline dword Locale::LCID() const
{
	return m_LCID;
}

inline const wchar * Locale::string( const StringKey & nKey )
{
	return string( m_LCID, nKey );
}

//----------------------------------------------------------------------------



#endif

//----------------------------------------------------------------------------
//EOF
