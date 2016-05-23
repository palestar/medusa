/*
	TestFileDisk.cpp
	(c)2005 Palestar Inc, Richard Lyle
*/

#include "UnitTest.h"
#include "File/FileDisk.h"



//----------------------------------------------------------------------------

class TestFileDisk : public UnitTest
{
public:
	TestFileDisk( const char * pName ) : UnitTest( pName )
	{}

	void test()
	{
		// test directory creation
		Test( FileDisk::createDirectory( ".\\Test\\Sub\\" ) );
		Test( FileDisk::fileExists( ".\\Test\\Sub" ) );

		// Test directory removal
		Test(! FileDisk::removeDirectory( ".\\Test" ) );		// un-forced remove should fail
		Test( FileDisk::removeDirectory( ".\\Test", true ) );	// forced remove directory should work
		Test(! FileDisk::fileExists( ".\\Test" ) );
		Test(! FileDisk::fileExists( ".\\Test\\Sub" ) );
	}

} TEST_FILEDISK( "TestFileDisk" );

//----------------------------------------------------------------------------
//EOF
