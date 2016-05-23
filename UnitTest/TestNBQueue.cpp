/**
	@file TestNBQueue.cpp
	
	(c)2012 Palestar Inc
	@author Richard Lyle @date 8/23/2012 5:31:18 PM
*/

#include "UnitTest.h"
#include "Debug/SafeThread.h"
#include "Standard/NBQueue.h"
#include "Standard/Time.h"

#define TEST_THREAD_COUNT	10
#define TEST_SECONDS		30

static volatile int nCounter = 0;

//---------------------------------------------------------------------------------------------------

class TestNBQueue;

class TestNBQThread : public SafeThread
{
public:
	TestNBQThread( int a_nThreadId, NBQueue<int> * a_pQueue ) : 
		m_nThreadId( a_nThreadId ), 
		m_pStack( a_pQueue ), 
		m_bActive( true )
	{
		// actually start our thread, this will call run() ..
		resume();
	}

	//! SafeThread interface
	virtual int run()
	{
		while( m_bActive )
		{
			if ( (rand() % 100) < 50 )
			{
				int nPopped = 0;
				if ( m_pStack->dequeue( nPopped ) )
				{
					//printf( "Thread %d popped value %d\n", m_nThreadId, nPopped );
				}
				else
					Thread::sleep( 0 );
			}
			else
			{
				m_pStack->enqueue( Atomic::increment( &nCounter ) );
			}
		}

		return 0;
	}

	int					m_nThreadId;
	NBQueue<int> *		m_pStack;
	volatile bool		m_bActive;
};

//----------------------------------------------------------------------------

class TestNBQueue : public UnitTest
{
public:
	TestNBQueue( const char * pName ) : UnitTest( pName )
	{}

	void test()
	{
		std::list< TestNBQThread * > threads;
		for(int i=0;i<TEST_THREAD_COUNT;++i)
			threads.push_back( new TestNBQThread( i, &m_TestQueue ) );

		dword nStartTime = Time::seconds();
		while( (Time::seconds() - nStartTime) < TEST_SECONDS )
			Thread::sleep( 1 );

		for( std::list< TestNBQThread *>::iterator iThread = threads.begin();
			iThread != threads.end(); ++iThread )
		{
			TestNBQThread * pThread = *iThread;
			pThread->m_bActive = false;

			delete pThread;
		}
	}

	NBQueue<int>		m_TestQueue;

} TEST_CLIENTSERVER( "TestNBQueue" );

//---------------------------------------------------------------------------------------------------
//EOF
