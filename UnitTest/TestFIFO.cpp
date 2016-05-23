// FIFOBuffer_Test.cpp : Defines the entry point for the console application.
//

#include "UnitTest.h"
#include "Standard/Thread.h"
#include "Standard/FIFOBuffer.h"

#include <stdlib.h>
#include <stdio.h>



class TestFIFO : public UnitTest
{
public:
	TestFIFO( const char * pName ) : UnitTest( pName )
	{}

	void test()
	{
		FIFOBuffer buffer;

		for(int i=0;i<100;i++)
		{
			switch( rand() % 2 )
			{
			case 0:
				{
					int nBytes = rand() % 8000;
					byte * pBytes = new byte[ nBytes ];

					int nCRC = 0;
					for(int j=0;j<nBytes;j++)
					{
						pBytes[j] = rand();
						nCRC += pBytes[j];
					}

					buffer.push( &nCRC, sizeof(nCRC) );
					buffer.push( &nBytes, sizeof(nBytes) );
					buffer.push( pBytes, nBytes );

					delete [] pBytes;

					//printf( "Pushed %d bytes into FIFO, size = %d\n", nBytes, buffer.size());
				}
				break;
			case 1:
				{
					int nCRC = 0;
					if ( buffer.pop( &nCRC, sizeof(nCRC) ) == 0 )
						break;
					int nBytes = 0;
					Test( buffer.pop( &nBytes, sizeof(nBytes) ) == sizeof(nBytes) );
					
					byte * pBytes = new byte[ nBytes ];
					Test( buffer.pop( pBytes, nBytes ) == nBytes );

					int nCRC2 = 0;
					for(int j=0;j<nBytes;j++)
						nCRC2 += pBytes[j];

					Test( nCRC2 == nCRC );
					
					delete [] pBytes;

					//printf( "Popped %d bytes from FIFO, size = %d\n", nBytes, buffer.size() );
				}
				break;
			}

			Thread::sleep( 0 );
		}
	}
} TEST_FIFO( "FIFOBuffer" );

