/*
	TestType.cpp
	(c)2005 Palestar Inc, Richard Lyle
*/

#include "UnitTest.h"
#include "Reflection/TypeTemplate.h"
#include "Reflection/Value.h"
#include "Standard/String.h"
#include "Math/Vector3.h"

//----------------------------------------------------------------------------

class TestType : public UnitTest
{
public:
	TestType( const char * pName ) : UnitTest( pName )
	{}

	void test()
	{
		Type * pIntType = TypeTemplate<int>::instance();
		Test( pIntType != NULL );
		Type * pShortType = TypeTemplate<short>::instance();
		Test( pShortType != NULL );
		void * pIntData = pIntType->allocate();
		Test( pIntData != NULL );
		void * pShortData = pShortType->allocate();
		Test( pShortData != NULL );

		int nValue = 42;		// meaning of life...
		Test( Type::copy( pIntData, pIntType, &nValue, pIntType ) );
		Test( Type::copy( pShortData, pShortType, pIntData, pIntType ) );
		short nValue2 = -1;
		Test( Type::copy( &nValue2, pShortType, pShortData, pShortType ) );

		Test( nValue2 == nValue );

		// test indirect conversions now
		Type * pStringType = TypeTemplate<String>::instance();
		Test( pStringType != NULL );

		String * pString = (String *)pStringType->allocate();
		Test( Type::copy( pString, pStringType, pShortData, pShortType ) );

		Test( *pString == L"42" );
		(*pString) = L"12453";

		Test( Type::copy( pIntData, pIntType, pString, pStringType ) );
		Test( *((int *)pIntData) == 12453 );

		pIntType->free( pIntData );
		pShortType->free( pShortData );
		pStringType->free( pString );

		
		// Test value class
		{
			Value vi;
			Value vs;

			vi = 64;
			Test( ((int)vi) == 64 );

			vs = CharString("Hello");
			Test( ((WideString)vs) == L"Hello" );

			// Test automatic conversion
			vs = CharString("3847348");
			vi = vs;
			int i = vi;
			Test( i == 3847348 );
		}

		// Test Vector3 type
		{
			Value vector;
			vector = Vector3(1,2,3);
			CharString string = vector;
			Test( string == "1.00,2.00,3.00" );
		}
	}

} TEST_TYPE( "TestType" );

//----------------------------------------------------------------------------
//EOF
