/*
	WorldTimer.h

	This clas is used to keep the current tick for the game, it spawns it's own thread to count the ticks and can take adjustments
	through the interface

	(c)2000 Palestar Inc, Richard Lyle
*/

#ifndef GAMETIMER_H
#define GAMETIMER_H

#include "Debug/SafeThread.h"
#include "WorldDll.h"

//----------------------------------------------------------------------------

class DLL WorldTimer
{
public:
	// Construction
					WorldTimer();
	virtual			~WorldTimer();

	// Accessors
	bool			active() const;			// is the timer running
	dword			tick() const;			// get the current tick
	operator		dword() const;			// operator to get the current tick
	int				offset() const;			// get the current tick adjustment offset

	// Mutators
	void			start();				// start the timer
	void			stop();					// stop the timer
	void			setTick( dword tick );	// set the tick to an absolute value
	void			cron( dword tick );		// syncronize to this tick over time

	int				timerDemon();

private:
	// Types
	class Timer : public SafeThread
	{
	public:
		// Data
		WorldTimer *		m_pTimer;
		// Construction
		Timer( WorldTimer * pTimer ) 
			: m_pTimer( pTimer ), SafeThread( HIGH )
		{}
		// Thread interface
		int	run()
		{
			return( m_pTimer->timerDemon() );
		}
	};

	// Data
	volatile bool	m_Active;
	volatile dword	m_Tick;		
	volatile int	m_TickOffset;
	Timer *			m_pTimer;
};

//----------------------------------------------------------------------------

inline bool WorldTimer::active() const
{
	return m_Active;
}

inline dword WorldTimer::tick() const
{
	return m_Tick;
}

inline WorldTimer::operator dword() const
{
	return m_Tick;
}

inline int WorldTimer::offset() const
{
	return m_TickOffset;
}

//----------------------------------------------------------------------------

inline bool checkTrigger( dword tick, dword elapsed, dword interval )
{
	return elapsed >= interval || ((tick + elapsed) % interval) < (tick % interval);
}

//----------------------------------------------------------------------------



#endif

//----------------------------------------------------------------------------
//EOF
