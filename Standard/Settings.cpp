/*
	Settings.cpp
	(c)2005 Palestar Inc, Richard Lyle
*/

#include "Standard/Settings.h"
#include <stdio.h>

#if defined(_WIN32)
#include <io.h>
#else
#include <unistd.h>
#include <sys/types.h>
#endif

//---------------------------------------------------------------------------------------------------

static void CleanLine( char * pLine )
{
	for(int i=0;pLine[i];++i)
	{
		if ( pLine[i] == '\r' || pLine[i] == '\n' )
			pLine[i] = 0;
	}
}

//----------------------------------------------------------------------------

Settings::Settings() : m_sFile( "." PATH_SEPERATOR "config.ini" ), m_sSection( "Medusa" )
{}

Settings::Settings( const char * pSection, const char * pFile /*= NULL*/ ) 
	: m_sSection( pSection ), m_sFile( pFile != NULL ? pFile : ("." PATH_SEPERATOR "config.ini") )
{}

//----------------------------------------------------------------------------

void Settings::setFile( const char * pFile )
{
	m_sFile = pFile;
}

void Settings::setSection( const char * pSection )
{
	m_sSection = pSection;
}

//----------------------------------------------------------------------------

dword Settings::get( const char * pKey, dword nDefault )
{
	return strtoul( get( pKey, CharString().format( "%u", nDefault ) ), NULL, 10 );
}

CharString Settings::get( const char * pKey, const char * pDefault )
{
	CharString sKey( pKey );
	sKey += "=";

	FILE * pFile = fopen( m_sFile, "r+b" );
	if ( pFile != NULL )
	{
		bool bInSection = false;

		char sLine[ 1024 ];
		while( fgets( sLine, sizeof(sLine), pFile ) )
		{
			CleanLine( sLine );

			if ( sLine[0] == '#' || sLine[0] == 0 )
				continue;		// skip comments or empty lines..

			if ( sLine[0] == '[' )
			{
				const char * pEnd = strchr( sLine, ']' );
				if ( pEnd != NULL )
					bInSection = strnicmp<char>( m_sSection, sLine + 1, (pEnd - 1) - (sLine + 1) ) == 0;
				continue;
			}

			if (! bInSection )
				continue;
			const char * pEquals = strchr( sLine, '=' );
			if ( pEquals == NULL )
				continue;

			if ( strnicmp( sKey.cstr(), sLine, sKey.length() ) == 0 )
			{
				fclose( pFile );
				return CharString ( pEquals + 1 );
			}
		}

		fclose( pFile );
	}

	return CharString( pDefault );
}

bool Settings::put( const char * pKey, dword nValue )
{
	return put( pKey, CharString().format("%u", nValue ) );
}

bool Settings::put( const char * pKey, const char * pValue )
{
	CharString sKey( pKey );
	sKey += "=";

	FILE * pFile = fopen( m_sFile, "r+b" );
	if ( pFile == NULL )		
		pFile = fopen( m_sFile, "w+b" );		// if we failed to open, then it's likely because the file doesn't exist, make it..
	if ( pFile != NULL )
	{
		bool bInSection = false;

		dword nCurrentPos = 0;

		char sLine[ 1024 ];
		while( fgets( sLine, sizeof(sLine), pFile ) )
		{
			// remove any newline & return characters from the line..
			CleanLine( sLine );

			dword nLastPos = nCurrentPos;
			nCurrentPos = ftell( pFile );

			if ( sLine[0] == '#' || sLine[0] == 0 )
				continue;		// skip comments or empty lines..

			if ( sLine[0] == '[' )
			{
				bool bWasInSection = bInSection;
				const char * pEnd = strchr( sLine, ']' );
				if ( pEnd != NULL )
					bInSection = strnicmp<char>( m_sSection, sLine + 1, (pEnd - 1) - (sLine + 1) ) == 0;

				// if we are leaving our section, then write our key/value pair then..
				if ( bWasInSection && !bInSection )
				{
					fseek( pFile, 0, SEEK_END );
					dword nFileSize = ftell( pFile );
					fseek( pFile, nLastPos, SEEK_SET );

					dword nMoveSize = nFileSize - nLastPos;
					byte * pMoveBuffer = new byte[ nMoveSize ];
					fread( pMoveBuffer, 1, nMoveSize, pFile );

					fseek( pFile, nLastPos, SEEK_SET );
					fprintf( pFile, "%s=%s\r\n", pKey, pValue );
					fwrite( pMoveBuffer, nMoveSize, 1, pFile );
#if defined(_WIN32)
					_chsize( _fileno( pFile ), ftell( pFile ) );
#else
					ftruncate( fileno( pFile ), ftell( pFile ) );
#endif
					fclose( pFile );

					delete [] pMoveBuffer;

					return true;
				}

				continue;
			}

			if (! bInSection )
				continue;
			const char * pEquals = strchr( sLine, '=' );
			if ( pEquals == NULL )
				continue;

			// look for the old key, if found replace it with our new value..
			if ( strnicmp( sKey.cstr(), sLine, sKey.length() ) == 0 )
			{
				fseek( pFile, 0, SEEK_END );
				dword nFileSize = ftell( pFile );
				fseek( pFile, nCurrentPos, SEEK_SET );

				dword nMoveSize = nFileSize - nCurrentPos;
				byte * pMoveBuffer = new byte[ nMoveSize ];
				fread( pMoveBuffer, 1, nMoveSize, pFile );

				fseek( pFile, nLastPos, SEEK_SET );		// overwrite the old key=value
				fprintf( pFile, "%s=%s\r\n", pKey, pValue );
				fwrite( pMoveBuffer, nMoveSize, 1, pFile );
#if defined(_WIN32)
				_chsize( _fileno( pFile ), ftell( pFile ) );
#else
				ftruncate( fileno( pFile ), ftell( pFile ) );
#endif
				fclose( pFile );

				delete [] pMoveBuffer;

				return true;
			}
		}

		// no key/value or section was found, add a new section and our key/value pair at the end of the file then..
		if (! bInSection )
			fprintf( pFile, "\r\n[%s]\r\n", m_sSection.cstr() );

		fprintf( pFile, "%s=%s\r\n", pKey, pValue );
		fclose( pFile );

		return true;
	}

	return false;
}

//----------------------------------------------------------------------------
//EOF
