/**
	@file CriticalSection.cpp

	(c)2006 Palestar Inc
	@author Richard Lyle @date 10/10/2008 10:04:26 PM
*/

#include "CriticalSection.h"

#if defined(_WIN32)
#include <windows.h>
#else
#include <pthread.h>

struct PTCriticalSection 
{
	PTCriticalSection() 
	{
		pthread_mutexattr_init( &m_Attributes );
		pthread_mutexattr_settype( &m_Attributes, PTHREAD_MUTEX_RECURSIVE );
		pthread_mutex_init( &m_Mutex, &m_Attributes );
	}
	~PTCriticalSection()
	{
		pthread_mutex_destroy( &m_Mutex );
		pthread_mutexattr_destroy( &m_Attributes );
	}

	pthread_mutex_t		m_Mutex;
	pthread_mutexattr_t m_Attributes;
};

#endif

//-------------------------------------------------------------------------------

CriticalSection::CriticalSection() 
	: m_pCriticalSection( NULL ),
	m_bOwner( false )
{
	initialize();
}

CriticalSection::CriticalSection( const CriticalSection & a_Copy ) :
	m_pCriticalSection( a_Copy.m_pCriticalSection ),
	m_bOwner( false )
{}

CriticalSection::~CriticalSection()
{
	release();
}

//---------------------------------------------------------------------------------------------------

void CriticalSection::lock()
{
#if defined(_WIN32) || defined(_XBOX)
	if ( m_pCriticalSection != NULL )
		EnterCriticalSection( (CRITICAL_SECTION *)m_pCriticalSection );
#else
	if ( m_pCriticalSection != NULL )
		pthread_mutex_lock( &((PTCriticalSection *)m_pCriticalSection)->m_Mutex );
#endif
}

void CriticalSection::unlock()
{
#if defined(_WIN32) || defined(_XBOX)
	if ( m_pCriticalSection != NULL )
		LeaveCriticalSection( (CRITICAL_SECTION *)m_pCriticalSection );
#else
	if ( m_pCriticalSection != NULL )
		pthread_mutex_unlock( &((PTCriticalSection *)m_pCriticalSection)->m_Mutex );
#endif
}

//-------------------------------------------------------------------------------

void CriticalSection::initialize()
{
	release();

#if defined(_WIN32) || defined(_XBOX)

	CRITICAL_SECTION * pSection = new CRITICAL_SECTION();
	memset( pSection, 0, sizeof( CRITICAL_SECTION ) );
    InitializeCriticalSection( pSection );

	m_pCriticalSection = pSection;
	m_bOwner = true;
#else
	m_pCriticalSection = (void *)new PTCriticalSection();
	m_bOwner = true;
#endif
}

void CriticalSection::release()
{
#if defined(_WIN32) || defined(_XBOX)
	if ( m_pCriticalSection != NULL )
	{
		if ( m_bOwner )
		{
			DeleteCriticalSection( (CRITICAL_SECTION *)m_pCriticalSection );
			delete (CRITICAL_SECTION *)m_pCriticalSection;
		}
		m_pCriticalSection = NULL;
	}
#else
	if ( m_pCriticalSection != NULL )
	{
		if ( m_bOwner )
			delete ((PTCriticalSection *)m_pCriticalSection);
		m_pCriticalSection = NULL;
	}
#endif
}

CriticalSection & CriticalSection::operator=( const CriticalSection & a_Copy )
{
	release();

	m_pCriticalSection = a_Copy.m_pCriticalSection;
	m_bOwner = false;
	return *this;
}

//---------------------------------------------------------------------------------------------------
//EOF
