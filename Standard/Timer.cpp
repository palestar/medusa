/*
	Timer.cpp

	This object is used to block a thread until a specified amount of time has passed
	(c)2005 Palestar, Richard Lyle
*/

#include "Standard/Time.h"
#include "Standard/Timer.h"
#include "Standard/Thread.h"

//----------------------------------------------------------------------------

Timer::Timer() : m_nGoal( 0 ), m_nDuration( 1000 ), m_bLocked( true )
{}

Timer::Timer( dword nDuration ) : m_nGoal( 0 ), m_nDuration( nDuration ), m_bLocked( true )
{}

Timer::Timer( dword nDuration, bool bLocked ) : m_nGoal( 0 ), m_nDuration( nDuration ), m_bLocked( bLocked )
{}

//----------------------------------------------------------------------------

void Timer::start()
{
	m_nGoal = Time::milliseconds() + m_nDuration;
}

void Timer::wait()
{
	dword nTime = Time::milliseconds();
	if ( m_nGoal == 0 )
		m_nGoal = nTime + m_nDuration;

	int nSleep = m_nGoal - nTime;
	if ( nSleep > 0 )
		Thread::sleep( nSleep );

	if ( m_bLocked )
		m_nGoal += m_nDuration;
	else
		m_nGoal = nTime + m_nDuration;
}

bool Timer::signaled()
{
	dword nTime = Time::milliseconds();
	if ( m_nGoal == 0 )
		m_nGoal = nTime + m_nDuration;
	if ( m_nGoal > nTime )
		return false;

	if ( m_bLocked )
		m_nGoal += m_nDuration;
	else
		m_nGoal = nTime + m_nDuration;
	return true;
}

//----------------------------------------------------------------------------
// EOF
