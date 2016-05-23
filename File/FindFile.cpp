/*
	FindFile.cpp

	This object is used to find files on the disk using a mask
	(c)2005 Palestar, Richard Lyle
*/

#include "File/Path.h"
#include "File/FindFile.h"
#include "FileDisk.h"

#include "Debug/Assert.h"

#if defined(WIN32)

#include <windows.h>
#include <sys/types.h>
#include <sys/stat.h>			// _stat
#include <sys/utime.h>			// _utime
#include <io.h>					// _access
#include <direct.h>				// _mkdir, _rmdir
#include <time.h>				// time
#include <fcntl.h>
#include <errno.h>
#include <share.h>			

#else

#include <sys/types.h>
#include <sys/stat.h>			// _stat
#include <utime.h>				// utime
#include <time.h>				// time
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <dirent.h>				// opedir
#include <string.h>				// memset
#include <fnmatch.h>			// fnmatch
#include <limits.h>				// MAX_PATH

#include <limits.h>
#define MAX_PATH		PATH_MAX

#endif

//---------------------------------------------------------------------------------------------------

static int CharStringSort( CharString s1, CharString s2 )
{
	return stricmp<char>( s1, s2 );
}

//----------------------------------------------------------------------------

FindFile::FindFile()
{}

FindFile::FindFile( const char * fileMask, bool bFindHidden /*= false*/, bool bFullPath /*= false*/ )
{
	// find the files
	findFiles( fileMask, bFindHidden, bFullPath );
}

FindFile::~FindFile()
{}

//----------------------------------------------------------------------------

int FindFile::fileCount() const
{
	return( m_Files.size() );
}

const char * FindFile::file( int n ) const
{
	return( m_Files[ n ] );
}

int FindFile::directoryCount() const
{
	return( m_Directories.size() );
}

const char * FindFile::directory( int n ) const
{
	return( m_Directories[ n ] );
}

//----------------------------------------------------------------------------

void FindFile::findFiles( const char * pMask, bool bFindHidden /*= false*/, bool bFullPath /*= false*/ )
{
	// release any previous searches
	release();

	// get the root directory
	CharString sRoot = Path( pMask ).directory();
	// begin a new file search
	void * pFind = findOpen( pMask );
	while( pFind != NULL )
	{
		if (bFindHidden || !findIsHidden( pFind ) )
		{
			char sFileName[ 512 ];
			findName( pFind, sFileName, 512 );
			if ( strcmp( sFileName, "." ) != 0 && 
				strcmp( sFileName, ".." ) != 0 )
			{
				CharString sName = sFileName;
				if ( bFullPath )
					sName = sRoot + sName;

				if ( findIsDirectory( pFind ) )
					m_Directories.push( sName );
				else
					m_Files.push( sName );
			}
		}
		pFind = findNext( pFind );
	}
	findClose( pFind );
}

void FindFile::sortFiles()
{
	m_Files.qsort( CharStringSort );
}

void FindFile::sortDirectories()
{
	m_Directories.qsort( CharStringSort );
}

void FindFile::release()
{
	m_Files.release();
	m_Directories.release();
}

//---------------------------------------------------------------------------------------------------

struct CORE_FIND_DATA
{
#if defined(_WIN32) || defined(_XBOX)
	HANDLE				fh;
	WIN32_FIND_DATA		fd;
#else
	DIR *				pd;						// directory handle
	struct dirent 		pi;						// current directory entry
	char				spath[ MAX_PATH ];		// directory
	char				smask[ MAX_PATH ];		// filename filter
	char				sfile[ MAX_PATH ];		// full file path
#endif
};

void * FindFile::findOpen( const char * pMask )
{
	if ( pMask == NULL )
		return NULL;

	CORE_FIND_DATA * pFD = new CORE_FIND_DATA;
	if ( pFD != NULL )
	{
		memset( pFD, 0, sizeof(CORE_FIND_DATA) );

#if defined(_WIN32) || defined(_XBOX)
		pFD->fh = FindFirstFileA( pMask, &pFD->fd );
		if ( pFD->fh == INVALID_HANDLE_VALUE )
		{
			delete pFD;
			return NULL;
		}
#else
		char * pLastSlash = strrchr( (char *)pMask, '/' );
		if ( pLastSlash != NULL )
		{
			strncpy( pFD->smask, pLastSlash + 1, MAX_PATH );
			strncpy( pFD->spath, pMask, MAX_PATH );
			pFD->spath[ pLastSlash - pMask ] = 0;
		}
		else
		{
			strcpy( pFD->smask, pFD->spath );
			strcpy( pFD->spath, "." );
		}

		if ( (pFD->pd = opendir( pFD->spath )) == NULL )
		{
			delete pFD;
			return NULL;
		}

		return findNext( pFD );
#endif
	}

	return pFD;
}

