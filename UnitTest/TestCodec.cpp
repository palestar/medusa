/*
	TestCodec.cpp
	(c)2005 Palestar Inc, Richard Lyle
*/

#include "UnitTest.h"
#include "Standard/Codec.h"

//----------------------------------------------------------------------------

const int CODEC_DATA_SIZE = 8 * 1024;

//----------------------------------------------------------------------------

class TestCodec : public UnitTest
{
public:
	TestCodec( const char * pName ) : UnitTest( pName )
	{}

	void test()
	{
		for(int i=0;i<Codec::factoryCount();i++)
		{
			Codec::Factory * pFactory = Codec::factory( i );

			// generate random data
			byte data[ CODEC_DATA_SIZE ];
			for(int i=0;i<sizeof(data);i++)
				data[i] = (i & 0xff); //rand() & 0xff;

			// create the codec
			Codec * pCodec = pFactory->create();
			Test( pCodec != NULL );

			byte encoded[ CODEC_DATA_SIZE * 3 ];
			int nBytes = pCodec->encode( data, sizeof(data), encoded, sizeof(encoded), Codec::CL_NORMAL );
			Test( nBytes >= 0 );

			delete pCodec;

			printf("Codec %s, %d bytes encoded into %d bytes - %d%% Compression\n", 
				pFactory->name(), CODEC_DATA_SIZE, nBytes, ((nBytes * 100) / CODEC_DATA_SIZE) );
			
			pCodec = pFactory->create();
			Test( pCodec != NULL );

			int nDecodedBytes = pCodec->decode( encoded, nBytes, data, sizeof(data) );
			Test( nDecodedBytes == CODEC_DATA_SIZE );

			for(int j=0;j<sizeof(data);j++)
				Test( data[j] == (j & 0xff) );
			delete pCodec;
		}
	}

} TEST_CODEC( "TestCodec" );

//----------------------------------------------------------------------------
//EOF
