/*
	WorldTimer.cpp
	(c)2000 Palestar Inc, Richard Lyle
*/

#include "Debug/Log.h"
#include "Debug/Assert.h"
#include "Standard/Timer.h"
#include "Constants.h"
#include "WorldTimer.h"

//----------------------------------------------------------------------------

const int MAX_TICK_OFFSET = TICKS_PER_SECOND * 10;

//----------------------------------------------------------------------------

WorldTimer::WorldTimer() : m_Active( false ), m_Tick( 0 ), m_TickOffset( 0 ), m_pTimer( NULL )
{}

WorldTimer::~WorldTimer()
{
	// stop the timer thread
	stop();
}

//----------------------------------------------------------------------------

void WorldTimer::start()
{
	if (! m_Active )
	{
		m_Active = true;
		m_pTimer = new Timer( this );
		m_pTimer->resume();
	}
}

void WorldTimer::stop()
{
	if ( m_Active )
	{
		m_Active = false;

		delete m_pTimer;
		m_pTimer = NULL;
	}
}

void WorldTimer::setTick( dword tick )
{
	LOG_STATUS( "WorldTimer", "Setting tick to %u", tick );
	m_Tick = tick;
	m_TickOffset = 0;
}

void WorldTimer::cron( dword nTick )
{
	int nDiff = nTick - m_Tick;
	m_TickOffset = Clamp( nDiff, -MAX_TICK_OFFSET, MAX_TICK_OFFSET );
}

//----------------------------------------------------------------------------

int	WorldTimer::timerDemon()
{
	::Timer tick( TICK_DURATION_MS );

	dword adjust = 0;
	while( active() )
	{
		tick.wait();

		// advance our cron tick, we cannot use m_Tick because the code below might adjust m_Tick making
		// the timing adjustments happen all at once which defeats it's purpose of hiding the timer sycronization
		if ( m_TickOffset != 0 && ((++adjust) % TICKS_PER_SECOND) == 0)
		{
			if ( m_TickOffset > 0 )
			{
				// client is running beind server, adjust upwards
				m_Tick += 2;
				--m_TickOffset;
				continue;
			}
			else if ( m_TickOffset < 0 )
			{
				// client is running ahead, skip incrementing m_Tick this time
				--m_TickOffset;		
				continue;
			}
		}

		++m_Tick;
	}

	return 0;
}


//----------------------------------------------------------------------------
//EOF
