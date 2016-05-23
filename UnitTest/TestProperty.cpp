/*
	TestProperty.cpp
	(c)2005 Palestar Inc, Richard Lyle
*/

#include "UnitTest.h"
#include "Factory/FactoryTypes.h"
#include "Standard/String.h"

//----------------------------------------------------------------------------

struct EmbeddedStructure : public Widget
{
	DECLARE_PROPERTY_LIST();

	typedef Reference<EmbeddedStructure>	Ref;

	int				m_x;
	int				m_y;
};

BEGIN_PROPERTY_LIST( EmbeddedStructure, Widget )
	ADD_PROPERTY( m_x );
	ADD_PROPERTY( m_y );
END_PROPERTY_LIST();

struct X 
{
	X()
	{}
	virtual ~X()
	{}

	int			m_SomeData[ 4 ];
};


class TestPropertyClass : public Widget
{
public:
	DECLARE_PROPERTY_LIST();		// This line must exist in the class definition

	typedef Reference<TestPropertyClass>	Ref;

	TestPropertyClass() : m_n( 0 ), m_pStructure( NULL ), m_pPointer( NULL )
	{
		m_IntArray[0] = 0;
		m_IntArray[1] = 0;
		m_IntArray[2] = 0;
		m_IntArray[3] = 0;
		m_IntArray[4] = 0;
		m_IntArray[5] = 0;
	}

	int				m_n;					// some properties we're going to expose
	String			m_s;
	int				m_IntArray[ 6 ];

	EmbeddedStructure *
					m_pStructure;
	EmbeddedStructure *
					m_pPointer;
	EmbeddedStructure::Ref	
					m_pRef;

	EmbeddedStructure
					m_Structure;

	Array< int >	
					m_DynamicIntArray;
	Array< EmbeddedStructure >
					m_DynamicStructureArray;
};

BEGIN_PROPERTY_LIST( TestPropertyClass, Widget )
	ADD_PROPERTY( m_n )
		ADD_OPTION( m_n, 32 );				// options are only hints to the user as to what the property should be set too...
		ADD_OPTION( m_n, 64 );
		ADD_OPTION( m_n, CharString("X") );
	ADD_PROPERTY( m_s )
		SET_PROPERTY_NOTE( m_s, "String Property" );
	ADD_PROPERTY_ARRAY( m_IntArray );
	ADD_PROPERTY( m_pStructure );
	ADD_PROPERTY( m_pPointer );
	ADD_PROPERTY( m_pRef );
	ADD_PROPERTY( m_Structure );
	ADD_PROPERTY( m_DynamicIntArray );
	ADD_PROPERTY( m_DynamicStructureArray );
END_PROPERTY_LIST()


class TestPropertyClass2 : public X, public TestPropertyClass
{
public:
	DECLARE_PROPERTY_LIST();		// This line must exist in the class definition

	typedef Reference<TestPropertyClass2>	Ref;

	int				m_X;
	int				m_Y;
};

BEGIN_PROPERTY_LIST( TestPropertyClass2, TestPropertyClass )
	ADD_PROPERTY( m_X );
	ADD_PROPERTY( m_Y )
END_PROPERTY_LIST()

//----------------------------------------------------------------------------

class TestProperty : public UnitTest
{
public:
	TestProperty( const char * pName ) : UnitTest( pName )
	{}

