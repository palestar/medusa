/*
	LZW.cpp

	This class is used to compress / uncompress data, LZW implementation
	(c)2005 Palestar, Richard Lyle
*/

#include "Standard/Bits.h"
#include "Standard/LZW.h"

//----------------------------------------------------------------------------

Codec::FactoryTemplate<LZW>		LZW_FACTORY( "LZW" );

//----------------------------------------------------------------------------

class Encoder
{
public:
	// Construction
	Encoder( byte * pOutput, byte * pOutputEnd ) 
		: m_pOutput( pOutput ), m_pOutputEnd( pOutputEnd ), m_Rack( 0 ), m_Mask( 0x80 )
	{}
	~Encoder()
	{
		if ( m_Mask != 0x80 )
			*m_pOutput = m_Rack;
	}

	// Mutator
	bool encode( unsigned int value, int bits )
	{
		unsigned int mask = 1 << ( bits - 1 );
		while( mask != 0 )
		{
			if ( mask & value )
				m_Rack |= m_Mask;
			
			m_Mask >>= 1;
			if ( m_Mask == 0 )
			{
				if ( m_pOutput >= m_pOutputEnd )
					return false;		// end of output buffer has been reached, we can encode no further

				*m_pOutput++ = m_Rack;
				m_Rack = 0;
				m_Mask = 0x80;
			}
			mask >>= 1;
		}

		return true;
	}

	// Data
	byte *	m_pOutput;
	byte *	m_pOutputEnd;
	byte	m_Rack;
	byte	m_Mask;
};

class Decoder
{
public:
	// Construction
	Decoder( byte * input ) 
		: m_pInput( input ), m_Rack( 0 ), m_Mask( 0x80 )
	{}

	// Mutator
	unsigned int decode( int bits )
	{
		unsigned int mask = 1 << ( bits - 1 );
		unsigned int value = 0;

		while( mask != 0 )
		{
			if ( m_Mask == 0x80 )
				m_Rack = *m_pInput++;
			if ( m_Rack & m_Mask )
				value |= mask;

			m_Mask >>= 1;
			if ( m_Mask == 0 )
				m_Mask = 0x80;

			mask >>= 1;
		}

		return value;
	}

	// Data
	byte *	m_pInput;
	byte	m_Rack;
	byte	m_Mask;
};

//----------------------------------------------------------------------------

Codec *	LZW::clone()
{
	return new LZW();
}

int LZW::encode( const void * pInput, int nInputBytes, void * pOutput, int nOutputBytes, EncodeLevel nLevel )
{
	// check for negative or zero bytes
	if ( nInputBytes <= 0 )
		return 0;

	// cast the input
	byte * pIn = (byte *)pInput;
	// create an encoder
	Encoder output( (byte *)pOutput, ((byte *)pOutput) + nOutputBytes );

	// initialize the dictionary
	initializeDictionary();

	int stringCode = *pIn++;
	nInputBytes--;		// subtract for the first byte taken

	while( nInputBytes > 0 )
	{
		nInputBytes--;

		// get the next byte
		int character = *pIn++;
		
		// lookup child node and hash 
		Dictionary & d = hash( findChildNode( stringCode, character ) );
		if ( d.codeValue != -1 )
		{
			stringCode = d.codeValue;
		}
		else
		{
			d.codeValue = m_NextCode++;
			d.parentCode = stringCode;
			d.character = (char)character;

			output.encode( stringCode, m_CurrentCodeBits );
			stringCode = character;

			if ( m_NextCode > MAX_CODE )
			{
				if (! output.encode( FLUSH_CODE, m_CurrentCodeBits ) )
					return -1;	

				initializeDictionary();
			}
			else if ( m_NextCode > m_NextBumpCode )
			{
				if (! output.encode( BUMP_CODE, m_CurrentCodeBits ) )
					return -1;	

				m_CurrentCodeBits++;
				m_NextBumpCode <<= 1;
				m_NextBumpCode |= 1;
			}

		}
	}

	if (! output.encode( stringCode, m_CurrentCodeBits ) || !output.encode( END_OF_STREAM, m_CurrentCodeBits ) )
		return -1;		// error

	// return the encoded size
	return( (output.m_pOutput - ((byte *)pOutput) ) + 1 );
}

