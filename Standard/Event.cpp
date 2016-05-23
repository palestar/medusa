/*
	Event.cpp

	Event object, used to syncronize multiple threads
	(c)2005 Palestar, Richard Lyle
*/

#include "Debug/Assert.h"
#include "Debug/Log.h"
#include "Event.h"
#include "Atomic.h"
#include "CharString.h"

#if defined(_WIN32)
#include <windows.h>
#else

#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

struct PTEvent
{
	PTEvent( bool a_bManualReset, const char * a_pName ) : 
		m_bAutoReset( !a_bManualReset ),
		m_bState( false ),
		m_pName( a_pName != NULL ? strdup( a_pName ) : NULL ),
		m_nPipeFD( -1 ),
		m_bPipeOwner( false ),
		m_nUsers( 1 )
	{
		pthread_cond_init(&m_Cond, 0 );
		pthread_mutex_init(&m_Mutex, 0 );

		if ( m_pName != NULL )
		{
			// for named events we use named pipes..
			m_sNamedPipe.format( "/tmp/%s.ipc", m_pName );

			m_nPipeFD = ::open( m_sNamedPipe, O_RDWR|O_CREAT|O_NONBLOCK, 0666 );
			if ( m_nPipeFD >= 0 )
			{
				// attempt to lock the pipe, if we succed then will will delete the pipe on destruction..
				m_bPipeOwner = ::lockf( m_nPipeFD, F_TLOCK, 0 ) == 0;
				// start reading at the end of the file..
				::lseek( m_nPipeFD, 0, SEEK_END );
			}
			else
				LOG_ERROR( "Event", "Failed to open pipe %s.", m_sNamedPipe.cstr() );

		}
	}
	~PTEvent()
	{
		ASSERT( m_nUsers == 0 );
		if ( m_pName != NULL )
		{
			if ( m_nPipeFD >= 0 )
			{
				::close( m_nPipeFD );
				m_nPipeFD = -1;

				if ( m_bPipeOwner )
				{
					LOG_STATUS( "Event", "Owner deleting named pipe %s", m_sNamedPipe.cstr() );
					::unlink( m_sNamedPipe );
				}
			}

			free( m_pName );
			m_pName = NULL;
		}

		pthread_cond_destroy( &m_Cond );
		pthread_mutex_destroy( &m_Mutex );
	}

	bool wait( dword a_nTimeout )
	{
		if( pthread_mutex_lock( &m_Mutex ) != 0 )
		{
			LOG_ERROR( "Event", "Failed to lock mutex for event" );
			return false;
		}
	    
		int rv = -1;
		if (! m_bState )
		{
			if ( m_pName != NULL )
			{
				timeval tv, to;
				if ( a_nTimeout != 0xffffffff )
				{
					gettimeofday(&tv, NULL);
					qword microseconds = ((qword)tv.tv_sec * 1000 * 1000) + ((qword)a_nTimeout * 1000) + (qword)tv.tv_usec;
					to.tv_sec = microseconds / (1000 * 1000);
					to.tv_usec = microseconds - ((qword)to.tv_sec * 1000 * 1000);
				}

				rv = 0;
				do {
					pthread_mutex_unlock( &m_Mutex );

					char buf[1];
					int nBytesRead = ::read( m_nPipeFD, buf, 1 );
					if ( nBytesRead < 0 && errno != EAGAIN && errno != EWOULDBLOCK )
						LOG_ERROR( "Event", "read returned error %d when reading from pipe %s", errno, m_sNamedPipe.cstr() );
					else if ( nBytesRead <= 0 )
						Thread::sleep( 1 );

					if( pthread_mutex_lock( &m_Mutex ) != 0 )
					{
						LOG_ERROR( "Event", "Failed to lock mutex for event" );
						return false;
					}

					if ( nBytesRead == 1 )
						m_bState = buf[0];

					gettimeofday(&tv, NULL);
					if ( tv.tv_sec > to.tv_sec || (tv.tv_sec == to.tv_sec && tv.tv_usec >= to.tv_usec) )
						rv = -1;	// to

				} while( rv == 0 && !m_bState );

				if( rv == 0 && m_bAutoReset )
					m_bState = false;
			}
			else
			{
				timespec to; timeval tv;
				if(a_nTimeout != 0xffffffff)
				{
					gettimeofday(&tv, NULL);

					qword nanoseconds = ((qword)tv.tv_sec * 1000 * 1000 * 1000) + ((qword)a_nTimeout * 1000 * 1000) + ((qword) tv.tv_usec * 1000);
					to.tv_sec = nanoseconds / (1000 * 1000 * 1000);
					to.tv_nsec = nanoseconds - ((qword)to.tv_sec * 1000 * 1000 * 1000);
				}

				do 	{
					//Regardless of whether it's an auto-reset or manual-reset event:
					//wait to obtain the event, then lock anyone else out
					if(a_nTimeout != 0xffffffff)
						rv = pthread_cond_timedwait(&m_Cond, &m_Mutex, &to);
					else
						rv = pthread_cond_wait(&m_Cond, &m_Mutex );

				} while(rv == 0 && !m_bState );

				if( rv == 0 && m_bAutoReset )
					m_bState = false;
			}
		}
		else
		{
			if( m_bAutoReset )
				m_bState = false;
			rv = 0;
		}

		pthread_mutex_unlock( &m_Mutex );
		return rv != 0;
	}

