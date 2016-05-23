/*
	TestExceptionHandler.cpp
	(c)2005 Palestar Inc, Richard Lyle
*/

#include "UnitTest.h"
#include "Debug/Log.h"
#include "Debug/ExceptionHandler.h"
#include "Debug/Error.h"
#include "Debug/SafeThread.h"

#include <excpt.h>

//----------------------------------------------------------------------------

const int THREAD_COUNT = 8;			// number of threads to create
const int TEST_TIME = 5;			// number of seconds to test

//----------------------------------------------------------------------------

static bool bRunExceptionThreads = true;

class ExceptionThread : public SafeThread
{
public:
	int run()
	{
		int x = 0;
		while( bRunExceptionThreads )
		{
			x += rand() % 128;
			Thread::sleep( 100 );
		}

		return 0;
	}
};

static void GenerateException()
{
	__try {
		throw 0;
	}
	__except( ProcessException( GetExceptionInformation() ) )
	{}
}

//----------------------------------------------------------------------------

class TestExceptionHandler : public UnitTest
{ 
public:
	TestExceptionHandler( const char * pName ) : UnitTest( pName )
	{}

	void test()
	{
		int i;

		// disable the error handler
		SetProcessErrorMode( EM_DISABLED );
		// output log messages to stdout..
		StandardOutReactor sr;

		// start up several threads, each calling into the profiler..
		ExceptionThread * pThreads[ THREAD_COUNT ];
		for(i=0;i<THREAD_COUNT;i++)
		{
			pThreads[ i ] = new ExceptionThread();
			pThreads[ i ]->resume();
		}

		//Thread::sleep( 1000 );
		GenerateException();

		try {
			THROW_EXCEPTION( "Descriptive Error Message" );
		}
		catch( ExceptionBase )
		{}

		// close out all our threads..
		bRunExceptionThreads = false;
		for(i=0;i<THREAD_COUNT;i++)
			delete pThreads[ i ];

		// enable the error handler
		SetProcessErrorMode( EM_DEFAULT );
	}

} TEST_EXCEPTION_HANDLER( "TestExceptionHandler" );

//----------------------------------------------------------------------------
//EOF
