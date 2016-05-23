/*
	TestProfiler.cpp
	(c)2005 Palestar Inc, Richard Lyle
*/

//----------------------------------------------------------------------------

#pragma warning( disable : 4146 )

#include "UnitTest.h"
#include "Debug/Profile.h"
#include "Debug/SafeThread.h"

//----------------------------------------------------------------------------

static bool bRunProfilerThreads = true;

class ProfilerThread : public SafeThread
{
public:
	int run()
	{
		int x = 0;
		while( bRunProfilerThreads )
		{
			PROFILE_START( "ProfilerThread::run()" );
			x += 1;
			PROFILE_END();
		}

		return 0;
	}
};

//----------------------------------------------------------------------------

const int PROFILER_THREAD_COUNT = 4;
const int PROFILER_TEST_TIME = 1;			// number of seconds to test

//----------------------------------------------------------------------------

class TestProfiler : public UnitTest
{
public:
	TestProfiler( const char * pName ) : UnitTest( pName )
	{}

	void test()
	{
		int i;

		// start up several threads, each calling into the profiler..
		ProfilerThread * pThreads[ PROFILER_THREAD_COUNT ];
		for(i=0;i<PROFILER_THREAD_COUNT;i++)
		{
			pThreads[ i ] = new ProfilerThread();
			pThreads[ i ]->resume();
		}


		Thread::sleep( 1000 * PROFILER_TEST_TIME );

		Profiler::lock( true );
		for(int j=0;j<Profiler::threadCount();j++)
		{
			dword nThreadId = Profiler::thread( j );
			for(int k=0;k<Profiler::profileCount( nThreadId );k++)
			{
				Profiler::Profile & profile = Profiler::profile( nThreadId, k );
				printf( "%u: %s, CPU: %s, Hits: %s, AvCPU: %s, AvHits: %s, Memory: %s\n", nThreadId, profile.pName,
					FormatNumber<char,qword>( profile.nCPU ),
					FormatNumber<char,dword>( profile.nHits ),
					FormatNumber<char,qword>( profile.nAvCPU ),
					FormatNumber<char,dword>( profile.nAvHits ),
					FormatNumber<char,int>( profile.nBytes) );
			}
		}
		Profiler::lock( false );

		bRunProfilerThreads = false;
		for(i=0;i<PROFILER_THREAD_COUNT;i++)
			delete pThreads[ i ];
	}

} TEST_BLANK( "TestProfiler" );

//----------------------------------------------------------------------------
//EOF


//----------------------------------------------------------------------------
//EOF
