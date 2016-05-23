/*
	UUE.cpp
	(c)2005 Palestar Inc, Richard Lyle
*/

#include "Standard/UUE.h"
#include <assert.h>



//----------------------------------------------------------------------------

// signle-character encode
#define ENC(c) ((c) ? ((c) & 077) + ' ': '`')
// single-character decode 
#define DEC(c)	(((c) - ' ') & 077)

//----------------------------------------------------------------------------

UUE::UUE() : m_pEncoded( NULL )
{}

UUE::UUE( const UUE & copy ) : m_pEncoded( NULL )
{
	operator=( copy );
}

UUE::UUE( const byte * pData, int bytes ) : m_pEncoded( NULL )
{
	encode( pData, bytes );
}

UUE::~UUE()
{
	if ( m_pEncoded != NULL )
	{
		delete [] m_pEncoded;
		m_pEncoded = NULL;
	}
}

void UUE::encode( const byte * pEncode, int bytes )
{
	if ( m_pEncoded != NULL )
	{
		delete [] m_pEncoded;
		m_pEncoded = NULL;
	}

	if ( bytes > 0 )
	{
		char * pEncodedBuffer = new char[ (bytes * 2) + 16 ];
		char * pEncoded = pEncodedBuffer;

		while( bytes > 0 )
		{
			// encode up to 45 bytes of data per line
			int n = bytes > 45 ? 45 : bytes;
			bytes -= n;

			// encode the length
			*pEncoded++ = ENC( n );
			for(int i=0;i<n;i+=3)
			{
				pEncoded[0] = ENC( pEncode[0] >> 2 );
				pEncoded[1] = ENC( (pEncode[0] << 4) & 060 | (pEncode[1] >> 4) & 017 );
				pEncoded[2] = ENC( (pEncode[1] << 2) & 074 | (pEncode[2] >> 6) & 03 );
				pEncoded[3] = ENC( pEncode[2] & 077 );

				pEncode += 3;
				pEncoded += 4;
			}
			*pEncoded++ = '\n';
		}

		// null terminate the string
		*pEncoded++ = 0;

		// store the encoded data in the member string
		m_pEncoded = new char[ strlen( pEncodedBuffer ) + 1 ];
		strcpy( m_pEncoded, pEncodedBuffer );

		// release the encoding buffer
		delete [] pEncodedBuffer;
	}
}

UUE & UUE::operator=( const UUE & copy )
{
	if ( m_pEncoded != NULL )
	{
		delete [] m_pEncoded;
		m_pEncoded = NULL;
	}

	int nLength = strlen( copy.m_pEncoded );
	m_pEncoded = new char[ nLength ];
	strcpy( m_pEncoded, copy.m_pEncoded );

	return *this;
}

//----------------------------------------------------------------------------

UUD::UUD() : m_pDecoded( NULL ), m_DecodedBytes( 0 )
{}

UUD::UUD( const UUD & copy ) : m_pDecoded( NULL ), m_DecodedBytes( 0 )
{
	operator=( copy );
}

UUD::UUD( const char * pString ) : m_pDecoded( NULL ), m_DecodedBytes( 0 )
{
	if ( !decode( pString ) )
		throw BadEncoding();
}

UUD::~UUD()
{
	release();
}

void UUD::release()
{
	if ( m_pDecoded != NULL )
	{
		delete [] m_pDecoded;
		m_pDecoded = NULL;
		m_DecodedBytes = 0;
	}
}

bool UUD::decode( const char * pEncoded )
{
	if ( m_pDecoded != NULL )
	{
		delete m_pDecoded;
		m_pDecoded = NULL;
	}

	// allocate a buffer for the decoded bytes
	int		decodedBytes = 0;
	int		expectedBytes = ((strlen( pEncoded ) + 2)/3) << 4;
	byte * pDecodedBuffer = new byte[ expectedBytes ];
	byte * pDecoded = pDecodedBuffer;

	bool decoded = false;

	const char * pDecode = pEncoded;
	while( true )
	{
		if ( *pDecode == 0 )
		{
			// end of encoding
			assert( decodedBytes <= expectedBytes );		// check for buffer overrun!
			decoded = true;
			break;
		}

		// read the length
		int n = DEC( *pDecode++ );
		if ( n <= 0 )
			break;

		decodedBytes += n;

		while (n > 0) 
		{
			*pDecoded++ = DEC(pDecode[0]) << 2 | DEC(pDecode[1]) >> 4;
			if ( n >= 2 )
				*pDecoded++ = DEC(pDecode[1]) << 4 | DEC(pDecode[2]) >> 2;
			if ( n >= 3 )
				*pDecoded++ = DEC(pDecode[2]) << 6 | DEC(pDecode[3]);

			pDecode += 4;
			n -= 3;
		}

		assert( *pDecode == '\n' );
		pDecode++;
	}

	if ( ! decoded )
	{
		// failed to decode, release the buffer and return failure
		delete pDecodedBuffer;
		return false;
	}

	m_DecodedBytes = decodedBytes;
	m_pDecoded = new byte[ m_DecodedBytes ];
	memcpy( m_pDecoded, pDecodedBuffer, m_DecodedBytes );

	delete pDecodedBuffer;

	return true;
}

UUD & UUD::operator=( const UUD & copy )
{
	release();

	m_DecodedBytes = copy.m_DecodedBytes;
	m_pDecoded = new byte[ m_DecodedBytes ];
	memcpy( m_pDecoded, copy.m_pDecoded, m_DecodedBytes );

	return *this;
}

//----------------------------------------------------------------------------
//EOF
