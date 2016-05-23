/*
	FileDisk.cpp
	(c)2005 Palestar, Richard Lyle
*/

#include "Debug/Log.h"
#include "FileDisk.h"
#include "FindFile.h"
#include <ctype.h>

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

struct CORE_FILE
{
	int		fh;
};

//----------------------------------------------------------------------------------------

Array< CharString >		FileDisk::s_SearchPaths;

//----------------------------------------------------------------------------------------

FileDisk::FileDisk() : m_pHandle( 0 ), m_Access( READ )
{}

FileDisk::FileDisk( const FileDisk & copy ) 
	: m_pHandle( 0 ), m_Access( copy.m_Access ), m_sFileName( copy.m_sFileName ), File()
{
	if ( copy.m_pHandle != NULL )
	{
#if defined(_WIN32) || defined(_XBOX)
		HANDLE hProcess = GetCurrentProcess();
		DuplicateHandle( hProcess, copy.m_pHandle, hProcess, &m_pHandle, 0, true, DUPLICATE_SAME_ACCESS);
#else
		int fh = dup( ((CORE_FILE *)copy.m_pHandle)->fh );
		if ( fh >= 0 )
		{
			CORE_FILE * pFH2 = new CORE_FILE;
			pFH2->fh = fh;
			m_pHandle = pFH2;
		}
#endif
	}
}

FileDisk::FileDisk(const char * pFileName, AccessType access) 
	: m_pHandle( 0 ), m_Access( READ ), File()
{
    open( pFileName, access);
}

FileDisk::~FileDisk()
{
    close();
}

//----------------------------------------------------------------------------------------

File * FileDisk::clone() const
{
	return new FileDisk( *this );
}

bool FileDisk::isReadable() const
{
    return( m_Access == READ || m_Access == READ_WRITE );
}

bool FileDisk::isWritable() const
{
    return( m_Access == WRITE || m_Access == READ_WRITE );
}

File::Size FileDisk::size() const
{
	qword nSize = 0;
	if ( m_pHandle == NULL )
		return 0xFFFFFFFF;

#if defined(_WIN32) || defined(_XBOX)
	dword nHigh = 0;
	dword nLow = GetFileSize( m_pHandle, &nHigh );
	if ( nLow == 0xFFFFFFFF && GetLastError() != NO_ERROR )
		return 0xFFFFFFFF;

	nSize = (((qword)nHigh) << 32) | nLow;
#else
	dword nSaved = lseek( ((CORE_FILE *)m_pHandle)->fh, 0, SEEK_CUR );
	nSize = lseek( ((CORE_FILE *)m_pHandle)->fh, 0, SEEK_END );
	lseek( ((CORE_FILE *)m_pHandle)->fh, nSaved, SEEK_SET );
#endif

	return (File::Size)nSize;
}

File::Position FileDisk::position() const
{
	qword nPos = 0;
	if ( m_pHandle == NULL )
		return 0xFFFFFFFF;

#if defined(_WIN32) || defined(_XBOX)
	dword nHigh = 0;
	dword nLow = SetFilePointer(m_pHandle, 0, (long *)&nHigh, FILE_CURRENT);
	if ( nLow == 0xFFFFFFFF && GetLastError() != NO_ERROR )
		return 0xFFFFFFFF;
	nPos = (((qword)nHigh) << 32) | nLow;
#else
	nPos = lseek( ((CORE_FILE *)m_pHandle)->fh, 0, SEEK_CUR );
#endif
	return (Position)nPos;
}

//----------------------------------------------------------------------------------------

File::Size FileDisk::read( void * pBuffer, Size nBytes )
{
	if ( m_pHandle == NULL || pBuffer == NULL )
		return 0xFFFFFFFF;
	if ( nBytes == 0 )
		return 0;

#if defined(_WIN32) || defined(_XBOX)
	unsigned int nRead = 0;
    if (!ReadFile(m_pHandle, pBuffer, nBytes, (unsigned long *)&nRead, NULL))
		return 0;
	return nRead;
#else
	return ::read( ((CORE_FILE *)m_pHandle)->fh, pBuffer, nBytes );
#endif
}