	void test()
	{
		TestPropertyClass2::Ref pTest = new TestPropertyClass2();
		// test pointer conversions
		Widget * pWidget = pTest;			// this will convert the pointer...
		Test( pWidget->set( "m_n", 82 ) );
		Test( pTest->m_n == 82 );
		Test( pWidget->set( "m_X", 33 ) );
		Test( pTest->m_X == 33 );
		Test( pWidget->set( "m_Y", 34 ) );
		Test( pTest->m_Y == 34 );

		pTest->m_n = 42;
		pTest->m_s = L"Hello";
		pTest->m_pStructure = NULL;

		Test( pTest->set( "m_n", 52 ) );
		Test( pTest->m_n == 52 );

		String sTest;
		Test( pTest->get( "m_s", sTest ) );
		Test( pTest->m_s == sTest );

		// Test for failure when trying to convert a string to an integer
		int nTest = -1;
		//Test(! pTest->get( "m_s", nTest ) );

		// Test pointer to type conversions (char * -> String, wchar * -> String)
		Test( pTest->set( "m_s", "Dork" ) );
		Test( pTest->m_s == STR("Dork") );
		Test( pTest->set( "m_s", "Wimp" ) );
		Test( pTest->m_s == STR("Wimp") );

		Property::Find found;
		Test( pTest->find( "m_n", found ) );
		Test( found.pProperty != NULL );
		Test( found.pProperty->optionCount() == 3 );

		Test( pTest->find( "m_s", found ) );
		Test( found.pProperty != NULL );
		Test( found.pProperty->note() != NULL );

		Test( pTest->set( "m_IntArray[2]", 23 ) );
		Test( pTest->set( "m_IntArray[5]", 64 ) );
		Test(! pTest->set( "m_IntArray[20]", 64 ) );		// should fail, array out of bounds!
		Test( pTest->m_IntArray[2] == 23 );
		Test( pTest->m_IntArray[5] == 64 );

		// Test Pointer Conversions
		TypeCopyDefault< EmbeddedStructure *, EmbeddedStructure * >::instance();
		TypeCopyDefault< EmbeddedStructure *, EmbeddedStructure::Ref >::instance();
		TypeCopyDefault< EmbeddedStructure::Ref, EmbeddedStructure * >::instance();
		TypeCopyDefault< EmbeddedStructure::Ref, EmbeddedStructure::Ref >::instance();

		EmbeddedStructure * pEmbedded = new EmbeddedStructure;
		Test( pTest->set( "m_pPointer", pEmbedded ) );
		Test( pTest->m_pPointer == pEmbedded );

		Test( pTest->set( "m_pRef", pEmbedded ) );
		Test( pTest->m_pRef.pointer() == pEmbedded );
		Test( pEmbedded->referenceCount() == 1 );				// make sure the reference count got incremented

		Test(! pTest->set( "m_pRef", NULL ) );
		Test( pTest->set( "m_pRef", (EmbeddedStructure *)NULL ) );

		// Test member structures
		Test( pTest->set( "m_Structure.m_x", 23 ) );
		Test( pTest->set( "m_Structure.m_y", 86 ) );
		Test( pTest->m_Structure.m_x == 23 );
		Test( pTest->m_Structure.m_y == 86 );
		Test( pTest->get( "m_Structure.m_x", nTest ) && nTest == 23 );
		Test( pTest->get( "m_Structure.m_y", nTest ) && nTest == 86 );

		Test( pTest->set( "m_pStructure", new EmbeddedStructure) );
		Test( pTest->m_pStructure != NULL );
		Test( pTest->set( "m_pStructure->m_x", 25) ); 
		Test( pTest->set( "m_pStructure->m_y", -50) );
		Test( pTest->m_pStructure->m_x == 25 );
		Test( pTest->m_pStructure->m_y == -50 );

		Test( pTest->get( "m_pStructure->m_x", nTest ) && nTest == 25 );
		Test( pTest->get( "m_pStructure->m_y", nTest ) && nTest == -50 );

		Test( pTest->set( "m_DynamicIntArray[0]", 12 ) );
		Test( pTest->set( "m_DynamicIntArray[1]", 56 ) );
		Test( pTest->m_DynamicIntArray[0] == 12 );
		Test( pTest->m_DynamicIntArray[1] == 56 );

		Test( pTest->set( "m_DynamicStructureArray[0].m_x", 34 ) );
		Test( pTest->set( "m_DynamicStructureArray[0].m_y", 67 ) );
		Test( pTest->m_DynamicStructureArray.size() == 1 );
		Test( pTest->m_DynamicStructureArray[0].m_x == 34 );
		Test( pTest->m_DynamicStructureArray[0].m_y == 67 );

		Array< Property::Find > enu;
		Test( pTest->enumerate( enu ) );

		for(int i=0;i<enu.size();i++)
		{
			Property::Find & f = enu[i];

			CharString sValue = "?";
			pTest->get( f.sExpr, sValue );

			printf( "%s %s, p=%p, size=%d, value = %s\n", f.pType->name(), f.sExpr, f.pData, f.pType->size(), sValue );
		}

		Test( pTest->set( "m_pStructure", (EmbeddedStructure *)NULL) );
		Test(! pTest->set( "m_pStructure->m_x", 32 ) );		// should fail, NULL pointer..

		// Test the generic Value container.. 
		Value v1 = 5221.0f;
		float fNumber = v1;
		Test( fNumber == 5221.0f );
		CharString sNumber = v1;
		Test( sNumber == "5221.000000" );

		Value v2 = v1;
		Test( ((float)v2) == 5221.0f );
	}

} TEST_PROPERTY( "TestProperty" );

//----------------------------------------------------------------------------
//EOF
