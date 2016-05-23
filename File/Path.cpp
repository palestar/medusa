/*
	Path.cpp
	(c)2005 Palestar, Richard Lyle
*/

#include "File/Path.h"

#if PATH_SEPERATORC == '/'
#define REPLACE_SEPERATOR		"\\"
#define REAPLCE_SEPERATORC		'\\'
#else
#define REPLACE_SEPERATOR		"/"
#define REAPLCE_SEPERATORC		'/'
#endif

//----------------------------------------------------------------------------

Path::Path()
{}

Path::Path( const CharString &path )
{
	setPath(path);
}

Path::Path( const char * pPath )
{
	setPath(pPath);
}

//----------------------------------------------------------------------------

CharString Path::path()	const			// c:\example\file.ext
{
	return( m_Directory + m_Name + m_Extension );
}

CharString Path::directory() const		// c:\example
{
	return( m_Directory );
}

CharString Path::file()	const			// file.ext
{
	return( m_Name + m_Extension );
}

CharString Path::name()	const			// file
{
	return( m_Name );
}

CharString Path::extension() const		// .ext
{
	return( m_Extension );
}

//----------------------------------------------------------------------------

void Path::setPath( const char * pPath )
{
	CharString sPath( pPath );
	sPath.replace( REPLACE_SEPERATOR, PATH_SEPERATOR );

	int nSlash = -1;
	int nExt = -1;

	int nLen = sPath.length() - 1;
	while( nLen >= 0 && nSlash < 0 )
	{
		char c = sPath[ nLen ];
		if ( nSlash < 0 && c == PATH_SEPERATORC )
			nSlash = nLen;
		else if ( nExt < 0 && nSlash < 0 && c == '.' )
			nExt = nLen;

		nLen--;
	}

	if ( nSlash >= 0 )
	{
		m_Directory = sPath;
		m_Directory.left( nSlash + 1 );
	}
	else
	{
		m_Directory = "." PATH_SEPERATOR;
	}


	if ( nExt >= 0 )
	{
		int nNameLen = (sPath.cstr() + nExt) - (sPath.cstr() + nSlash + 1);
		m_Name = sPath.cstr() + nSlash + 1;
		m_Name.left( nNameLen );
		m_Extension = sPath.cstr() + nExt;
	}
	else
	{
		m_Name = sPath.cstr() + nSlash + 1;
		m_Extension = "";
	}
}

void Path::setDirectory( const char * pDir )
{
	m_Directory = pDir;
	m_Directory.replace( REPLACE_SEPERATOR, PATH_SEPERATOR );
	if ( m_Directory[ m_Directory.length() - 1 ] != PATH_SEPERATORC )
		m_Directory += PATH_SEPERATORC;
}

void Path::setFile( const char * pFile )
{
	m_Name = pFile;
	m_Name.left(m_Name.reverseFind('.'));
	m_Extension = pFile;
	m_Extension.right(m_Extension.length() - m_Name.reverseFind('.'));
}

void Path::setName( const char * pName )
{
	m_Name = pName;
}

void Path::setExtension( const char * pExt )
{
	m_Extension = pExt;
}

//----------------------------------------------------------------------------
// EOF
