/*
	Thread.cpp
	(c)2005 Palestar, Richard Lyle
*/

#include "Debug/Log.h"
#include "Standard/Thread.h"
#include "Standard/Types.h"

#if defined(_WIN32)
#include <windows.h>
#else
#include <sys/time.h>
#include <pthread.h>
#include <unistd.h>
#endif

//----------------------------------------------------------------------------

const int KILL_TIMEOUT		= 30 * 1000;		// when destroying a thread, wait this long before killing the thread outright
const int MIN_STACK_SIZE	= 16 * 1024;

CriticalSection					Thread::sm_Lock;						// sempahore for locking the thread list
std::map< dword, Thread * >		Thread::sm_Threads;					// static list of all thread objects

//---------------------------------------------------------------------------------------------

Thread::Thread( Priority p /*= STANDARD*/, int stack /*= DEFAULT_STACK_SIZE*/ ) :
	m_Priority( p ), 
	m_StackSize( stack ), 
	m_Id( 0 ), 
	m_bSuspended( true ), 
	m_pHandle( NULL ), 
	m_nExitCode( -1 ), 
	m_bAutoDelete( false ),
	m_nNextStorageId( 1 )
{
	if ( m_StackSize < MIN_STACK_SIZE )
		m_StackSize = MIN_STACK_SIZE;
}

//! This constructor is only used when allocating a Thread object for a thread not created through this thread object.
Thread::Thread( unsigned int a_nID, void * a_pHandle ) :
	m_Priority( STANDARD ), 
	m_StackSize( DEFAULT_STACK_SIZE ), 
	m_Id( a_nID ), 
	m_bSuspended( false ), 
	m_pHandle( a_pHandle ), 
	m_nExitCode( -1 ), 
	m_bAutoDelete( true ),
	m_nNextStorageId( 1 )
{
	if ( m_Id != 0 )
	{
		AutoLock lock( &sm_Lock );
		sm_Threads[ m_Id ] = this;
	}
}

Thread::~Thread()
{
	if (m_pHandle != NULL)
	{
		// don't deadlock on threads trying to delete themselves
		if (! inThread() )	
		{
			// wait for the thread to exit
			if (! wait( KILL_TIMEOUT ) )
			{
				// kill the thread
				kill();
			}
		}

		// handle may have already been released by kill()
		if ( m_pHandle != NULL )
		{
			// release the handle
#if defined(_WIN32)
			CloseHandle( (HANDLE)m_pHandle );
#else
			// detach this thread if it called delete on itself, so the resources will get cleaned up.
			pthread_detach( (pthread_t)m_pHandle );
#endif
			m_pHandle = NULL;
		}
	}
}

//---------------------------------------------------------------------------------------------

int Thread::exitCode() const
{
	return m_nExitCode;
}

//---------------------------------------------------------------------------------------------

bool Thread::suspend()
{
	if (! m_pHandle )
		return false;

	if (! m_bSuspended )
	{
#if defined(_WIN32) 
		SuspendThread( (HANDLE)m_pHandle );
		m_bSuspended = true;
		return true;
#else

#endif
	}

	return false;
}

bool Thread::resume()
{
	if ( m_bSuspended )
	{
		m_bSuspended = false;
		if ( m_pHandle == NULL )
		{
			// start a new thread
#if defined(_WIN32) || defined(_XBOX)
			m_pHandle = CreateThread(NULL, m_StackSize, (LPTHREAD_START_ROUTINE)threadMain, this, 0, (dword *)&m_Id );
#else
			pthread_attr_t attr;
			pthread_attr_init(&attr );

			if ( m_StackSize > 0 )
				pthread_attr_setstacksize(&attr, m_StackSize );

			pthread_t pt;
			if ( pthread_create( &pt, &attr, (void *(*)(void*))threadMain, this ) != 0 )
			{
				pthread_attr_destroy( &attr );
				return false;
			}
		
			m_Id = (dword)pt;
			m_pHandle = (void *)pt;

			pthread_attr_destroy( &attr );
#endif
			// did we fail to start our thread?
			if (m_pHandle == NULL)
				return false;

			// yield the CPU now so that the thread can startup, this helps prevent a race condition
			// that can cause a crash if this Thread object is deleted immediately after calling resume()
			sleep( 0 );
			return true;
		}
		else
		{
			// resume the existing the thread
#if defined(_WIN32)
			ResumeThread( (HANDLE)m_pHandle );
			return true;
#endif
		}
	}

	return false;
}

bool Thread::kill()
{
	if (m_pHandle == NULL)
		return false;

#if defined(_WIN32)
	if ( TerminateThread( (HANDLE)m_pHandle, -1 ) == 0 )
		return false;
	CloseHandle( (HANDLE)m_pHandle );
#else
	if ( pthread_cancel( (pthread_t)m_pHandle ) != 0 )
		return false;
	if ( pthread_join( (pthread_t)m_pHandle, (void **)&m_nExitCode ) != 0 )
		return false;
#endif

	m_pHandle = NULL;
	m_bSuspended = true;
	return true;
}

