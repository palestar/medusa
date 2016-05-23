/*
	TestTree.cpp
	(c)2005 Palestar Inc, Richard Lyle
*/

#include "UnitTest.h"
#include "Standard/Tree.h"
#include "Standard/Types.h"



//----------------------------------------------------------------------------

const int TEST_COUNT = 5000;

//----------------------------------------------------------------------------

class TestTree : public UnitTest
{
public:
	TestTree( const char * pName ) : UnitTest( pName )
	{}

	void test()
	{
		Tree< dword, dword > tree;

		srand( 7777 );

		int nInserted = 0;
		for(int i=0;i<TEST_COUNT;i++)
		{
			dword nNum = rand();
			if (! tree.find( nNum ).valid() )
			{
				tree[ nNum ] = nNum;
				nInserted++;
			}
		}
		Test( tree.size() == nInserted );

		srand( 7777 );
		for(int i=0;i<TEST_COUNT;i++)
		{
			dword nNum = rand();
			Test( tree.find( nNum ).valid() );
		}
		

		int nIterated = 0;

		Tree< dword, dword >::Iterator it = tree.head();
		while( it.valid() )
		{
			nIterated++;

			Test( *it == it.key() );

			it++;
		}

		Test( nIterated == nInserted );

		srand( 7777 );
		for(int i=0;i<TEST_COUNT;i++)
			tree.remove( rand() );

		Test( tree.size() == 0 );
	}

} TEST_TREE( "TestTree" );


//----------------------------------------------------------------------------
//EOF
