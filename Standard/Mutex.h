/**
	@file Mutex.h
	@brief Synchronization class for locking a shared asset.

	(c)2006 Palestar Inc
	@author Richard Lyle @date 10/9/2008 8:01:30 AM
*/

#ifndef MUTEX_H
#define MUTEX_H

#include "AutoLock.h"
#include "MedusaDll.h"
#include <stdlib.h>		// NULL

//----------------------------------------------------------------------------

class DLL Mutex : public AbstractLock
{
public:
	// Exceptions
	class LockTimeout {};

	// Construction
	Mutex( const char * a_pName = NULL );
	~Mutex();

	// AbstractLock interface
	virtual void	lock();
	virtual void	unlock();

	// Mutators
	void			initialize();
	void			release();

private:
	const char *	m_pName;
	void *			m_pMutex;
};

//----------------------------------------------------------------------------


#endif

//---------------------------------------------------------------------------------------------------
//EOF
