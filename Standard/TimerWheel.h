/**
	@file TimerWheel.h
	@brief This class provides a interface for multiple objects to get updated efficiently from a single
	system. 

	(c)2009 Palestar Inc
	@author Richard Lyle @date 8/20/2010 8:20:42 PM
*/

#ifndef TIMERWHEEL_H
#define TIMERWHEEL_H

#include <list>
#include <map>

#include "MedusaDll.h"

//---------------------------------------------------------------------------------------------------

class DLL TimerWheel 
{
public:
	//! This Timer object allows a user to hook into the TimerWheel by inheriting from this interface
	//! and adding this object to the TimerWheel for updates.
	class DLL Timer
	{
	public:
		Timer( unsigned int a_nInterval = 1000, 
			bool a_bLocked = false );
		virtual ~Timer();

		//! Accessors
		unsigned int			interval() const;			// interval of this timer in MS
		bool					locked() const;				// true if timer is locked, locked timers will catch up if they get behind

		//! Mutators
		void					setInterval( unsigned int a_nMS );
		void					setLocked( bool a_bLocked );

		//! Interface
		virtual void			onFire() = 0;

	protected:
		bool					m_bActive;
		unsigned int			m_nInterval;
		bool					m_bLocked;
		unsigned int			m_nNextTime;

		friend class TimerWheel;
	};

	static bool					start( Timer * a_pTimer );
	static bool					stop( Timer * a_pTimer );
	static bool					update( unsigned int a_nTimeout = 0 );		// update all timers, returns false if the timeout occured before all timers could be updated.

protected:
	//! Types
	typedef std::list< Timer * >					TimerList;
	typedef std::map< unsigned int, TimerList >		TimerMap;

	//! Data
	static TimerMap	&			GetTimerMap();
	static TimerList::iterator&	GetUpdateTimer();
};

//---------------------------------------------------------------------------------------------------

inline unsigned int TimerWheel::Timer::interval() const
{
	return m_nInterval;
}

inline bool TimerWheel::Timer::locked() const
{
	return m_bLocked;
}

#endif

//---------------------------------------------------------------------------------------------------
//EOF
