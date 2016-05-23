/*
	ZLIB.cpp
	(c)2005 Palestar Inc, Richard Lyle
*/

#include "Standard/ZLIB.h"

// include the zlib library header
#include "zlib/zlib.h"
// standard library
#include <stdlib.h>
#include <string.h>



//----------------------------------------------------------------------------

// ZLIB allocators
static void * Allocate( void * pOpaque, unsigned int items, unsigned int size )
{
	return malloc( items * size );
}

static void Free( void * pOpaque, void * pFree )
{
	free( pFree );
}

//----------------------------------------------------------------------------

Codec::FactoryTemplate<ZLIB>	ZLIB_FACTORY( "ZLIB" );

ZLIB::ZLIB() : m_pCompressor( NULL ), m_pExpander( NULL )
{}

ZLIB::~ZLIB()
{
	reset();
}

//----------------------------------------------------------------------------

Codec * ZLIB::clone()
{
	return new ZLIB();
}

int ZLIB::encode( const void * input, int inputSize, void * output, int outputSize, EncodeLevel nLevel )
{
	if ( inputSize <= 0 )
		return 0;		// nothing to compress

	if ( m_pCompressor == NULL )
	{
		int nLevel = -1;
		switch( nLevel )
		{
		case CL_BEST:
			nLevel = 9;
			break;
		case CL_FAST:
			nLevel = 3;
			break;
		case CL_NONE:
			nLevel = 0;
			break;
		}

		// initialize the compressor
		m_pCompressor = new z_stream;
		memset( m_pCompressor, 0, sizeof(z_stream) );
		((z_stream *)m_pCompressor)->zalloc = Allocate;
		((z_stream *)m_pCompressor)->zfree = Free;
		
		deflateInit( (z_stream *)m_pCompressor, nLevel );
	}

	// setup and compress the data
	((z_stream *)m_pCompressor)->next_in = (byte *)input;
	((z_stream *)m_pCompressor)->avail_in = inputSize;
	((z_stream *)m_pCompressor)->total_in = 0;
	
	((z_stream *)m_pCompressor)->next_out = (byte *)output;
	((z_stream *)m_pCompressor)->avail_out = outputSize;
	((z_stream *)m_pCompressor)->total_out = 0;

	try {
		if ( deflate( (z_stream *)m_pCompressor, Z_SYNC_FLUSH ) < 0 )
			return -1;		// error occured
		if ( ((z_stream *)m_pCompressor)->total_in != (unsigned int)inputSize )
			return -1;		// failed to compress all data

		return ((z_stream *)m_pCompressor)->total_out;
	}
	catch( ... )
	{}

	// caught an exception while compressing...
	return -1;
}

int ZLIB::decode( const void * input, int inputSize, void * output, int outputSize )
{
	if ( inputSize <= 0 )
		return 0;		// nothing to decompress

	if ( m_pExpander == NULL )
	{
		m_pExpander = new z_stream;
		memset( m_pExpander, 0, sizeof(z_stream) );
		((z_stream *)m_pExpander)->zalloc = Allocate;
		((z_stream *)m_pExpander)->zfree = Free;

		inflateInit( (z_stream *)m_pExpander );
	}

	// setup and compress the data
	((z_stream *)m_pExpander)->next_in = (byte *)input;
	((z_stream *)m_pExpander)->avail_in = inputSize;
	((z_stream *)m_pExpander)->total_in = 0;
	
	((z_stream *)m_pExpander)->next_out = (byte *)output;
	((z_stream *)m_pExpander)->avail_out = outputSize;
	((z_stream *)m_pExpander)->total_out = 0;

	try {
		if ( inflate( (z_stream *)m_pExpander, Z_SYNC_FLUSH ) < 0 )
			return -1;		// error occured

		return ((z_stream *)m_pExpander)->total_out;
	}
	catch( ... )
	{}

	return -1;		// failed to decompress!
}

void ZLIB::reset()
{
	if ( m_pCompressor != NULL )
	{
		deflateEnd( (z_stream *)m_pCompressor );
		delete ((z_stream *)m_pCompressor);
		
		m_pCompressor = NULL;
	}
	if ( m_pExpander != NULL )
	{
		inflateEnd( (z_stream *)m_pExpander );
		delete ((z_stream *)m_pExpander);
		
		m_pExpander = NULL;
	}
}

//----------------------------------------------------------------------------
//EOF
