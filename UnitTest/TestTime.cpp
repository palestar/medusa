/*
	TestTime.cpp
	(c)2005 Palestar Inc, Richard Lyle
*/

#include "UnitTest.h"
#include "Standard/Time.h"



//----------------------------------------------------------------------------

class TestTime : public UnitTest
{
public:
	TestTime( const char * pName ) : UnitTest( pName )
	{}

	void test()
	{
		dword nTime = Time::seconds();
		Test( nTime != 0 );

		CharString sTime = Time::format( nTime, "%m/%d/%Y %H:%M:%S" );
		Test( Time::isTime( nTime, sTime ) );
		sTime = Time::format( nTime, "%m/%d/%y %H:%M:%S" );
		Test( Time::isTime( nTime, sTime ) );

		sTime = Time::format( nTime, "*/*/* %H:%M:%S" );
		Test( Time::isTime( nTime, sTime ) );
	}

} TEST_TIME( "TestTime" );

//----------------------------------------------------------------------------
//EOF
