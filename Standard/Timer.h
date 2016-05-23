/*
	Timer.h

	This object is used to block a thread until a specified amount of time has passed. 

	If a timer is unlocked, then the next goal time will be the last time signaled() / wait() was
	called plus the duration of the timer. An unlocked timer is useful for loose time keeping.

	A locked timer, always increments the goal time by the duration and will not use the current time to calculate
	the next goal time. You want a locked timer if you need something called exactly N times per second. 

	(c)2005 Palestar, Richard Lyle
*/

#ifndef TIMER_H
#define TIMER_H

#include "Standard/Types.h"
#include "MedusaDll.h"

//----------------------------------------------------------------------------

class DLL Timer
{
public:
	// Construction
	Timer();										// defaults to a duration of 1000 ms and locked 
	Timer( dword duration );						// defaults to locked
	Timer( dword duration, bool bLocked );	

	// Accessors
	dword		duration() const;					// get the duration in MS
	dword		goal() const;						// uses Core::timeClock() to check against
	bool		locked() const;						// is the timer locked or not

	// Mutators
	void		setDuration( dword nDuration );		// set the duration in MS
	void		setLocked( bool bLocked );

	void		start();							// start the timer now, sets the goal time to the current time + duration
	void		wait();								// blocks the calling thread until the timer is triggered
	bool		signaled();							// poll the timer without blocking, returns true if timer is triggered

private:
	dword		m_nGoal;
	dword		m_nDuration;
	bool		m_bLocked;		
};

//----------------------------------------------------------------------------

inline dword Timer::duration() const
{
	return m_nDuration;
}

inline dword Timer::goal() const
{
	return m_nGoal;
}

inline bool Timer::locked() const
{
	return m_bLocked;
}

//----------------------------------------------------------------------------

inline void Timer::setDuration( dword nDuration )
{
	m_nDuration = nDuration;
}

inline void Timer::setLocked( bool bLocked )
{
	m_bLocked = bLocked;
}

//----------------------------------------------------------------------------



#endif

//----------------------------------------------------------------------------
// EOF
