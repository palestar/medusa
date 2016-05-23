/*
	TestExpression.cpp
	(c)2005 Palestar Inc, Richard Lyle
*/

#include "UnitTest.h"
#include "Standard/Expression.h"

//----------------------------------------------------------------------------

class TestExpression : public UnitTest
{
public:
	TestExpression( const char * pName ) : UnitTest( pName )
	{}

	void test()
	{
		Expression ex;

		Test( ex.parse( "x = y" ) == 3 );
		Test( ex.token( 0 ).nType == Expression::TOKEN_SYMBOL );
		Test( ex.token( 0 ).sToken == CharString("x") );
		Test( ex.token( 1 ).nType == Expression::TOKEN_ASSIGNMENT );
		Test( ex.token( 2 ).nType == Expression::TOKEN_SYMBOL );
		Test( ex.token( 2 ).sToken == "y" );
		Test( ex.restring() == "x=y" );
	}

} TEST_EXPRESSION( "TestExpression" );

//----------------------------------------------------------------------------
//EOF
