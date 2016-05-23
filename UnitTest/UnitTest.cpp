/*
	UnitTest.cpp
	(c)2005 Palestar Inc, Richard Lyle
*/

#include "Standard/StringHelpers.h"
#include "Standard/Thread.h"
#include "UnitTest.h"

#include <stdio.h>
#include <conio.h>



//----------------------------------------------------------------------------

UnitTest * UnitTest::s_pHead = NULL;

UnitTest::UnitTest( const char * pName ) : m_pName( pName ), m_pNext( NULL )
{
	// attach into our linked list of unit test objects
	m_pNext = s_pHead;
	s_pHead = this;
}

//----------------------------------------------------------------------------

int main(int argc, char* argv[])
{
	int nTestCount = 0;
	int nFailedCount = 0;
	int nPassedCount = 0;

	UnitTest * pNext = UnitTest::s_pHead;
	while( pNext != NULL )
	{
		UnitTest * pTest = pNext;
		pNext = pNext->next();

		bool bFailure = false;

		if ( argc > 1 )
		{
			bool bFound = false;
			for(int i=1;i<argc && !bFound;i++)
				if ( stricmp( pTest->name(), argv[i]) == 0 )
					bFound = true;

			if (! bFound )
				continue;		// skip this test it doesn't match any of our arguments...
		}

		nTestCount++;

#ifndef _DEBUG
		try {
#endif
			pTest->test();
#ifndef _DEBUG
		}
		catch( ... )
		{
			bFailure = true;
		}
#endif

		printf("-------------------------------------------------------------\n");
		if ( bFailure )
		{
			printf("%s FAILED\n", pTest->name() );
			nFailedCount++;
		}
		else
		{
			printf("%s PASSED\n", pTest->name() );
			nPassedCount++;
		}
		printf("-------------------------------------------------------------\n");
	}
	printf("%d TESTS RAN, %d FAILED, %d PASSED...\n", nTestCount, nFailedCount, nPassedCount );
	//getch();

	Thread::sleep( 5000 );

	return nFailedCount > 0 ? 1 : 0;
}

//----------------------------------------------------------------------------
// EOF

