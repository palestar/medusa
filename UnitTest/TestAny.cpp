/**
	@file TestAny.cpp

	(c)2012 Palestar Inc
	@author Richard Lyle @date 12/27/2012 5:37:42 PM
*/

#include "UnitTest.h"
#include "Standard/Any.h"

#include <string>

//---------------------------------------------------------------------------------------------------

class TestAny : public UnitTest
{
public:
	TestAny( const char * pName ) : UnitTest( pName )
	{}

	void test()
	{
		Any data;

		int n = 20392;
		data.set( n );
		Test( data.get<int>() == n );

		std::string s = "Hello World";
		data.set( s );
		Test( data.get<std::string>() == s );
	}

} TEST_ANY( "TestAny" );

//---------------------------------------------------------------------------------------------------
//EOF
