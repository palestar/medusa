/**
	@file EventList.h
	@brief This class is used to wait on multiple Event objects.

	(c)2013 Palestar Inc
	@author Richard Lyle @date 1/10/2013 9:10:05 PM
*/

#ifndef EVENTLIST_H
#define EVENTLIST_H

//---------------------------------------------------------------------------------------------------

class DLL EventList
{
public:
	//! Types
	struct EventInfo {
		Event *				m_pEvent;
		void *				m_nUserData;
	};

	//! Construction
	EventList();
	~EventList();

	//! Accessors
	int			eventInfoCount() const;
	const EventInfo & 
				eventInfo( int i ) const;

	//! Mutators
	bool		addEvent( Event * a_pEvent, void * a_pUserData = NULL );
	bool		removeEvent( Event * a_pEvent );

	//! Wait for an event to be signaled, returns true if an event was signaled and returns the signaled event info in the given EventInfo object.
	//! If timeout occurs, then false is returned. Pass a timeout of 0 for no-wait.
	bool		waitForEvent( EventInfo & a_Info, dword a_nTimeout = 0xffffffff );
protected:

};

#endif

//---------------------------------------------------------------------------------------------------
//EOF
