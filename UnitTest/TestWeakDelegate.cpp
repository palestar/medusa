/**
	@file TestWeakDelegate.cpp

	(c)2009 Palestar Inc
	@author Richard Lyle @date 4/3/2010 6:05:10 PM
*/

#include "UnitTest.h"
#include "Standard/WeakDelegate.h"

//---------------------------------------------------------------------------------------------------

class MyClass : public Referenced
{
public:
	MyClass() : m_nTotal( 0 )
	{}

	void Add(int n )
	{
		m_nTotal += n;
	}

	int m_nTotal;
};

class TestWeakDelegate : public UnitTest
{
public:
	TestWeakDelegate( const char * pName ) : UnitTest( pName )
	{}

	void test()
	{
		MyClass * pObject = new MyClass();

		WeakDelegate<int> wd = WeakDelegate<int>::Create<MyClass,&MyClass::Add>( pObject );
		Test( wd( 50 ) );
		Test( wd( 50 ) );
		Test( pObject->m_nTotal == 100 );

		delete pObject;

		// this should fail to invoke, but not crash..
		Test( !wd( 50 ) );
	}

} TEST_BLANK( "TestWeakDelegate" );

//---------------------------------------------------------------------------------------------------
//EOF
