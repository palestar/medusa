/*
	FileChange.cpp

	This class is used to wait for a directory/file change, written for the MirrorServer
	who uses this to wait for a change to the directory structure.

	(c)2005 Palestar, Richard Lyle
*/

#include "Debug/Log.h"
#include "FileChange.h"

#if defined(_WIN32)
#include <windows.h>
#else

#include "FindFile.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <unistd.h>
#include <poll.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/inotify.h>
#include <sys/ioctl.h>

struct INotifyHandle {
	INotifyHandle(int a_fd ) : m_fd( a_fd )
	{}

	int		m_fd;				// file descriptor
};

#include <string>
#include <list>

#endif

//----------------------------------------------------------------------------

FileChange::FileChange() : m_pHandle( 0 )
{}

FileChange::FileChange( const char * a_pDirectory ) : m_pHandle( 0 )
{
	if ( !open( a_pDirectory ) )
		throw FileChangeFailure();
}

FileChange::~FileChange()
{
	close();
}

//----------------------------------------------------------------------------

bool FileChange::open( const char * a_pDirectory )
{
	if ( (m_pHandle = FileChange::fileChangeOpen( a_pDirectory )) == 0 )
		return false;
	return true;
}

void FileChange::close()
{
	if ( m_pHandle != 0 )
	{
		FileChange::fileChangeClose( m_pHandle );
		m_pHandle = 0;
	}
}

void FileChange::wait()
{
	FileChange::fileChangeWait( m_pHandle );
}

bool FileChange::signaled()
{
	return FileChange::fileChangePoll( m_pHandle );
}

//---------------------------------------------------------------------------------------------------

void * FileChange::fileChangeOpen( const char * a_pDirectory )
{
#if defined(_WIN32)
	void * pFile = FindFirstChangeNotification( a_pDirectory, true, 
		FILE_NOTIFY_CHANGE_FILE_NAME|FILE_NOTIFY_CHANGE_ATTRIBUTES|
		FILE_NOTIFY_CHANGE_DIR_NAME|FILE_NOTIFY_CHANGE_LAST_WRITE );
	if ( INVALID_HANDLE_VALUE == (HANDLE)pFile  )
		return NULL;

	return pFile;
#else
	int fd = inotify_init();
	if ( fd < 0 )
	{
		LOG_ERROR( "FileChange", "Failed initialize inotify, errno = %d", errno );
		return NULL;
	}

	// inotify has to be setup for each directory we want to watch.. too bad it can't watch 
	// all the directories under the root one.
	std::list< std::string > dirList;
	dirList.push_back( a_pDirectory );

	while( dirList.size() > 0 )
	{
		std::string sDirectory( dirList.front() );
		dirList.pop_front();

		int wd = inotify_add_watch( fd, sDirectory.c_str(), 
			IN_CLOSE_WRITE|IN_CREATE|IN_DELETE|IN_DELETE_SELF|
			IN_MODIFY|IN_MOVE_SELF|IN_MOVED_FROM|IN_MOVED_TO );
		if ( wd < 0 )
			LOG_ERROR( "FileChange", "Failed to add watch for directory %s, errno = %d", sDirectory.c_str(), errno );

		FindFile ff( (sDirectory + PATH_SEPERATOR + "*").c_str(), false, true );
		for(int i=0;i<ff.directoryCount();++i)
			dirList.push_back( ff.directory(i) );
	}

	return new INotifyHandle( fd );
#endif
}

bool FileChange::fileChangeWait( void * pFile )
{
#if defined(_WIN32)
	// wait for file change or timeout
	while( WaitForSingleObject( (HANDLE)pFile, 1000 ) == WAIT_TIMEOUT );
	// setup for next wait
	FindNextChangeNotification( (HANDLE)pFile );
	return true;
#else
	INotifyHandle * pHandle = (INotifyHandle *)pFile;

	fd_set rfds;
	FD_ZERO (&rfds);
	FD_SET (pHandle->m_fd, &rfds);
	if ( select (FD_SETSIZE, &rfds, NULL, NULL, NULL) < 0 )
		return false;

	// get the number of bytes in the buffer, and read them all out..
	int nBytes = 0;
	if ( ioctl( pHandle->m_fd, FIONREAD, &nBytes ) == 0 )
	{
		while( nBytes > 0 )
		{
			char buffer[ 1024 * 8 ];
			int r = read( pHandle->m_fd, buffer, nBytes > sizeof(buffer) ? sizeof(buffer) : nBytes );
			if ( r < 0 )
				break;
			nBytes -= r;
		}
	}

	return true;
#endif
}

bool FileChange::fileChangePoll( void * pFile )
{
	if (! pFile )
		return false;

#if defined(_WIN32)
	dword nSignaled = WaitForSingleObject( (HANDLE)pFile, 0 );
	if ( nSignaled == WAIT_TIMEOUT )
		return false;

	// signaled!
	FindNextChangeNotification( (HANDLE)pFile );
	return true;
#else
	INotifyHandle * pHandle = (INotifyHandle *)pFile;

	bool bSignaled = false;

	// get the number of bytes in the buffer, and read them all out..
	int nBytes = 0;
	if ( ioctl( pHandle->m_fd, FIONREAD, &nBytes ) == 0 && nBytes > 0 )
	{
		bSignaled = true;

		while( nBytes > 0 )
		{
			char buffer[ 1024 * 8 ];
			int r = read( pHandle->m_fd, buffer, nBytes > sizeof(buffer) ? sizeof(buffer) : nBytes );
			if ( r < 0 )
				break;
			nBytes -= r;
		}
	}

	return bSignaled;
#endif
}

void FileChange::fileChangeClose( void * pFile )
{
	if (! pFile )
		return;

#if defined(_WIN32)
	FindCloseChangeNotification( (HANDLE)pFile );
#else
	INotifyHandle * pHandle = (INotifyHandle *)pFile;
	::close( pHandle->m_fd );
	delete pHandle;
#endif
}

//----------------------------------------------------------------------------
// EOF
