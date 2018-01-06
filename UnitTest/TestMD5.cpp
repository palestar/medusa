/*
	TestMD5.cpp
	(c)2005 Palestar Inc, Richard Lyle
*/

//----------------------------------------------------------------------------

#include "UnitTest.h"
#include "Standard/MD5.h"
#include <stdio.h>



class TestMD5 : public UnitTest
{
public:
	TestMD5( const char * pName ) : UnitTest( pName )
	{}

	void test()
	{
		printf("Spot32 = %s\n", (char *)MD5("spot32").checksum().cstr() );
		printf("Spot7471 = %s\n", (char *)MD5("spot7471").checksum().cstr());
		printf("666666 = %s\n", (char *)MD5("666666").checksum().cstr());
	}

} TEST_MD5( "TestMD5" );


//----------------------------------------------------------------------------
//EOF
