/*
	TestReferenced.cpp
	(c)2005 Palestar Inc, Richard Lyle
*/

#include "UnitTest.h"

#include "Standard/Reference.h"
#include "Standard/WeakReference.h"

//---------------------------------------------------------------------------------------------------

class TestObject : public Referenced
{
public:
	typedef Reference< TestObject >			Ref;
	typedef WeakReference< TestObject >		Ptr;

	TestObject()
	{}
};

//---------------------------------------------------------------------------------------------------

class TestReferenced : public UnitTest
{
public:
	TestReferenced( const char * pName ) : UnitTest( pName )
	{}

	void test()
	{
		TestObject::Ref pRef1 = new TestObject();
		Test( pRef1->referenceCount() == 1 );
		TestObject::Ref pRef2 = pRef1;
		Test( pRef1->referenceCount() == 2 );

		TestObject::Ptr pWeak = pRef1;
		Test( pRef1->referenceCount() == 2 );		// should not increment the reference

		pRef2 = NULL;
		Test( pRef1->referenceCount() == 1 );
		pRef1 = NULL;							
		Test(! pWeak.valid() );						// pointer should be NULL automatically
	}

} TEST_REFERENCED( "TestReferenced" );

//----------------------------------------------------------------------------
//EOF
