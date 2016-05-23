/*
	Test.h

	All component tests use this component
	(c)2005 Palestar, Richard Lyle
*/

#ifndef TEST_H
#define TEST_H

//-------------------------------------------------------------------------------

// On test failure, this exception is thrown
class TestException {};

// Behaves like "assert"
inline void Test( bool result )
{
	if ( !result )
		throw TestException();
}

#endif

//-------------------------------------------------------------------------------
// EOF
