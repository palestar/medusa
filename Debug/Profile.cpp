/*
	Profile.cpp

	Profile code
	(c)2005 Palestar, Richard Lyle
*/

#define DEBUG_DLL
#include "Debug/Profile.h"
#include "Debug/Assert.h"
#include "Standard/Exception.h"
#include "Standard/Tree.h"
#include "Standard/Time.h"
#include "Standard/Thread.h"

#include <stdio.h>

//----------------------------------------------------------------------------

const int MAX_TIMER_DEPTH = 128;

//------------------------------------------------------------------------------

CriticalSection				Profiler::sm_Lock;
Profiler::ThreadHash		Profiler::sm_ThreadHash;
Profiler::MessageHash		Profiler::sm_MessageHash;
Profiler::StackHash			Profiler::sm_StackHash;
dword						Profiler::sm_nLastUpdate = 0;
qword						Profiler::sm_nLastCPU = 0;
qword						Profiler::sm_nTotalCPU = 0;
qword						Profiler::sm_nPeakCPU = 0;
float						Profiler::sm_fLoad = 1.0f;

//----------------------------------------------------------------------------

// begin profiling
void Profiler::start( const char * pName )
{
	AutoLock lock( &sm_Lock );

	Array< Timer > & timers = sm_StackHash[ Thread::getCurrentThreadId() ];

	// push a timer structure into the stack
	Timer & timer = timers.push();
	timer.pName = pName;
	timer.nTime = Time::CPU();
	timer.nBytes = 0; //Heap::bytes();

	ERROR_ON( timers.size() > MAX_TIMER_DEPTH );		// throws exception if someone forgets to call Profiler::end()
}

void Profiler::end()
{
	AutoLock lock( &sm_Lock );

	dword nThread = Thread::getCurrentThreadId();

	Array< Timer > & timers = sm_StackHash[ nThread ];
	if ( timers.size() > 0 )
	{
		Timer & timer = timers[ timers.size() - 1 ];

		// add/update the Profile structure
		Profile & profile = sm_ThreadHash.insert( nThread ).insert( ((dword)timer.pName) );
		profile.pName = timer.pName;
		profile.nCPU += Time::CPU() - timer.nTime;
		//profile.nBytes += Heap::bytes() - timer.nBytes;
		profile.nHits += 1;

		// remove the timing section from the stack
		timers.pop();
	}

	updateProfiles();
}

void Profiler::stop()
{
	AutoLock lock( &sm_Lock );

	dword nThread = Thread::getCurrentThreadId();
	sm_ThreadHash.removeByKey( nThread );
	sm_StackHash.remove( nThread );
}

void Profiler::message( dword nLine, const char * pMessage )
{
	AutoLock lock( &sm_Lock );
	sm_MessageHash.insert( nLine, CharString( pMessage ) );
}

void Profiler::removeMessage( dword nLine )
{
	AutoLock lock( &sm_Lock );
	sm_MessageHash.removeByKey( nLine );
}

void Profiler::clearMessages()
{
	AutoLock lock( &sm_Lock );
	sm_MessageHash.release();
}

//----------------------------------------------------------------------------

void Profiler::lock( bool bLock )
{
	if ( bLock )
		sm_Lock.lock();
	else
		sm_Lock.unlock();
}

int Profiler::threadCount()
{
	return sm_ThreadHash.size();
}

dword Profiler::thread( int n )
{
	return sm_ThreadHash.key( n );
}

int Profiler::profileCount( dword nThread )
{
	return sm_ThreadHash.hash()[ nThread ].size();
}

Profiler::Profile & Profiler::profile( dword nThread, int n )
{
	return sm_ThreadHash.hash()[ nThread ][ n ];
}

int Profiler::messageCount()
{
	return sm_MessageHash.size();
}

const char * Profiler::message( int n )
{
	return sm_MessageHash[ n ];
}

qword Profiler::totalCPU()
{
	return sm_nTotalCPU;
}

qword Profiler::peakCPU()
{
	return sm_nPeakCPU;
}

float Profiler::CPUused()
{
	return sm_fLoad;
}

//----------------------------------------------------------------------------

void Profiler::updateProfiles()
{
	dword nClock = Time::milliseconds();
	dword nElapsed = nClock - sm_nLastUpdate;
	if ( nElapsed >= 1000 )
	{
		qword nCPU = Time::CPU();

		sm_nPeakCPU = 0;
		for(int i=0;i<sm_ThreadHash.size();i++)
		{
			dword nThread = sm_ThreadHash.key( i );

			// update any currently open timers for this thread, this allows
			// us to detect dead-locks in the code using the profiler...
			Array< Timer > & timers = sm_StackHash[ nThread ];
			for(int k=0;k<timers.size();k++)
			{
				Timer & timer = timers[ k ];

				Profile & profile = sm_ThreadHash.insert( nThread ).insert( ((dword)timer.pName) );
				profile.pName = timer.pName;
				profile.nCPU += Time::CPU() - timer.nTime;
				if ( profile.nCPU > sm_nPeakCPU )
					sm_nPeakCPU = profile.nCPU;
				//profile.nBytes += Heap::bytes() - timer.nBytes;
				profile.nHits += 1;

				// update the timer time
				timer.nTime = nCPU;
				//timer.nBytes = Heap::bytes();
			}

			ProfileHash & profiles = sm_ThreadHash[ i ];
			for(int j=0;j<profiles.size();j++)
			{
				Profile & profile = profiles[ j ];

				profile.nAvCPU = (profile.nCPU * 1000) / nElapsed;
				profile.nCPU = 0;
				profile.nAvHits = (profile.nHits * 1000) / nElapsed;
				profile.nHits = 0;
			}
		}

		qword nCPUElapsed = nCPU - sm_nLastCPU;
		sm_nTotalCPU = (nCPUElapsed * 1000) / nElapsed;
		sm_nLastCPU = nCPU;
		sm_nLastUpdate = nClock;
		sm_fLoad = Clamp<float>( ((float)sm_nPeakCPU) / ((float)sm_nTotalCPU), 0.0f, 1.0f );
	}
}

//-------------------------------------------------------------------------------
// EOF
