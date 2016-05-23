/*
	TestStringHash.cpp
	(c)2005 Palestar Inc, Richard Lyle
*/

#include "UnitTest.h"
#include "Standard/StringHash.h"
#include "Standard/StringHash64.h"



//----------------------------------------------------------------------------

class TestStringHash : public UnitTest
{
public:
	TestStringHash( const char * pName ) : UnitTest( pName )
	{}

	void test()
	{
		Test( StringHash64( "m:\\Ports\\start.prt") == LL( 15530459713961322492 ) );
		Test( StringHash64( "m:/Ports/start.prt") == LL( 15530459713961322492 ) );
		Test( StringHash64( "M:/PORTS/START.PRT") == LL( 15530459713961322492 ) );

		Test( StringHash( "The Quick Brown Fox") == 1672002192 );
		Test( StringHash( "The QUICK BROWN FOX") == 1672002192 );
		Test( StringHash( "the quick brown fox") == 1672002192 );
	}

} TEST_STRINGHASH( "TestStringHash" );

//----------------------------------------------------------------------------
//EOF

