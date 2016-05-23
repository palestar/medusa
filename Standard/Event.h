/*
	Event.h

	Event object, used to syncronize mutliple threads
	(c)2005 Palestar, Richard Lyle
*/

#ifndef EVENT_H
#define EVENT_H

#include "Standard/Types.h"
#include "Standard/Exception.h"
#include "MedusaDll.h"

//----------------------------------------------------------------------------

class DLL Event 
{
public:
	// Exceptions
	DECLARE_EXCEPTION(FailedToCreateEvent);

	// Construction
	Event();
	Event( bool manualReset );
	Event( const char * pName );		// creates a named event that can be inherited
	Event( const Event & copy );		// copy another event handle
	virtual ~Event();

	// Accessors
	void			wait() const;			// wait for event to be signaled
	bool			wait( dword timeout );	// wait for signal, or timeout, returns true if event timed out
	bool			signaled() const;		// is event signaled
	void *			event() const;			// get the event handle

	// Mutators
	void			clear();				// set the event to unsignaled
	void			signal();				// set the event to signaled

	Event &			operator=( const Event & copy );
private:
	// Data
	void *			m_hEvent;			// event object

	// Static
	static void *	eventCreate( bool bManualReset = false, const char * pName = NULL );
	static void *	eventDuplicate( void * pEvent );
	static bool		eventWait( void * pEvent, dword nTimeOut = 0xffffffff );
	static bool		eventSignaled( void * pEvent );
	static void		eventClear( void * pEvent );
	static void		eventSignal( void * pEvent );
	static void		eventClose( void * pEvent );
};

//----------------------------------------------------------------------------

inline void * Event::event() const
{
	return m_hEvent;	
}

//----------------------------------------------------------------------------



#endif

//-------------------------------------------------------------------------------
// EOF
