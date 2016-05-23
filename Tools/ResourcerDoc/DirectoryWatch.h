/*
	DirectoryWatch.h
	(c)2005 Palestar, Richard Lyle
*/

#ifndef DIRECTORY_WATCH_H
#define DIRECTORY_WATCH_H

#include "Standard/Thread.h"
#include "Resource.h"

class DirectoryWatch : public Thread
{
public:
	// Construction
	DirectoryWatch( const CString &directory, bool watchFiles, HWND refreshWindow ) 
		: m_RefreshWindow( refreshWindow ), m_Running( true )
	{
		m_ChangeHandle = FindFirstChangeNotification(
			directory, true, 
			watchFiles ? FILE_NOTIFY_CHANGE_DIR_NAME|FILE_NOTIFY_CHANGE_FILE_NAME|FILE_NOTIFY_CHANGE_ATTRIBUTES
			|FILE_NOTIFY_CHANGE_SIZE|FILE_NOTIFY_CHANGE_LAST_WRITE : FILE_NOTIFY_CHANGE_DIR_NAME );
		ASSERT( m_ChangeHandle );
	}
	virtual ~DirectoryWatch()
	{
		m_Running = false;
		//TerminateThread( m_Handle, 0 );
		FindCloseChangeNotification( m_ChangeHandle );
	}

	// Thread interface
	int			run()
	{
		while( m_Running )
		{
			if ( WaitForSingleObject( m_ChangeHandle, 100 ) == WAIT_OBJECT_0 )
			{
				PostMessage(m_RefreshWindow, WM_COMMAND, ID_VIEW_REFRESH, 0 );
				FindNextChangeNotification( m_ChangeHandle );
			}
		}

		return 0;
	}

private:
	// Data
	HANDLE		m_ChangeHandle;
	HWND		m_RefreshWindow;

	bool		m_Running;
};

#endif

//----------------------------------------------------------------------------
// EOF