void FindFile::findName( void * pFind, char * pName, int nMaxName )
{
#if defined(_WIN32) || defined(_XBOX)
	strncpy( pName, ((CORE_FIND_DATA *)pFind)->fd.cFileName,nMaxName );
#else
	strncpy( pName, ((CORE_FIND_DATA *)pFind)->pi.d_name, nMaxName );
#endif
}

dword FindFile::findTime( void * pFind )
{
#if defined(_WIN32) || defined(_XBOX)
	CORE_FIND_DATA * pFD = (CORE_FIND_DATA *)pFind;

	// convert the filetime to a time_t
	qword time = (((qword)pFD->fd.ftLastWriteTime.dwHighDateTime) << 32) | pFD->fd.ftLastWriteTime.dwLowDateTime;
	time -= 116444736000000000;		// midnight (00:00:00), January 1, 1970
	time /= 10000000;				// convert from nano-seconds to seconds

	return (dword)time;
#else
	return FileDisk::fileDate( ((CORE_FIND_DATA *)pFind)->sfile );
#endif
}

bool FindFile::findIsLocked( void * pFind )
{
#if defined(_WIN32) || defined(_XBOX)
	return (((CORE_FIND_DATA *)pFind)->fd.dwFileAttributes & FILE_ATTRIBUTE_READONLY) != 0;
#else
	return FileDisk::fileLocked( ((CORE_FIND_DATA *)pFind)->sfile );
#endif
}

bool FindFile::findIsHidden( void * pFind )
{
#if defined(_WIN32) || defined(_XBOX)
	return (((CORE_FIND_DATA *)pFind)->fd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) != 0;
#else
	return false;		// not supported in unix!
#endif
}

bool FindFile::findIsDirectory( void * pFind )
{
#if defined(_WIN32) || defined(_XBOX)
	return (((CORE_FIND_DATA *)pFind)->fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
#else
	return ((CORE_FIND_DATA *)pFind)->pi.d_type == DT_DIR;
#endif
}

void * FindFile::findNext( void * pFind )
{
#if defined(_WIN32) || defined(_XBOX)
	if ( pFind == NULL )
		return NULL;

	CORE_FIND_DATA * pFD = (CORE_FIND_DATA *)pFind;
	if ( FindNextFile(pFD->fh, &pFD->fd) == 0 )
	{
		// no more files, close out
		findClose( pFind );
		return NULL;
	}

	return pFD;
#else
	if ( pFind == NULL )
		return NULL;
	
	CORE_FIND_DATA * pFD = (CORE_FIND_DATA *)pFind;

	struct dirent * pi = NULL;
	while( pi = readdir( pFD->pd ) )
	{
		// some file systems (e.g. vboxsf) don't support the readdir() functionality fully, so if we get a DT_UNKNOWN
		// type try to determine if it's a directory or not by attempting to open it as a directory.
		if ( pi->d_type == DT_UNKNOWN )
		{
			// file typs is unknow, determine if this is a directory or file by trying to opendir() 
			// using this file as it's base name..
			char test[ MAX_PATH ];
			strcpy( test, pFD->spath );
			strcat( test, "/" );
			strcat( test, pi->d_name );
			strcat( test, "/." );

			DIR * pTestDir = opendir( test );
			if ( pTestDir != NULL )
			{
				// appears to be a directory..
				pi->d_type = DT_DIR;
				closedir( pTestDir );
			}
			else
			{
				// not a directory, so flag as a file..
				pi->d_type = DT_REG;
			}
		}

		// match against all directories, and check all files against our mask...
		if ( pi->d_type == DT_DIR || 
			(pi->d_type == DT_REG  && fnmatch( pFD->smask, pi->d_name, 0 ) == 0) )
		{
			strcpy( pFD->pi.d_name, pi->d_name );
			pFD->pi.d_type = pi->d_type;

			strcpy( pFD->sfile, pFD->spath );
			strcat( pFD->sfile, "/" );
			strcat( pFD->sfile, pFD->pi.d_name );

			return pFD;
		}
	}

	// no more files in the directory
	findClose( pFD );
	return NULL;
#endif
}

void FindFile::findClose( void * pFind )
{
#if defined(_WIN32) || defined(_XBOX)
	if ( pFind != NULL )
	{
		CORE_FIND_DATA * pFD = (CORE_FIND_DATA *)pFind;
		// close the handle
		FindClose( pFD->fh );
		// release the allocated structure
		delete pFD;
	}
#else
	if ( pFind != NULL )
	{
		CORE_FIND_DATA * pFD = (CORE_FIND_DATA *)pFind;

		closedir( pFD->pd );
		delete pFD;
	}
#endif
}

//----------------------------------------------------------------------------
// EOF
