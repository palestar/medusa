/*
	Mutex.cpp
	(c)2005 Palestar, Richard Lyle
*/

#include "Mutex.h"
#include "Time.h"
#include "Types.h"
#include "Debug/Trace.h"

#if defined(_WIN32)
#include <windows.h>
#else
#include <pthread.h>

struct PTMutex 
{
	PTMutex() 
	{
		pthread_mutexattr_init( &m_Attributes );
		pthread_mutexattr_settype( &m_Attributes, PTHREAD_MUTEX_RECURSIVE );
		pthread_mutex_init( &m_Mutex, &m_Attributes );
	}
	~PTMutex()
	{
		pthread_mutex_destroy( &m_Mutex );
		pthread_mutexattr_destroy( &m_Attributes );
	}

	pthread_mutex_t		m_Mutex;
	pthread_mutexattr_t m_Attributes;
};

#endif

//! Define to 1 to use a critical section instead of a mutex... it's faster, but doesn't support finding dead-locks.
#define USE_CRITICAL_SECTION 0

#if !defined(_DEBUG)
//! How long will we wait for a lock before we throw an exception... this is useful for catching dead-locks and other issues with not unlocking 
//! a semaphore correctly.
const unsigned int MAX_LOCK_TIME = 300 * 1000;
#else
//! don't timeout on debug builds, I might be stepping through code.
const unsigned int MAX_LOCK_TIME = 86400 * 1000;
#endif

//-------------------------------------------------------------------------------

Mutex::Mutex( const char * a_pName /*= NULL*/ ) 
	: m_pName( a_pName ), m_pMutex( NULL )
{
	initialize();
}

Mutex::~Mutex()
{
	release();
}

//---------------------------------------------------------------------------------------------------

void Mutex::lock()
{
#if defined(_WIN32) || defined(_XBOX)

#if USE_CRITICAL_SECTION
	if ( m_pMutex != NULL )
		EnterCriticalSection( (CRITICAL_SECTION *)m_pMutex );
#else
	if ( m_pMutex != NULL && WaitForSingleObject( (HANDLE)m_pMutex, MAX_LOCK_TIME ) != WAIT_OBJECT_0 )
		throw LockTimeout();
#endif

#else
	if ( m_pMutex != NULL )
		pthread_mutex_lock( &((PTMutex *)m_pMutex)->m_Mutex );
#endif
}

void Mutex::unlock()
{
#if defined(_WIN32) || defined(_XBOX)

#if USE_CRITICAL_SECTION
	if ( m_pMutex != NULL )
		LeaveCriticalSection( (CRITICAL_SECTION *)m_pMutex );
#else
	if ( m_pMutex != NULL )
		ReleaseMutex( (HANDLE)m_pMutex );
#endif

#else
	if ( m_pMutex != NULL )
		pthread_mutex_unlock( &((PTMutex *)m_pMutex)->m_Mutex );
#endif
}

//---------------------------------------------------------------------------------------------------

void Mutex::initialize()
{
	release();

#if defined(_WIN32) || defined(_XBOX)

#if USE_CRITICAL_SECTION
	CRITICAL_SECTION * pSection = new CRITICAL_SECTION();
	memset( pSection, 0, sizeof( CRITICAL_SECTION ) );
    InitializeCriticalSection( pSection );

	m_pMutex = pSection;
#else
	HANDLE hMutex = CreateMutex( NULL, FALSE, m_pName );
	m_pMutex = hMutex;
#endif

#else
	m_pMutex = (void *)new PTMutex();
#endif
}

void Mutex::release()
{
#if defined(_WIN32) || defined(_XBOX)

#if USE_CRITICAL_SECTION
	if ( m_pMutex != NULL )
	{
		DeleteCriticalSection( (CRITICAL_SECTION *)m_pMutex );
		delete (CRITICAL_SECTION *)m_pMutex;
		m_pMutex = NULL;
	}
#else
	if ( m_pMutex != NULL )
	{
		CloseHandle( (HANDLE)m_pMutex );
		m_pMutex = NULL;
	}
#endif

#else
	if ( m_pMutex != NULL )
	{
		delete (PTMutex *)m_pMutex;
		m_pMutex = NULL;
	}
#endif
}

//-------------------------------------------------------------------------------
// EOF