File::Size FileDisk::write( const void * pBuffer, Size nBytes )
{
 	if ( m_pHandle == NULL || pBuffer == NULL )
		return 0xFFFFFFFF;
	if ( nBytes == 0 )
		return 0;			// nothing to write, success!

#if defined(_WIN32) || defined(_XBOX)
    unsigned int nWritten = 0;
    WriteFile(m_pHandle, pBuffer, nBytes, (unsigned long *)&nWritten, NULL);
	return nWritten;
#else
	return ::write( ((CORE_FILE *)m_pHandle)->fh, pBuffer, nBytes );
#endif
}

void FileDisk::flush()
{}

File::Position FileDisk::setPosition( Position nPos )
{
	if ( m_pHandle == NULL )
		return 0xFFFFFFFF;

#if defined(_WIN32) || defined(_XBOX)
	dword nHigh = (dword)((qword)nPos >> 32);
	dword nLow = SetFilePointer(m_pHandle, (dword)nPos, (long *)&nHigh, FILE_BEGIN);
	if ( nLow == 0xFFFFFFFF && GetLastError() != NO_ERROR )
		return 0xFFFFFFFF;
	return (Position)( ((qword)nHigh << 32) | nLow );
#else
	return (Position)::lseek( ((CORE_FILE *)m_pHandle)->fh, (dword)nPos, SEEK_SET );
#endif
}

File::Size FileDisk::setSize(Size newSize)
{
    if (newSize > position())
	{
        // changing file size but not current position
        Position savedPosition( position() );

		setPosition(newSize);
        setEOF();

		// restore the position
        setPosition( savedPosition );
	}
	else
    {
        // truncating file down below current position
        setPosition(newSize);
        setEOF();
    }

	return size();
}

File::Size FileDisk::setEOF()
{
	if ( m_pHandle == NULL )
		return 0;
#if defined(_WIN32) || defined(_XBOX)
    if (!SetEndOfFile(m_pHandle))
		return 0;
#else
	if (ftruncate( ((CORE_FILE *)m_pHandle)->fh, lseek( ((CORE_FILE *)m_pHandle)->fh, 0, SEEK_CUR) ) < 0 )
		return 0;
#endif
	return size();
}

void FileDisk::lock()
{}

void FileDisk::unlock()
{}

//----------------------------------------------------------------------------


void * FileDisk::handle() const
{
    return m_pHandle;
}

FileDisk::AccessType FileDisk::access() const
{ 
    return m_Access; 
}

const char * FileDisk::fileName() const
{
    return m_sFileName;
}

//----------------------------------------------------------------------------

