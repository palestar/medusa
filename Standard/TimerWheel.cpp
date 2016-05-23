/**
	@file TimerWheel.cpp

	(c)2009 Palestar Inc
	@author Richard Lyle @date 8/20/2010 8:20:58 PM
*/

#include "TimerWheel.h"
#include "Time.h"

#include "Debug/Assert.h"
#include "Debug/Trace.h"

//---------------------------------------------------------------------------------------------------

TimerWheel::Timer::Timer( unsigned int a_nInterval /*= 1000*/, bool a_bLocked /*= false*/ ) :
	m_bActive( false ),
	m_nInterval( a_nInterval ),
	m_bLocked( a_bLocked ),
	m_nNextTime( 0 )
{}

TimerWheel::Timer::~Timer()
{
	if ( m_bActive )
	{
		if (! TimerWheel::stop( this ) )
			ASSERT( 0 && "Failed to stop timer that is getting destoyed!" );
	}
}

void TimerWheel::Timer::setInterval( unsigned int a_nMS )
{
	if ( m_nInterval != a_nMS )
	{
		m_nInterval = a_nMS;
		if ( m_bActive && !TimerWheel::start( this ) )
			ASSERT( 0 && "Failed to restart timer!" );
	}
}

void TimerWheel::Timer::setLocked( bool a_bLocked )
{
	m_bLocked = a_bLocked;
}

//---------------------------------------------------------------------------------------------------

bool TimerWheel::start( Timer * a_pTimer )
{
	if ( a_pTimer == NULL )
		return false;		// NULL timer or it's already active..
	if ( a_pTimer->m_bActive )
	{
		if (! stop( a_pTimer ) )	// stop if already active..
			return false;
	}

	TimerMap & map = GetTimerMap();

	a_pTimer->m_bActive = true;
	a_pTimer->m_nNextTime = Time::milliseconds() + a_pTimer->m_nInterval;

	// add timer to our map ..
	map[ a_pTimer->m_nNextTime ].push_back( a_pTimer );

	return true;
}

bool TimerWheel::stop( Timer * a_pTimer )
{
	if ( a_pTimer == NULL || !a_pTimer->m_bActive )
		return false;

	TimerMap & map = GetTimerMap();
	
	TimerList::iterator & iUpdate = GetUpdateTimer();
	TimerList & timers = map[ a_pTimer->m_nNextTime ];
	for( TimerList::iterator iTimer = timers.begin(); iTimer != timers.end(); ++iTimer )
	{
		if ( *iTimer == a_pTimer )
		{
			if ( iTimer == iUpdate )
				++iUpdate;

			a_pTimer->m_bActive = false;

			timers.erase( iTimer );
			return true;
		}
	}

	return false;
}

bool TimerWheel::update( unsigned int a_nTimeout /*= 0*/ )
{
	TimerMap & map = GetTimerMap();
	if ( map.size() == 0 )
		return true;

	dword nTime = Time::milliseconds();
	dword nTimeout = a_nTimeout != 0 ? nTime + a_nTimeout : 0;

	for( TimerMap::iterator iTimerList = map.begin(); iTimerList != map.end(); )
	{
		if ( iTimerList->first > nTime )
			break;		// done with map, break out of for loop..

		TimerList & timers = iTimerList->second;
		if ( timers.size() > 0 )
		{
			TimerList::iterator & iUpdate = GetUpdateTimer();

			iUpdate = timers.begin();
			while( iUpdate != timers.end() )
			{
				Timer * pTimer = *GetUpdateTimer();
				timers.erase( iUpdate++ );

				// timer will be fired at this point, move it to it's new location in the linked
				// list before we call onFire()..
				if ( pTimer->m_bLocked )
					pTimer->m_nNextTime = pTimer->m_nNextTime + pTimer->m_nInterval;
				else
					pTimer->m_nNextTime = nTime + pTimer->m_nInterval;
				map[ pTimer->m_nNextTime ].push_back( pTimer );

				pTimer->onFire();
			}
		}

		map.erase( iTimerList++ );

		// check if we hit our timeout..
		if ( nTimeout != 0 && Time::milliseconds() > nTimeout )
			return false;
	}

	return true;
}

//---------------------------------------------------------------------------------------------------

TimerWheel::TimerMap & TimerWheel::GetTimerMap()
{
	static TimerMap MAP;
	return MAP;
}

TimerWheel::TimerList::iterator & TimerWheel::GetUpdateTimer()
{
	static TimerList::iterator i;
	return i;
}

//---------------------------------------------------------------------------------------------------
//EOF