	bool signaled() 
	{
		if ( m_pName != NULL )
		{
			// for named Events, we need to call wait() so the signalled flag can be set..
			if (! wait( 0 ) )
				return true;
			return false;
		}
		else
		{
			if( pthread_mutex_lock( &m_Mutex ) != 0)
				return false;
		    
			bool bSignaled = m_bState;
			if( m_bAutoReset )
			{
				//It's an auto-reset event that's currently available;
				//we need to stop anyone else from using it
				m_bState = false;
			}
			//Else we're trying to obtain a manual reset event with a signalled state;
			//don't do anything

			pthread_mutex_unlock( &m_Mutex );

			return bSignaled;
		}
	}

	void clear()
	{
		if ( m_pName != NULL )
		{
			char buf[1];
			buf[0] = 0;

			int nBytesWriteen = ::write( m_nPipeFD, buf, 1 );
			if ( nBytesWriteen < 1 )
				LOG_DEBUG_LOW( "Event", "Failed to clear pipe %s.", m_sNamedPipe.cstr() );
		}
		else
		{
			if( pthread_mutex_lock( &m_Mutex ) == 0)
			{
				m_bState = false;
				pthread_mutex_unlock( &m_Mutex );
			}
		}
	}

	void signal()
	{
		if ( m_pName != NULL )
		{
			char buf[1];
			buf[0] = 1;

			int nBytesWriteen = ::write( m_nPipeFD, buf, 1 );
			if ( nBytesWriteen < 1 )
				LOG_DEBUG_LOW( "Event", "Failed to signal pipe %s.", m_sNamedPipe.cstr() );
		}
		else
		{
			if( pthread_mutex_lock( &m_Mutex ) == 0)
			{
				m_bState = true;

				if( m_bAutoReset )
					pthread_cond_signal(&m_Cond);
				else
					pthread_cond_broadcast(&m_Cond);

				pthread_mutex_unlock( &m_Mutex );
			}
		}
	}

	//! Data
	bool			m_bAutoReset;
	bool			m_bState;

	char *			m_pName;
	CharString		m_sNamedPipe;		// our named pipe filename
	int				m_nPipeFD;			// our named pipe file descriptor
	bool			m_bPipeOwner;

	pthread_cond_t	m_Cond;
	pthread_mutex_t m_Mutex;
	unsigned int	m_nUsers;
};

#endif

//-------------------------------------------------------------------------------

Event::Event() : m_hEvent( NULL )
{
	m_hEvent = eventCreate();
	if ( m_hEvent == NULL )
		throw FailedToCreateEvent();
}

