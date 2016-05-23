/*
	TestString.cpp
	(c)2005 Palestar Inc, Richard Lyle
*/

#include "UnitTest.h"
#include "Standard/String.h"



//----------------------------------------------------------------------------

class TestString : public UnitTest
{
public:
	TestString( const char * pName ) : UnitTest( pName )
	{}

	void test()
	{
		WideString s;
		CharString c;

		// test ANSI to UNICODE conversion
		s = "Quick Brown Fox";
		c = s;
		s = c;
		c = L"Testing";

		// test conversion from string to const char *
		//const char * p = s;
		//Test( strcmp<char>( p, s ) == 0 );
	}

} TEST_STRING( "TestString" );

//----------------------------------------------------------------------------
//EOF