bool FileDisk::open( const char * pFile, AccessType nAccess /*= READ*/, bool bSearchPath /*= true*/)
{
	close();

	if ( pFile == NULL || nAccess > READ_WRITE )
		return false;

    m_Access = nAccess;
    m_sFileName = pFile;
	normalizePath( m_sFileName.buffer() );

#if defined(_WIN32) || defined(_XBOX)
    // xlat tables
    static const dword xaccess[3] =
    {
        GENERIC_READ,                       // READ
        GENERIC_WRITE,                      // WRITE
        GENERIC_READ | GENERIC_WRITE        // READ_WRITE
    };                                      
    static const dword xshare[3] =        
    {                                       
        FILE_SHARE_READ|FILE_SHARE_WRITE,	// READ
        FILE_SHARE_READ,                    // WRITE
        FILE_SHARE_READ                     // READ_WRITE
    };                                      
    static const dword xcreate[3] = 
    {                                       
        OPEN_EXISTING,                      // READ
        OPEN_ALWAYS,						// WRITE
        OPEN_ALWAYS                         // READ_WRITE
    };

	m_pHandle = CreateFileA( m_sFileName, xaccess[nAccess], xshare[nAccess], NULL, 
		xcreate[nAccess], FILE_ATTRIBUTE_NORMAL, NULL);
	if ( m_pHandle == INVALID_HANDLE_VALUE && nAccess == READ && bSearchPath )
	{
		int i = 0;
		for ( ; i < s_SearchPaths.size(); i++ )
		{
			m_sFileName = s_SearchPaths[i] + pFile;
			m_pHandle = CreateFileA( m_sFileName, xaccess[nAccess], xshare[nAccess], NULL, 
				xcreate[nAccess], FILE_ATTRIBUTE_NORMAL, NULL);
			if ( m_pHandle != INVALID_HANDLE_VALUE )
				break;
		}
	}
	if ( m_pHandle == INVALID_HANDLE_VALUE )
		return false;		// failed to open the file

	// From the SDK documentation:
	// Windows Server 2003, Windows XP, Windows 2000:  If CREATE_ALWAYS and FILE_ATTRIBUTE_NORMAL are specified, 
	// CreateFile will fail and set the last error to ERROR_ACCESS_DENIED if the file exists and has the FILE_ATTRIBUTE_HIDDEN or 
	// FILE_ATTRIBUTE_SYSTEM attribute. To avoid the error, specify the same attributes as the existing file.
	
	// We avoid this problem, by using the OPEN_ALWAYS, and not using CREATE_ALWAYS... then we trunc the file automatically
	// if the file is being opened with WRITE access

	if ( nAccess == WRITE )
		SetEndOfFile( m_pHandle );
#else
	static int filemode[3] =
	{
		O_RDONLY,					// READ
		O_WRONLY|O_CREAT|O_TRUNC,	// WRITE
		O_RDWR|O_CREAT,				// READ_WRITE
	};

	int fh = ::open( m_sFileName, filemode[ nAccess ], S_IRWXU | S_IRWXG | S_IROTH | S_IWOTH );	
	if ( fh < 0 && bSearchPath )
	{
		int i = 0;
		for ( ; i < s_SearchPaths.size(); i++ )  
		{
			m_sFileName = s_SearchPaths[i] + CharString( pFile );
			int fh = ::open( m_sFileName, filemode[ nAccess ] );
			if ( fh >= 0 )
				break;
		}
	}

	if ( fh < 0 )
	{
		LOG_ERROR( "FileDisk", "Failed to open file %s, errno = %d", m_sFileName.cstr(), errno );
		return false;
	}

	// do file locking, so we are as close to Windows as possible, plus some code depends on file locks
	// to prevent multiple servers from writing to the same file at the same time.
	if ( nAccess == WRITE || nAccess == READ_WRITE )
	{
		if ( lockf( fh, F_TLOCK, 0 ) < 0 )
		{
			::close( fh );
			return false;		// failed to get a lock, close handle and return failure..
		}
	}

	CORE_FILE * pFH = new CORE_FILE;
	pFH->fh = fh;

	m_pHandle = pFH;
#endif

	return true;
}

void FileDisk::close()
{
 	if ( m_pHandle != 0 )
	{
#if defined(_WIN32) || defined(_XBOX)
		CloseHandle( m_pHandle );
#else
		// close releases file locks as well, so no need to call lockf again..
		::close( ((CORE_FILE *)m_pHandle)->fh );
		delete (CORE_FILE *)m_pHandle;
#endif
		m_pHandle = 0;
	}
}

//---------------------------------------------------------------------------------------------------

void FileDisk::normalizePath( char * pPath )
{
#if defined(_WIN32)
	for(int i=0;pPath[i];++i)
		if ( pPath[i] == '/' )
			pPath[i] = '\\';
#else
	for(int i=0;pPath[i];++i)
		if ( pPath[i] == '\\' )
			pPath[i] = '/';
#endif
}

CharString FileDisk::home()
{
#if defined(_WIN32)
	char sHome[ MAX_PATH ];
	GetModuleFileNameA( GetModuleHandle(NULL), sHome, MAX_PATH );

	// find the last slash
	char * pLastSlash = strrchr( sHome, PATH_SEPERATORC );
	if ( pLastSlash == NULL )
		return ".";
	*pLastSlash = 0;
	return sHome;

#elif defined(_XBOX)
	return "Z:\\";
#else
	return ".";
#endif
}

