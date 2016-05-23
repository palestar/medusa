/*
	CriticalSection.h

	Used for locked global data when using threads...This is simular to a Mutex object, but faster on Windows since
	there is no queue for what thread will get access to the data next. You would usually want to use this if you only
	have two threads locking this section... if you have more, then use a Mutex.

	(c)2005 Palestar, Richard Lyle
*/

#ifndef CRITICAL_SECTION_H
#define CRITICAL_SECTION_H

#include "AutoLock.h"
#include "MedusaDll.h"
#include <stdlib.h>		// NULL

//----------------------------------------------------------------------------

class DLL CriticalSection : public AbstractLock
{
public:
	// Exceptions
	class LockTimeout {};

	// Construction
	CriticalSection();
	CriticalSection( const CriticalSection & a_Copy );
	~CriticalSection();

	// AbstractLock interface
	virtual void	lock();
	virtual void	unlock();

	// Mutators
	void			initialize();
	void			release();

	CriticalSection &
					operator=( const CriticalSection & a_Copy );

private:
	void *			m_pCriticalSection;
	bool			m_bOwner;
};

#endif

//----------------------------------------------------------------------------
// EOF