Event::Event( bool bManualReset ) : m_hEvent( NULL )
{
	m_hEvent = eventCreate( bManualReset );
	if ( m_hEvent == NULL )
		throw FailedToCreateEvent();
}

Event::Event( const char * pName ) : m_hEvent( NULL )
{
	m_hEvent = eventCreate( true, pName );
	if ( m_hEvent == NULL )
		throw FailedToCreateEvent();
}

Event::Event( const Event & copy ) : m_hEvent( NULL )
{
	operator=( copy );
}

Event::~Event()
{
	if ( m_hEvent != NULL )
		eventClose( m_hEvent );
}

//-------------------------------------------------------------------------------

void Event::wait() const
{
	eventWait( m_hEvent );
}

bool Event::wait( dword nTimeOut )
{
	return eventWait( m_hEvent, nTimeOut );
}

bool Event::signaled() const
{
	return eventSignaled( m_hEvent );
}

//----------------------------------------------------------------------------

void Event::clear()
{
	eventClear( m_hEvent );
}

void Event::signal()
{
	eventSignal( m_hEvent );
}

Event & Event::operator=( const Event & copy )
{
	// release the old event handle
	if ( m_hEvent != NULL )
		eventClose( m_hEvent );
	if (! (m_hEvent = eventDuplicate( copy.m_hEvent )) )
		throw FailedToCreateEvent();
	
	return *this;
}

//---------------------------------------------------------------------------------------------------

void * Event::eventCreate( bool bManualReset /*= false*/, const char * pName /*= NULL*/ )
{
#if defined(_WIN32) || defined(_XBOX)
	SECURITY_ATTRIBUTES sa;
	sa.nLength = sizeof( SECURITY_ATTRIBUTES );
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle = true;

	return CreateEventA( &sa, bManualReset, false, pName );
#else
	return new PTEvent( bManualReset, pName );
#endif
}

void * Event::eventDuplicate( void * pEvent )
{
#if defined(_WIN32) || defined(_XBOX)
	// duplicate the other event handle
	void * pDup = NULL;
	if (! DuplicateHandle( GetCurrentProcess(), (HANDLE)pEvent, GetCurrentProcess(), 
		(HANDLE *)&pDup, 0, true, DUPLICATE_SAME_ACCESS ) )
		return NULL;

	return pDup;
#else
	if (! pEvent )
		return NULL;
	((PTEvent *)pEvent)->m_nUsers += 1;
	return pEvent;
#endif
}

bool Event::eventWait( void * pEvent, dword nTimeOut /*= 0xffffffff*/ )
{
	if (! pEvent )
		return false;

#if defined(_WIN32) || defined(_XBOX)
	return WaitForSingleObject( (HANDLE)pEvent, nTimeOut ) == WAIT_TIMEOUT;
#else
	return ((PTEvent *)pEvent)->wait( nTimeOut );
#endif
}

bool Event::eventSignaled( void * pEvent )
{
	if (! pEvent )
		return false;
#if defined(_WIN32) || defined(_XBOX)
	return WaitForSingleObject( (HANDLE)pEvent, 0 ) == WAIT_OBJECT_0;
#else
	return ((PTEvent *)pEvent)->signaled();
#endif
}

void Event::eventClear( void * pEvent )
{
	if (! pEvent )
		return;
#if defined(_WIN32) || defined(_XBOX)
	ResetEvent( (HANDLE)pEvent );
#else
	((PTEvent *)pEvent)->clear();
#endif
}

void Event::eventSignal( void * pEvent )
{
	if (! pEvent )
		return;
#if defined(_WIN32) || defined(_XBOX)
	SetEvent( (HANDLE)pEvent );
#else
	((PTEvent *)pEvent)->signal();
#endif
}

void Event::eventClose( void * pEvent )
{
#if defined(_WIN32) || defined(_XBOX)
	CloseHandle( (HANDLE)pEvent );
#else
	if (! pEvent )
		return;
	if ( --((PTEvent *)pEvent)->m_nUsers == 0 )
		delete (PTEvent *)pEvent;
#endif
}

//-------------------------------------------------------------------------------
// EOF
