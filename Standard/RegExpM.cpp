/*
	RegExpM.cpp
	(c)2005 Palestar Inc, Richard Lyle
*/

#include "Standard/RegExpM.h"

#ifndef REGEXP_UNICODE
#define REGEXP_UNICODE
#endif

#include "ThirdParty/re2/regexp.h"


//----------------------------------------------------------------------------

RegExpM::RegExpM() : m_pCompiled( NULL ), m_nMatches( 0 ), m_pMatches( NULL )
{}

RegExpM::RegExpM( const wchar * pExp ) : m_pCompiled( NULL ), m_nMatches( 0 ), m_pMatches( NULL )
{
	regComp( pExp );
}

RegExpM::RegExpM( const char * pExp ) : m_pCompiled( NULL ), m_nMatches( 0 ), m_pMatches( NULL )
{
	regComp( String(pExp) );
}

RegExpM::~RegExpM()
{
	release();
}

//----------------------------------------------------------------------------

RegExpM * RegExpM::regComp(const char * pExp )
{
	return regComp( WideString( pExp ) );
}

// set the regular expression, used by RegFind()
RegExpM * RegExpM::regComp(const wchar * pExp )
{
	release();

	if ( pExp == NULL )
		return NULL;		// invalid expression string
	if ( re_comp_w( &m_pCompiled, pExp ) < 0 )
		return NULL;		// error in the expression

	// allocate the matches array
	m_nMatches = re_nsubexp( m_pCompiled );
	m_pMatches = new regmatch[ m_nMatches ];

	return this;
}

int RegExpM::regFind( const char * pStr )
{
	return regFind( WideString( pStr ) );
}

// RegFind	- match a regexp against a string
// Returns	- Returns position of regexp or -1 if regular expression not found
// Note		- The regular expression should have been previously compiled using RegComp

int RegExpM::regFind( const wchar * pStr )
{
	if ( !pStr || *pStr == 0 )
		return -1;
	if ( m_pCompiled == NULL )
		return -1;		// regComp() never called

	m_sFindText = pStr;
	int nFound = re_exec_w( m_pCompiled, m_sFindText, m_nMatches, m_pMatches );
	if ( nFound < 1 )
		return -1;		// no matches found!
	
	m_sFoundText = pStr + m_pMatches[0].begin;
	m_sFoundText.left( m_pMatches[0].end - m_pMatches[0].begin );

	return m_pMatches[0].begin;
}

// returns the length in characters of the text found by the last call to regFind()
int RegExpM::getFindLen()
{
	return m_sFoundText.length();
}

String RegExpM::getReplaceString( const wchar * sReplaceExp )
{
	wchar c;

	if( sReplaceExp == NULL || m_sFindText.length() == 0 )
		return "";

	String sOut;

	// Now we can create the string
	const wchar * pIn = sReplaceExp;
	while ((c = *pIn++) != ('\0')) 
	{
		int no = -1;
		if (c == ('&'))
			no = 0;
		else if (c == ('\\') && *pIn >= '0' && *pIn <= '9' )
			no = *pIn++ - ('0');

		if (no < 0) 
		{	
			// Ordinary character. 
			if (c == ('\\') && (*pIn == ('\\') || *pIn == ('&')))
				c = *pIn++;

			sOut += c;
		} 
		else if ( no < m_nMatches && m_pMatches[no].end > m_pMatches[no].begin ) 
		{
			// Get tagged expression
			int len = m_pMatches[no].end - m_pMatches[no].begin;
			int tagpos = m_pMatches[no].begin - m_pMatches[0].begin;

			String sText = m_sFoundText.buffer() + tagpos;
			sText.left( len );

			sOut += sText;
		}
	}

	return sOut;
}

void RegExpM::release()
{
	if ( m_pCompiled != NULL )
	{
		re_free( m_pCompiled );
		m_pCompiled = NULL;
	}
	if ( m_pMatches != NULL )
	{
		delete [] m_pMatches;
		m_pMatches = NULL;
	}
	m_nMatches = 0;
}

// Here's a function that will do global search and replace using regular expressions.
int RegExpM::regSearchReplace( String & s, const wchar * pSearchExp, const wchar * pReplaceExp )
{
	int nPos = 0;
	int nReplaced = 0;
	int nCurrPos = 0;

	RegExpM r( pSearchExp );

	// Find a RegExp match starting from the current position
	while( (nPos = r.regFind( s.buffer() + nCurrPos )) != -1 )
	{
		String sReplaceStr = r.getReplaceString( pReplaceExp );
		int nFindLen = r.getFindLen();
		
		// As the match was searched relative to nCurrPos it needs to be added
		// to nPos to get the absolute position for replacing the match.
		s.replace( nCurrPos + nPos, nFindLen, sReplaceStr );

		// now the current position is advanced by the position of the match
		// + the length of the string the match got replaced with.
		// So the search continues at the end of the replaceString, thus preventing
		// infinite loops
		nCurrPos += nPos + sReplaceStr.length();

		// increment the replaced number
		nReplaced++;
	}
	
	return nReplaced;
}

int RegExpM::regSearchReplace( CharString & pSearch, const char * pSearchExp, const char * pReplaceExp )
{
	// conver the char strings into wide strings
	WideString sResult( pSearch.buffer() );
	int nReplaced = regSearchReplace( sResult, WideString( pSearchExp ), WideString( pReplaceExp ) );
	// return the modified string
	pSearch = sResult.buffer();

	return nReplaced;
}

//----------------------------------------------------------------------------
//EOF
