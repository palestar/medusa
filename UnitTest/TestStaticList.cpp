/*
	TestStaticList.cpp
	(c)2005 Palestar Inc, Richard Lyle
*/

#include "UnitTest.h"
#include "Standard/StaticList.h"



//----------------------------------------------------------------------------

class StaticListClass : public StaticList<StaticListClass>
{
public:
	StaticListClass( int x ) : m_X( x )
	{}
	virtual ~StaticListClass()
	{}

	int		m_X;
};

//----------------------------------------------------------------------------

class TestStaticList : public UnitTest
{
public:
	TestStaticList( const char * pName ) : UnitTest( pName )
	{}

	void test()
	{
		new StaticListClass(3);
		new StaticListClass(2);
		new StaticListClass(1);
		new StaticListClass(0);

		List<StaticListClass *>::Iterator it = StaticListClass::staticList().head();

		int nCount = 0;
		while( it.valid() )
		{
			Test( (*it)->m_X == nCount );

			nCount++;
			it++;
		}

		Test( nCount == 4 );

		it = StaticListClass::staticList().head();
		while( it.valid() )
		{
			StaticListClass * pDelete = *it;
			delete pDelete;

			it = StaticListClass::staticList().head();
		}
	}

} TEST_STATIC_LIST( "TestStaticList" );

//----------------------------------------------------------------------------
//EOF
