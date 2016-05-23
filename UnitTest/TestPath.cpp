/*
	TestPath.cpp
	(c)2005 Palestar Inc, Richard Lyle
*/

//----------------------------------------------------------------------------

#include "UnitTest.h"
#include "File/Path.h"

//----------------------------------------------------------------------------

class TestPath : public UnitTest
{
public:
	TestPath( const char * pName ) : UnitTest( pName )
	{}

	void test()
	{
		Path sTestPath;
		sTestPath.setPath( "\\file.ext" );
		Test( sTestPath.directory() == "\\" );
		Test( sTestPath.name() == "file" );
		Test( sTestPath.extension() == ".ext" );
		Test( sTestPath.file() == "file.ext" );

		sTestPath.setPath( "\\blah\\directory/2\\3/file.extension" );
		Test( sTestPath.directory() == "\\blah\\directory\\2\\3\\" );
		Test( sTestPath.name() == "file" );
		Test( sTestPath.extension() == ".extension" );
		Test( sTestPath.file() == "file.extension" );
	}

} TEST_PATH( "TestPath" );

//----------------------------------------------------------------------------
//EOF
