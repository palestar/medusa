/*
	Thread.h

	Thread Class
	(c)2005 Palestar, Richard Lyle
*/

#ifndef THREAD_H
#define THREAD_H

#include "Reflection/Value.h"
#include "Standard/Types.h"
#include "Standard/Array.h"
#include "Standard/Exception.h"
#include "Standard/CriticalSection.h"
#include "MedusaDll.h"

#pragma warning( disable: 4251 )		// dll-interface warning for template classes

//----------------------------------------------------------------------------

class DLL Thread
{
public:
	// Types
	enum {
		DEFAULT_STACK_SIZE = 4096,
	};

	enum Priority {
		LOW,				// lowest priority
		STANDARD,
		HIGH,
		CRITICAL,			// highest priority
	};

	// Exceptions
	DECLARE_EXCEPTION( FailedToCreateThread );
	DECLARE_EXCEPTION( BadThreadHandle );

	// Construction
	Thread( Priority p = STANDARD, int stack = DEFAULT_STACK_SIZE );
	Thread( unsigned int a_nID, void * a_pHandle );
	virtual	~Thread();

	// accessors
	bool				suspended() const;			// is this thread currently suspended
	Priority			priority() const;			// what is the thread priority
	bool				inThread() const;			// is the caller of this function running in this threads context
	int					exitCode() const;

	void *				handle() const;				// get the internal thread handle
	dword				id() const;					// get the thread id


	// mutators
	bool				suspend();					// suspend the thread
	bool				resume();					// resume the thread
	bool				kill();						// kill the thread
	bool				wait(int a_nTimeout = -1 );	// wait for thread to exit, returns false on timeout

	//! Thread storage interface
	dword				allocateTS( Value a_Value );
	Value *				findTS( dword a_nId );		// returns pointer to Value, or NULL 
	void				freeTS( dword a_nId );

	// Interface
	virtual int			runProxy();				
	virtual int			run();		

	// helpers
	static void			sleep( int ms );			// sleep for specified amount of time
	static void			threadLock( bool bLock );	// lock the thread static data
	static int			threadCount();				// get the total number of active thread objects
	static Thread *		thread( int n );			// get thread n

	static Thread *		getCurrentThread();			// 
	static void *		getCurrentThreadHandle();	// get the calling thread handle
	static dword		getCurrentThreadId();		// get the calling thread id

private:
	//! Types
	typedef std::map< dword, Thread * >		ThreadMap;
	typedef std::map< dword, Value >		ThreadStorage;

	// data
	bool				m_bSuspended;
	Priority			m_Priority;
	int					m_StackSize;

	void *				m_pHandle;						// thread handle
	unsigned long		m_Id;							// thread id
	int					m_nExitCode;					// thread exit code
	bool				m_bAutoDelete;					// if true, then thread object is destroyed when thread exits
	
	CriticalSection		m_StorageLock;
	ThreadStorage		m_Storage;
	dword				m_nNextStorageId;

	// static
	static int			threadMain( void *argument );

	static CriticalSection
						sm_Lock;					// semaphore for locking the thread list
	static ThreadMap	sm_Threads;					// static list of all thread objects
};

//----------------------------------------------------------------------------

inline bool Thread::suspended() const
{ 
	return m_bSuspended;
}

inline Thread::Priority Thread::priority() const
{ 
	return m_Priority; 
}

inline bool Thread::inThread() const
{
	return getCurrentThreadId() == m_Id;
}

inline void * Thread::handle() const
{
	return m_pHandle;
}

inline dword Thread::id() const
{
	return m_Id;
}

//----------------------------------------------------------------------------

#endif

//----------------------------------------------------------------------------
// EOF
