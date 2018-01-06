/*
	TestArray.cpp
	(c)2005 Palestar Inc, Richard Lyle
*/

#include "UnitTest.h"

#include "Standard/Array.h"
#include "Standard/String.h"
#include "Standard/Types.h"
#include "Standard/Time.h"

#include <vector>

//----------------------------------------------------------------------------

const int ELEMENTS = 100000;
const int SORT_ELEMENTS = 20;

int ArrayCompare( int e1, int e2 )
{
	return e1 - e2;
}

int StringCompare( String e1, String e2 )
{
	return e1.compareNoCase( e2 );
}

//----------------------------------------------------------------------------

class TestArray : public UnitTest
{
public:
	TestArray( const char * pName ) : UnitTest( pName )
	{}

	void test()
	{
		u64 begin;
		int i;

		//----------------------------------------------------------------------------

		Array< dword > testArray;

		//----------------------------------------------------------------------------

		begin = Time::CPU();
		for(i=0;i<ELEMENTS;i++)
			testArray.push( rand() );

		printf("array insert  = %I64u\n", Time::CPU() - begin );

		//----------------------------------------------------------------------------

		begin = Time::CPU();
		for(i=0;i<ELEMENTS;i++)
			testArray[ i ]++;

		printf("array modify  = %I64u\n", Time::CPU() - begin );

		//----------------------------------------------------------------------------

		std::vector< dword > testVector;

		//----------------------------------------------------------------------------

		begin = Time::CPU();
		for(i=0;i<ELEMENTS;i++)
			testVector.push_back( rand() );

		printf("vector insert = %I64u\n", Time::CPU() - begin );

		//----------------------------------------------------------------------------

		begin = Time::CPU();
		for(i=0;i<ELEMENTS;i++)
			testVector[ i ]++;

		printf("vector modify = %I64u\n", Time::CPU() - begin );

		//----------------------------------------------------------------------------

		// Test sorting
		Array< int > sortArray;
		for(i=0;i<SORT_ELEMENTS;i++)
			sortArray.push( rand() );

		printf("\nUnsorted: ");
		for(i=0;i<sortArray.size();i++)
			printf(" %d", sortArray[i] );

		sortArray.qsort();

		printf( "\nSorted: " );
		for(i=0;i<sortArray.size();i++)
			printf(" %d", sortArray[i] );
		printf( "\n");

		// Test sorting
		sortArray.release();
		for(i=0;i<SORT_ELEMENTS;i++)
			sortArray.push( 5 );

		printf("\nUnsorted: ");
		for(i=0;i<sortArray.size();i++)
			printf(" %d", sortArray[i] );

		sortArray.qsort();

		printf( "\nSorted: " );
		for(i=0;i<sortArray.size();i++)
			printf(" %d", sortArray[i] );
		printf( "\n");

		// Test sorting
		sortArray.release();
		for(i=0;i<SORT_ELEMENTS;i++)
			sortArray.push( SORT_ELEMENTS - i );

		printf("\nUnsorted: ");
		for(i=0;i<sortArray.size();i++)
			printf(" %d", sortArray[i] );

		sortArray.qsort();

		printf( "\nSorted: " );
		for(i=0;i<sortArray.size();i++)
			printf(" %d", sortArray[i] );
		printf( "\n");

		// Test string sorting
		Array< String > stringArray;
		stringArray.push( "Dog" );
		stringArray.push( "Cat" );
		stringArray.push( "Frog" );
		stringArray.push( "House" );
		stringArray.push( "Zebra" );
		stringArray.push( "Apple" );

		printf("\nUnsorted: ");
		for(i=0;i<sortArray.size();i++)
			printf(" %s", (char *)stringArray[i].cstr() );

		stringArray.qsort( StringCompare );

		printf( "\nSorted: " );
		for(i=0;i<sortArray.size();i++)
			printf(" %s", (char *)stringArray[i].cstr());
		printf( "\n");
	}

} TEST_ARRAY( "TestArray" );


//----------------------------------------------------------------------------
//EOF
