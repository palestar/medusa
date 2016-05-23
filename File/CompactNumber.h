/**
	@file CompactNumber.h
	@brief This class is used to contain u32 value, but when seiralized it will use 8, 16, 24, or 32 bits to stream. 
	This gives a maximum of 29 bits (536,870,911) that can be stored in this number.

	The primary use of this class is for storing or transmitting numbers that are typically very small so we 
	only use a byte to transmit them 99% of the time we hope.

	(c)2012 Palestar Inc
	@author Richard Lyle @date 3/25/2012 2:44:34 PM
*/

#ifndef COMPACTNUMBER_H
#define COMPACTNUMBER_H

#include "Debug/Log.h"

#include "Stream.h"
#include "MedusaDll.h"		// include last

//---------------------------------------------------------------------------------------------------

class DLL CompactNumber 
{
public:
	DECLARE_EXCEPTION( Overflow );

	CompactNumber() : m_nNumber( 0 )
	{}
	CompactNumber( const CompactNumber & a_Copy ) : m_nNumber( a_Copy.m_nNumber )
	{}
	CompactNumber( u32 a_nAssign ) : m_nNumber( a_nAssign )
	{
		if ( m_nNumber >= (1 << 29) )
			throw Overflow();
	}

	//! Data
	u32				m_nNumber;
};

//---------------------------------------------------------------------------------------------------

inline const InStream & operator>>( const InStream & input, CompactNumber & n )
{
	if ( (input.filter() & FF_TEXT) == 0 )
	{
		u8 nData[ 4 ];
		input >> nData[0];
		if ( nData[0] & 0x80 )
		{
			nData[0] &= 0x7f;

			input >> nData[1];
			if ( nData[1] & 0x80 )
			{
				nData[1] &= 0x7f;

				input >> nData[2];
				if ( nData[2] & 0x80 )
				{
					nData[2] &= 0x7f;

					input >> nData[3];
					n = nData[0] | (nData[1] << 7) | (nData[2] << 14) | (nData[3] << 21); // 0-536,870,911 (29 bits)
				}
				else
					n = nData[0] | (nData[1] << 7) | (nData[2] << 14); // 0-2,097,151 (21 bits)
			}
			else
				n = nData[0] | (nData[1] << 7); // 0-16,383 (14 bits)
		}
		else
			n = nData[0]; // 0-127 (7 bits)
	}
	else
		input >> n.m_nNumber;

	return input;
}

inline const OutStream & operator<<( const OutStream & output, const CompactNumber & n )
{
	if ( (output.filter() & FF_TEXT) == 0 )
	{
		if ( n.m_nNumber > 127 )
		{
			if ( n.m_nNumber > 16383 )
			{
				if ( n.m_nNumber > 2097151 )
				{
					if ( n.m_nNumber > 536870911 )
					{
						LOG_ERROR( "CompactNumber", "%llu is out of range to serialize with CompactNumbr", n.m_nNumber );
						throw File::FileError();		// out of range 
					}

					output << (u8)((n.m_nNumber & 0x7f) | 0x80) 
						<< (u8)(((n.m_nNumber >> 7) & 0x7f) | 0x80)
						<< (u8)(((n.m_nNumber >> 14) & 0x7f) | 0x80)
						<< (u8)((n.m_nNumber >> 21) & 0xff);
				}
				else
				{
					output << (u8)((n.m_nNumber & 0x7f) | 0x80) 
						<< (u8)(((n.m_nNumber >> 7) & 0x7f) | 0x80)
						<< (u8)((n.m_nNumber >> 14) & 0x7f);
				}
			}
			else
			{
				output << (u8)((n.m_nNumber & 0x7f) | 0x80) 
					<< (u8)((n.m_nNumber >> 7) & 0x7f);
			}
		}
		else
			output << (u8)n.m_nNumber;
	}
	else
		output << n.m_nNumber;

	return output;
}

//---------------------------------------------------------------------------------------------------

#endif

//---------------------------------------------------------------------------------------------------
//EOF
