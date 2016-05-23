/*
	UnitTest.h

	Wrapper class for a unit test
	(c)2005 Palestar Inc, Richard Lyle
*/

#ifndef UNITTEST_H
#define UNITTEST_H

//----------------------------------------------------------------------------

class UnitTest
{
public:
	// Exceptions
	class Failure {};

	// Construction
	UnitTest( const char * pName );
	// Accessors
	const char *			name() const;
	UnitTest *				next() const;
	// Interface
	virtual void			test() = 0;

private:
	// Data
	const char *			m_pName;
	UnitTest *				m_pNext;

public:
	static UnitTest *		s_pHead;
};

//----------------------------------------------------------------------------

inline const char * UnitTest::name() const
{
	return m_pName;
}

inline UnitTest * UnitTest::next() const
{
	return m_pNext;
}

//----------------------------------------------------------------------------

inline void Test( bool result )
{
	if ( !result )
		throw UnitTest::Failure();
}

//----------------------------------------------------------------------------

#endif

//----------------------------------------------------------------------------
//EOF
