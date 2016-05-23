/*
	Locale.cpp
	(c)2005 Palestar Inc, Richard Lyle
*/

#define SYSTEM_DLL
#include "Debug/Assert.h"
#include "Debug/Trace.h"
#include "Standard/StringHash64.h"
#include "Standard/Settings.h"
#include "Standard/StringHelpers.h"
#include "File/FileDisk.h"
#include "System/Locale.h"

#include <stdio.h>
#include <ctype.h>

#if defined(_WIN32)
#include <windows.h>
#endif

#pragma warning(disable:4996)	// warning C4996: 'swscanf': This function or variable may be unsafe. Consider using swscanf_s instead. To disable deprecation, use _CRT_SECURE_NO_WARNINGS. See online help for details.

//----------------------------------------------------------------------------

Locale::Locale() : m_LCID( getDefaultLCID() )
{
#if defined(_WIN32)
	char sModule[ MAX_PATH ];
	GetModuleFileNameA( GetModuleHandle(NULL), sModule, MAX_PATH );
	m_sLocaleFile = sModule;

	int n = m_sLocaleFile.reverseFind( '\\' );
	if ( n > 0 )
		m_sLocaleFile.left( n );
	m_sLocaleFile += "\\locale.txt";
#else
	m_sLocaleFile = "locale.txt";
#endif

	if (! load( m_sLocaleFile, true ) )
		TRACE( "Warning: Failed to load default locale file!" );

	// optionally, load the LCID from the config.ini
	Settings config( "Locale" );
	m_LCID = config.get( "LCID", m_LCID );
}

Locale::~Locale()
{}

//----------------------------------------------------------------------------

const wchar * Locale::string( dword LCID, const StringKey & nKey ) 
{
	StringTree::Iterator strings = m_Strings.find( LCID );
	if ( strings.valid() )
	{
		// found the language, look for the string now
		StringHash::Iterator string = (*strings).find( nKey );
		if ( string.valid() )
			return *string;
	}

	if ( LCID != 0 )
		return string( 0, nKey );		// look in the default hash
	else
		return L"";					// string not found
}


//----------------------------------------------------------------------------

void Locale::setLocaleFile( const char * pFile )
{
	m_sLocaleFile = pFile;
}

void Locale::setLCID( dword LCID )
{
	m_LCID = LCID;
}

StringKey Locale::localizeString( const wchar * pString )
{
	CharString sString( pString );
	StringHash64 hash( sString );
	StringKey nKey( hash );

	// add the string to the default language table, if not already in the hash
	if (! m_Strings[ 0 ].find( nKey ).valid() )
		m_Strings[ 0 ][ nKey ] = pString;

	// return the key
	return nKey;
}

void Locale::setString( dword LCID, const StringKey & nKey, const wchar * pString )
{
	m_Strings[ LCID ][ nKey ] = pString;
}

const int LINE_BUFFER_SIZE = 4 * 1024;

bool Locale::load( const char * pFile, bool bClearStrings /*= true*/ )
{
	if ( bClearStrings )
	{
		// release and initalize the tree
		m_Strings.release();
		m_Strings[ 0x0000 ];		// Language Neutral
	}

	wchar * pLocale = FileDisk::loadUnicodeFile( pFile );
	if ( pLocale == NULL )
		return false;

	// move the loaded text into a string object, then delete the memory
	String sLocale = pLocale;
	delete [] pLocale;

	dword LCID = 0;
	qword KEY = 0;

	String sLine;
	while( sLocale.tokenize( sLine, STR("\n\r\0") ) )
	{
		// find the line end
		wchar * pLine = sLine.buffer();
		wchar * pLineEnd = pLine + sLine.length();

		// skip any whitespaces
		while( pLine < pLineEnd && *pLine != 0 && isspace( *pLine ) )
			pLine++;
		if ( *pLine == 0 )
			continue;		// empty line, next line
		if ( *pLine == L';' )
			continue;		// comment, skip line
		
		wchar * pAssignment = strchr( pLine, L'=' );
		if ( pAssignment != NULL )
		{
			*pAssignment = 0;

			wchar * pKey = pLine;
			wchar * pValue = pAssignment + 1;
			if ( stricmp( pKey, STR("LCID") ) == 0 )
				swscanf( pValue, STR("%u"), &LCID );
			else if ( swscanf( pKey, STR("%I64X"), &KEY) == 1 )
				m_Strings[ LCID ][ KEY ] = String( pValue ).unslash();
		}
	}

	return true;
}

bool Locale::save( const char * pFile )
{
	if ( pFile == NULL )
		return false;

	// load the locale file from disk again, just in case it got changed from the last time we loaded it..
	load( pFile, false );
	// build a string containing the locale
	String sLocale;
	// enumerate all strings and store them into the locale file
	StringTree::Iterator strings = m_Strings.head();
	while( strings.valid() )
	{
		// get the locale ID
		dword LCID = strings.key();

		// get the location name
		sLocale += String().format( STR("; Begin Language %s\r\n"), language( LCID ) );
		sLocale += String().format( STR("LCID=%u\r\n"), LCID );

		StringHash::Iterator it = (*strings).head();
		while( it.valid() )
		{
			// write out the LCID:KEY:STRING
			sLocale += String().format( STR("%16.16I64X=%s\r\n"), (qword)it.key(), (const wchar *)String( *it ).slash() );
			// next item
			it.next();
		}

		sLocale += String().format( STR("; End Language %s\r\n\r\n"), language( LCID ) );
		strings.next();
	}

	return FileDisk::saveUnicodeFile( pFile, sLocale );
}

//----------------------------------------------------------------------------

dword Locale::getDefaultLCID()
{
#if defined(_WIN32)
	return GetUserDefaultLCID() & 0xffff;
#else
	return 0;
#endif
}

Locale & Locale::locale()
{
	static Locale LOCALE;
	return LOCALE;
}

const wchar * Locale::language( dword LCID )
{
#if defined(_WIN32)
	static wchar sLanguage[ 64 ];

	char s[ 64 ];
	if (! GetLocaleInfoA( LCID, LOCALE_SLANGUAGE, s, sizeof(s) ) )
		return NULL;
	MultiByteToWideChar( CP_ACP, 0, s, strlen(s), sLanguage, 64 );
	return sLanguage;
#else
	return L"English";
#endif
}


//----------------------------------------------------------------------------
//EOF
