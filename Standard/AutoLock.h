/**
	@file AutoLock.h
	@brief This object is used to automatically lock and unlock a synchronization object such as the Mutex or CriticalSection classes.

	(c)2006 Palestar Inc
	@author Richard Lyle @date 10/10/2008 10:41:43 PM
*/

#ifndef AUTOLOCK_H
#define AUTOLOCK_H

#include "MedusaDll.h"
#include <stdlib.h>

//---------------------------------------------------------------------------------------------------

class AbstractLock
{
public:
	virtual void		lock() = 0;
	virtual void		unlock() = 0;
};

//---------------------------------------------------------------------------------------------------

// Types
class DLL AutoLock
{
public:
	// Construction
	AutoLock( AbstractLock * pLock ) : m_pLock( NULL )
	{
		// lock the semaphore on construction
		set( pLock );
	}
	AutoLock( const AutoLock & copy ) : m_pLock( NULL )
	{
		// lock the semaphore on construction
		set( copy.m_pLock );
	}
	~AutoLock()
	{
		// unlock on destruction
		release();
	}

	void set( AbstractLock * a_pLock )
	{
		if ( m_pLock != NULL )
			m_pLock->unlock();
		m_pLock = a_pLock;
		if ( m_pLock )
			m_pLock->lock();
	}
	void release()
	{
		if ( m_pLock != NULL )
		{
			m_pLock->unlock();
			m_pLock = NULL;
		}
	}

private:
	// Data
	AbstractLock *			m_pLock;
};


#endif

//---------------------------------------------------------------------------------------------------
//EOF