bool FileDisk::seekHome()
{
#if defined(_WIN32)
	return _chdir( home() ) == 0;
#else
	return chdir( home() ) == 0;
#endif
}

CharString FileDisk::currentDirectory()
{
	char sCurrent[ MAX_PATH ];
#if defined(_WIN32)
	return CharString( _getcwd( sCurrent, MAX_PATH ) );
#else
	return CharString( getcwd( sCurrent, MAX_PATH ) );
#endif
}

void FileDisk::addSearchPath(const char * pPath )
{
	CharString sPath( pPath );
	normalizePath( sPath.buffer() );

	if ( sPath[ sPath.length() - 1 ] != PATH_SEPERATORC )
		sPath += PATH_SEPERATORC;

	s_SearchPaths.push( sPath );
}

void FileDisk::resetSearchPaths()
{
    s_SearchPaths.release();
}

bool FileDisk::fileExists( const char * pFile )
{
#if defined(_WIN32) || defined(_XBOX)
	return GetFileAttributesA( pFile ) != 0xffffffff;
#else
	CharString sFile( pFile );
	normalizePath( sFile.buffer() );

	// remove any trailing slash from the name
	int n = strlen( sFile );
	if ( n > 0 && sFile[ n - 1 ] == PATH_SEPERATORC )
		sFile[ n - 1 ] = 0;

	struct stat st;
	if ( stat( sFile, &st ) < 0 )
		return false;

	return true;
#endif
}

bool FileDisk::fileLocked( const char * pFile )
{
#if defined(_WIN32) || defined(_XBOX)
	return (GetFileAttributesA( pFile ) & FILE_ATTRIBUTE_READONLY) != 0;
#else
	CharString sFile( pFile );
	normalizePath( sFile.buffer() );

	struct stat st;
	if ( stat( sFile, &st ) < 0 )
		return false;
	return (st.st_mode & S_IWRITE) == 0;
#endif
}

bool FileDisk::fileHidden( const char * pFile )
{
#if defined(_WIN32) || defined(_XBOX)
	return (GetFileAttributesA( pFile ) & FILE_ATTRIBUTE_HIDDEN) != 0;
#else
	return pFile[0] == '.';
#endif
}

