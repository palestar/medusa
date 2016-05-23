/*
	TestThread.cpp
	(c)2005 Palestar Inc, Richard Lyle
*/

#include "UnitTest.h"
#include "Standard/Thread.h"

//

//----------------------------------------------------------------------------

class TestThread : public UnitTest
{
public:
	TestThread( const char * pName ) : UnitTest( pName )
	{}

	void test()
	{
	}

} TEST_THREAD( "TestThread" );

//----------------------------------------------------------------------------
//EOF