bool Thread::wait(int a_nTimeout /*= -1*/ )
{
	if (! m_pHandle )
		return false;

#if defined(_WIN32)
	if ( WaitForSingleObject( (HANDLE)m_pHandle, a_nTimeout < 0 ? INFINITE : a_nTimeout ) == WAIT_TIMEOUT )
		return false;		// timed out...

	// thread has ended, get the exit code and clean up...
	GetExitCodeThread( (HANDLE)m_pHandle, (dword *)&m_nExitCode );
	// close up the thread handle..
	CloseHandle( (HANDLE)m_pHandle );
	m_pHandle = NULL;
	m_bSuspended = true;

	return true;
#else
	if ( a_nTimeout >= 0 )
	{
		timeval tv;
		gettimeofday(&tv, NULL);

		qword nanoseconds = ((qword) tv.tv_sec) * 1000 * 1000 * 1000 + a_nTimeout * 1000 * 1000 + ((qword) tv.tv_usec) * 1000;

		timespec ts;
		ts.tv_sec = nanoseconds / 1000 / 1000 / 1000;
		ts.tv_nsec = (nanoseconds - ((qword) ts.tv_sec) * 1000 * 1000 * 1000);

		if ( pthread_timedjoin_np( (pthread_t)m_pHandle, (void **)&m_nExitCode, &ts ) != 0 )
			return false;		// timed out
	}
	else
	{
		if ( pthread_join( (pthread_t)m_pHandle, (void **)&m_nExitCode ) != 0 )
			return false;			// error
	}

	m_pHandle = NULL;
	m_bSuspended = true;

	return true;
#endif
}

dword Thread::allocateTS( Value a_Value )
{
	AutoLock lock( &m_StorageLock );
	dword nId = m_nNextStorageId++;
	m_Storage[ nId ] = a_Value;

	return nId;
}

Value * Thread::findTS( dword a_nId )
{
	AutoLock lock( &m_StorageLock );
	ThreadStorage::iterator iStorage = m_Storage.find( a_nId );
	if ( iStorage != m_Storage.end() )
		return &iStorage->second;
	return 0;
}

void Thread::freeTS( dword a_nId )
{
	AutoLock lock( &m_StorageLock );
	m_Storage.erase( a_nId );
}

int Thread::runProxy()
{
	return run();
}

int Thread::run()
{
	return 0;
}

//---------------------------------------------------------------------------------------------

void Thread::sleep( int ms )
{
#if defined(_WIN32) || defined(_XBOX)
	Sleep( ms );
#else
	::usleep( ms * 1000 );
#endif
}

void Thread::threadLock( bool bLock )
{
	bLock ? sm_Lock.lock() : sm_Lock.unlock();
}

int Thread::threadCount()
{
	return sm_Threads.size();
}

Thread * Thread::thread( int n )
{
	AutoLock lock( &sm_Lock );
	for( ThreadMap::iterator iThread = sm_Threads.begin();
		iThread != sm_Threads.end(); ++iThread, --n )
	{
		if ( n <= 0 )
			return iThread->second;
	}

	return NULL;
}

//---------------------------------------------------------------------------------------------------

Thread * Thread::getCurrentThread()
{
	AutoLock lock( &sm_Lock );
	
	dword nMyThreadId = getCurrentThreadId();
	ThreadMap::iterator iThread = sm_Threads.find( nMyThreadId );
	if ( iThread != sm_Threads.end() )
		return iThread->second;

	return new Thread( nMyThreadId, getCurrentThreadHandle() );
}

void * Thread::getCurrentThreadHandle()
{
#if defined(_WIN32) || defined(_XBOX)
	return GetCurrentThread();
#else
	return (void *)pthread_self();
#endif
}

dword Thread::getCurrentThreadId()
{
#if defined(_WIN32) || defined(_XBOX)
	return GetCurrentThreadId();
#else
	return (dword)pthread_self();
#endif
}

//---------------------------------------------------------------------------------------------

int Thread::threadMain( void * pArgument )
{
	dword nThreadId = getCurrentThreadId();
	Thread *pThread = (Thread *) pArgument;

	// set the thread priority
#if defined(_WIN32)
	static int priority_xlat[] =
	{
		THREAD_PRIORITY_BELOW_NORMAL,	// LOW
		THREAD_PRIORITY_NORMAL,			// STANDARD
		THREAD_PRIORITY_ABOVE_NORMAL,	// HIGH
		THREAD_PRIORITY_TIME_CRITICAL,	// CRITICAL
	};
	if ( SetThreadPriority( (HANDLE)pThread->m_pHandle, priority_xlat[ pThread->m_Priority ] ) == 0 )
		LOG_ERROR( "Thread", "Failed to set thread priority" );
#endif

	AutoLock lock( &sm_Lock );
	sm_Threads[ nThreadId ] = pThread;
	lock.release();

	// pass control off to the thread virtual
	int exitCode = pThread->runProxy();

	lock.set( &sm_Lock );
	sm_Threads.erase( nThreadId );

	if ( pThread->m_bAutoDelete )
		delete pThread;

	return exitCode;
}


//---------------------------------------------------------------------------------------------
// EOF