int	LZW::decode( const void * pInput, int nInputBytes, void * pOutput, int nOutputBytes )
{
	unsigned int count;
	unsigned int oldCode;
	unsigned int newCode;
	int	character;

	byte * pOutputEnd = (byte *)pOutput + nOutputBytes;
	byte * pInputEnd = (byte *)pInput + nInputBytes;

	// track the start so we can calculate the output bytes
	byte * pOut = (byte *)pOutput;
	byte * pStart = pOut;

	// create an decoder
	Decoder input( (byte *)pInput );
	while( 1 )
	{
		// initialize the dictionary
		initializeDictionary();

		if ( input.m_pInput >= pInputEnd )
			return -1;
		oldCode = input.decode( m_CurrentCodeBits );
		if ( oldCode == END_OF_STREAM )
			return( pOut - pStart );

		if ( pOut >= pOutputEnd )
			return -1;	

		character = oldCode;
		*pOut++ = oldCode;

		while( 1 )
		{
			if ( input.m_pInput >= pInputEnd )
				return -1;
			newCode = input.decode( m_CurrentCodeBits );
			if ( newCode == END_OF_STREAM )
				return( pOut - pStart );		// return the total number decoded
			if ( newCode == FLUSH_CODE )
				break;
			if ( newCode == BUMP_CODE )
			{
				m_CurrentCodeBits++;
				continue;
			}

			if ( newCode >= m_NextCode )
			{
				m_DecodeStack[ 0 ] = (char)character;
				count = decodeString( 1, oldCode );
			}
			else
				count = decodeString( 0, newCode );

			character = m_DecodeStack[ count - 1 ];
			while( count > 0 )
			{
				if ( pOut >= pOutputEnd )
					return -1;	
				*pOut++ = m_DecodeStack[ --count ];
			}

			Dictionary & d = hash( m_NextCode );

			d.parentCode = oldCode;
			d.character = (char)character;
			m_NextCode++;

			oldCode = newCode;
		}
	}
}

void LZW::reset()
{
	initializeDictionary();
}

//----------------------------------------------------------------------------

void LZW::initializeDictionary()
{
	for(int i=0;i<TABLE_SIZE;i++)
		hash( i ).codeValue = UNUSED;
	m_NextCode = FIRST_CODE;
	m_CurrentCodeBits = 9;
	m_NextBumpCode = 511;
}

#pragma warning( disable: 4018 ) // d:\projects\hydra\standard\lzw.cpp(146) : warning C4018: '>=' : signed/unsigned mismatch

unsigned int LZW::findChildNode( int parentCode, int childCharacter )
{
	int offset;

	int index = ( childCharacter << ( BITS - 8 ) ) ^ parentCode;
	if ( index == 0 )
		offset = 1;
	else
		offset = TABLE_SIZE - index;

	while( 1 )
	{
		Dictionary & d = hash( index );

		if ( d.codeValue == UNUSED ) 
			return index;
		if ( d.parentCode == parentCode && d.character == (char)childCharacter )
			return index;

		if ( index >= offset )
			index -= offset;
		else
			index += TABLE_SIZE - offset;
	}
}

unsigned int LZW::decodeString( unsigned int count, unsigned int code )
{
	while( code > 255 )
	{
		Dictionary & d = hash( code );

		m_DecodeStack[ count++ ] = d.character;
		code = d.parentCode;
	}

	m_DecodeStack[ count++ ] = (char) code;
	return( count );
}

//----------------------------------------------------------------------------
// EOF


