/**
	@file TestSettings.cpp

	(c)2012 Palestar Inc
	@author Richard Lyle @date 4/16/2012 8:22:43 PM
*/

#include "UnitTest.h"
#include "Standard/Settings.h"

//---------------------------------------------------------------------------------------------------

class TestSettings : public UnitTest
{
public:
	TestSettings( const char * pName ) : UnitTest( pName )
	{}

	void test()
	{
		Settings t( "UnitTest", "UnitTest.ini" );
		
		t.put( "Key1", "Value1" );
		t.put( "Key2", "Value2" );
		t.put( "Key3", "Value3" );
		t.put( "Key4", "Value4" );
		t.put( "Key5", "Value5" );
		t.put( "Key6", "Value6" );
		t.put( "Key7", "Value7" );
		t.put( "Key8", "Value8" );
		t.put( "Key9", "Value9" );
		t.put( "Key10", "Value10" );
		t.put( "Key11", "Value11" );
		t.put( "Key12", "Value12" );

		Test( t.get( "Key1", "" ) == "Value1" );
		Test( t.get( "Key2", "" ) == "Value2" );
		Test( t.get( "Key3", "" ) == "Value3" );
		Test( t.get( "Key4", "" ) == "Value4" );
		Test( t.get( "Key5", "" ) == "Value5" );
		Test( t.get( "Key6", "" ) == "Value6" );
		Test( t.get( "Key7", "" ) == "Value7" );
		Test( t.get( "Key8", "" ) == "Value8" );
		Test( t.get( "Key9", "" ) == "Value9" );
		Test( t.get( "Key10", "" ) == "Value10" );
		Test( t.get( "Key11", "" ) == "Value11" );
		Test( t.get( "Key12", "" ) == "Value12" );
	}

} TEST_SETTINGS( "TestSettings" );

//---------------------------------------------------------------------------------------------------
//EOF