dword FileDisk::fileDate( const char * pFile )
{
#if defined(_WIN32) || defined(_XBOX)
	// open the file
	HANDLE fh = CreateFileA( pFile, GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
    if ( fh == INVALID_HANDLE_VALUE)
		return 0;
	
	FILETIME ft;
	GetFileTime( fh, NULL, NULL, &ft );
	CloseHandle( fh );

	// convert the filetime to a time_t
	qword time = ft.dwLowDateTime | (qword( ft.dwHighDateTime ) << 32);
	time -= 116444736000000000;		// midnight (00:00:00), January 1, 1970
	time /= 10000000;				// convert from nano-seconds to seconds

	return( (dword)(time & 0xfffffffe) );		// remove the first bit to round number to even second
												// certain file systems only support a 2 second resolution
#else
	CharString sFile( pFile );
	normalizePath( sFile.buffer() );

	struct stat st;
	if ( stat( sFile, &st ) < 0 )
		return false;
	return st.st_mtime;
#endif
}

dword FileDisk::fileCreationDate( const char * pFile )
{
#if defined(_WIN32) || defined(_XBOX)
	// open the file
	HANDLE fh = CreateFileA( pFile, GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
    if ( fh == INVALID_HANDLE_VALUE)
		return 0;
	
	FILETIME ft;
	GetFileTime( fh, &ft, NULL, NULL );
	CloseHandle( fh );

	// convert the filetime to a time_t
	qword time = ft.dwLowDateTime | (qword( ft.dwHighDateTime ) << 32);
	time -= 116444736000000000;		// midnight (00:00:00), January 1, 1970
	time /= 10000000;				// convert from nano-seconds to seconds

	return( (dword)(time & 0xfffffffe) );		// remove the first bit to round number to even second
												// certain file systems only support a 2 second resolution
#else
	CharString sFile( pFile );
	normalizePath( sFile.buffer() );

	struct stat st;
	if ( stat( sFile, &st ) < 0 )
		return false;
	return st.st_ctime;
#endif
}

dword FileDisk::fileSize( const char * pFile )
{
#if defined(_WIN32) || defined(_XBOX)
	WIN32_FIND_DATA ff;
	
	HANDLE fh = FindFirstFileA( pFile, &ff );
	if ( fh == INVALID_HANDLE_VALUE )
		return 0;
	FindClose( fh );

	return ff.nFileSizeLow;
#else
	CharString sFile( pFile );
	normalizePath( sFile.buffer() );

	struct stat st;
	if ( stat( sFile, &st ) < 0 )
		return false;
	return st.st_size;
#endif
}

bool FileDisk::deleteFile( const char * pFile )
{
#if defined(_WIN32) || defined(_XBOX)
	return DeleteFileA( pFile ) != 0;
#else
	CharString sFile( pFile );
	normalizePath( sFile.buffer() );

	return unlink( sFile ) == 0;
#endif
}

bool FileDisk::renameFile( const char * pOldName, const char * pNewName )
{
#if defined(_WIN32) || defined(_XBOX)
	return MoveFileA( pOldName, pNewName) != 0;
#else
	CharString sOldName( pOldName );
	normalizePath( sOldName.buffer() );
	CharString sNewName( pNewName );
	normalizePath( sNewName.buffer() );

	return rename( sOldName, sNewName) == 0;
#endif
}

bool FileDisk::copyFile( const char * pFrom, const char * pTo )
{
#if defined(_WIN32) || defined(_XBOX)
	return CopyFileA( pFrom, pTo, FALSE ) != 0;
#else
	qword nDstSize = 0;
	qword nSrcSize = (qword)-1;

	FileDisk from;
	if ( from.open( pFrom, READ ) )
	{
		FileDisk to;
		if ( to.open( pTo, WRITE ) )
		{
			nSrcSize = from.size();
			while( nDstSize < nSrcSize )
			{
				char sBuffer[ 1024 * 8 ];
				
				qword nBytes = nSrcSize - nDstSize;
				if ( nBytes > (1024 * 8) )
					nBytes = 1024 * 8;

				if ( from.read( sBuffer, (dword)nBytes ) != nBytes )
					return false;
				if ( to.write( sBuffer, nBytes ) != nBytes )
					return false;

				nDstSize += nBytes;
			}
		}
	}
	return true;
#endif
}

bool FileDisk::lockFile( const char * pFile, bool bReadOnly )
{
#if defined(_WIN32) || defined(_XBOX)
	dword nAttributes = GetFileAttributes( pFile );
	if ( bReadOnly )
		nAttributes |= FILE_ATTRIBUTE_READONLY;
	else
		nAttributes &= ~FILE_ATTRIBUTE_READONLY;

	return SetFileAttributes( pFile, nAttributes ) != 0;
#else
	CharString sFile( pFile );
	normalizePath( sFile.buffer() );

	struct stat st;
	if ( stat( sFile, &st ) < 0 )
		return false;

	if ( bReadOnly )
		st.st_mode &= ~S_IWRITE;
	else
		st.st_mode |= S_IWRITE;

	return chmod( pFile, st.st_mode ) == 0;
#endif
}

bool FileDisk::hideFile( const char * pFile, bool bHide )
{
#if defined(_WIN32) || defined(_XBOX)
	dword nAttributes = GetFileAttributesA( pFile );
	if ( bHide )
		nAttributes |= FILE_ATTRIBUTE_HIDDEN;
	else
		nAttributes &= ~FILE_ATTRIBUTE_HIDDEN;

	return SetFileAttributes( pFile, nAttributes ) != 0;
#else
	return false;		// not supported in unix!
#endif
}

bool FileDisk::touchFile( const char * pFile, dword nDate /*= 0*/ )
{
#if defined(_WIN32) || defined(_XBOX)
	if ( nDate == 0 )
		nDate = (dword)::time( NULL );	// set to the current time by default

	// convert the time_t to a FILETIME 
	LONGLONG ll = Int32x32To64(nDate, 10000000) + 116444736000000000;

	FILETIME ft;
    ft.dwLowDateTime = (DWORD) ll;
    ft.dwHighDateTime = (DWORD) (ll >> 32);

	// open the file
	HANDLE fh = CreateFileA( pFile, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
    if ( fh == INVALID_HANDLE_VALUE)
		return false;
	SetFileTime( fh, NULL, NULL, &ft );
	// close the file
	CloseHandle( fh );

	// success!
	return true;
#else
	CharString sFile( pFile );
	normalizePath( sFile.buffer() );

	if ( nDate == 0 )
		nDate = ::time( NULL );	// set to the current time by default

	struct utimbuf utb;
	utb.actime = utb.modtime = nDate;

	return ::utime( sFile, &utb ) == 0;
#endif
}

bool FileDisk::isDirectory( const char * pFile )
{
#if defined(_WIN32) || defined(_XBOX)
	return (GetFileAttributesA( pFile ) & FILE_ATTRIBUTE_DIRECTORY) != 0;
#else
	CharString sFile( pFile );
	normalizePath( sFile.buffer() );

	struct stat st;
	if ( stat( sFile, &st ) < 0 )
		return false;
	return (st.st_mode & S_IFDIR) != 0;
#endif
}

static bool MakeDirectory( const char * pDir )
{
#if defined(_WIN32) || defined(_XBOX)
	CharString sPath = pDir;
	FileDisk::normalizePath( sPath.buffer() );
	return CreateDirectoryA( sPath, NULL ) != 0;
#else
	CharString sPath( pDir );
	FileDisk::normalizePath( sPath.buffer() );
	return mkdir( sPath, S_IRWXU|S_IRWXG|S_IRWXO ) == 0;
#endif
}

bool FileDisk::createDirectory( const char * pDir )
{
	CharString sPath = pDir;
	normalizePath( sPath.buffer() );

	if (!fileExists( sPath ) && !MakeDirectory( sPath ) )
	{
		CharString sDir, sFull;

		if ( sPath.length() == 0 )
			return false;
		// make sure a trailing slash is on the path
		if ( sPath[ sPath.length() - 1 ] != PATH_SEPERATORC )
			sPath += PATH_SEPERATORC;
		// failed to create directory, it may already exist, if so just return true 
		if ( fileExists( sPath ) )
			return true;

		// handle \\ Paths
		if ( sPath[0] == PATH_SEPERATORC && sPath[1] == PATH_SEPERATORC )
		{
			sDir = sPath;
			sDir.left( 2 );
			sPath.right( sPath.length() - 2 );
			sFull += sDir;

			int nSlash = -1;
			while( (nSlash = sPath.find(PATH_SEPERATORC)) >= 0 )
			{
				sDir = sPath;
				sDir.left( nSlash + 1 );
				sPath.right( sPath.length() - sDir.length() );

				sFull += sDir;
				if ( fileExists( sFull ) )
					break;		// drive found
			}
		}

		// handle drive letters
		if ( isalpha( sPath[0] ) && sPath[1] == ':' && sPath[2] == PATH_SEPERATORC )
		{
			sFull += CharString( sPath ).left( 3 );
			sPath.right( sPath.length() - 3 );
		}

		// process the remaining path...
		int nSlash = -1;
		while( (nSlash = sPath.find(PATH_SEPERATORC)) >= 0 )
		{
			sDir = sPath;
			sDir.left( nSlash + 1 );
			sPath.right( sPath.length() - sDir.length() );

			sFull += sDir;
			if ( !fileExists( sFull ) && !MakeDirectory( sFull ) )
				return false;
		}

		return fileExists( pDir );
	}

	return true;
}

static bool RemoveDir( const char * pDir )
{
#if defined(_WIN32) || defined(_XBOX)
	CharString sPath = pDir;
	FileDisk::normalizePath( sPath.buffer() );
	return RemoveDirectoryA( sPath ) != 0;
#else
	CharString sPath( pDir );
	FileDisk::normalizePath( sPath.buffer() );
	return rmdir( sPath ) == 0;
#endif
}

bool FileDisk::removeDirectory( const char * pDir, bool bForce /*= false*/ )
{
	if ( bForce )
	{
		// remove any files in this directory
		CharString sDir = pDir;
		if (! sDir.endsWith( PATH_SEPERATOR ) )
			sDir += PATH_SEPERATOR;

		FindFile ff;
		ff.findFiles( sDir + "*.*", true, false );

		for(int k=0;k<ff.directoryCount();k++)
		{
			CharString sSubDir = ff.directory( k );
			if ( sSubDir[0] == '.' )
				continue;

			if (! removeDirectory( sDir + sSubDir + PATH_SEPERATOR, true ) )
				return false;			// failed to remove the sub-directory
		}

		for(int i=0;i<ff.fileCount();i++)
		{
			// make sure the read-only flag is removed
			lockFile( sDir + ff.file( i ), false );
			if (! deleteFile( sDir + ff.file( i ) ) )
				return false;			// failed to delete the file
		}
	}

	return RemoveDir( pDir );
}

void * FileDisk::loadFile( const char * pFile )
{
	FileDisk fd;
	if ( fd.open( pFile ) )
	{
		int nSize = (int)fd.size();
		void * pBuffer = new byte[ nSize ];

		if ( fd.read( pBuffer, nSize ) != nSize )
		{
			delete (byte *)pBuffer;
			pBuffer = NULL;
		}

		return pBuffer;
	}

	return NULL;
}

bool FileDisk::saveFile( const char * pFile, const void * pData, dword nBytes )
{
	FileDisk fd;
	if ( fd.open( pFile, WRITE ) )
		return fd.write( pData, nBytes ) == nBytes;
	return false;
}

char * FileDisk::loadTextFile( const char * fileName )
{
	FileDisk fd;
	if ( fd.open( fileName ) )
	{
		int nSize = (int)fd.size();
		char * pBuffer = new char[ nSize + 1];

		if ( fd.read( pBuffer, nSize ) == nSize )
		{
			// NULL terminate the buffer
			pBuffer[ nSize ] = 0;
		}
		else
		{
			delete [] pBuffer;
			pBuffer = NULL;
		}

		return pBuffer;
	}

	return NULL;

}

bool FileDisk::saveTextFile( const char * pFile, const char * pText )
{
	return saveFile( pFile, pText, (dword)strlen( pText ) );
}

wchar * FileDisk::loadUnicodeFile( const char * pFile )
{
	FileDisk fd;
	if ( fd.open( pFile ) )
	{
		// check for a unicode signature
		word ID = 0;
		if ( fd.read( &ID, sizeof(ID) ) != sizeof(ID) )
			return NULL;
		if ( ID != 0xFEFF )
			return NULL;		// nope, not a unicode text file

		int nSize = fd.size() - sizeof(word);
		int nCharacters = nSize / sizeof(wchar);
		wchar * pBuffer = new wchar[ nCharacters + 1 ];

		if ( fd.read( pBuffer, nSize ) == nSize )
		{
			// NULL terminate the buffer
			pBuffer[ nCharacters ] = 0;		// NULL terminate the string
		}
		else
		{
			delete [] pBuffer;
			pBuffer = NULL;
		}

		return pBuffer;
	}

	return NULL;
}

bool FileDisk::saveUnicodeFile( const char * pFile, const wchar * pText )
{
	FileDisk fd;
	if ( fd.open( pFile, WRITE ) )
	{
		// write a signature which indicates this is a unicode file, editors such as WordPad reconized this signature
		word ID = 0xFEFF;
		fd.write( &ID, sizeof(word) );
		// write the text
		fd.write( pText, (dword)( wcslen( pText ) * sizeof(wchar) ) );

		return true;
	}
	return false;

}

void FileDisk::unloadTextFile( char * pText )
{
	delete [] pText;
}

void FileDisk::unloadUnicodeFile( wchar * pText )
{
	delete [] pText;
}

//-------------------------------------------------------------------------------
// EOF
