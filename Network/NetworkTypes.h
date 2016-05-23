/**
	@file NetworkTypes.h
	@brief These data types provide more compact forms of serialization than normal data types. 

	These data types should only be used for server/client communications, when the data should be as compact as possible. These
	"Network" types should NOT be used for server to server communications, when bandwidth is not an issue.

	(c)2006 Palestar Inc
	@author Richard Lyle @date 6/27/2006 9:45:18 AM
*/

#ifndef NETWORKTYPES_H
#define NETWORKTYPES_H

#include "Standard/BitIterator.h"
#include "File/Stream.h"
#include "Math/Vector3.h"
#include "Debug/Error.h"

//---------------------------------------------------------------------------------------------------

template< typename C,int BITS,bool UNSIGNED >
class BitInteger
{
public:
	BitInteger() : m_nValue( 0 ) {};
	BitInteger( const C & a_nValue ) : m_nValue( a_nValue ) {};

	int         GetBits() const;                 // returns the template BITS value
	int			GetBytes() const;			     // returns the number of bytes to contain the bits
	int         ActualBits() const;              // Returns the actual total number of bits in C
	bool        IsUnsigned() const;
	C           GetMinValue() const;             // Returns the minimum value
	C           GetMaxValue() const;             // Returns the max value

	operator const C &() const;
	operator C &();

private:
	// Types
	enum {
		MAX_SIGNED_VALUE           = ((1 << (BITS -1)) - 1),
		MAX_UNSIGNED_VALUE         = ((1 << BITS) - 1),
		MIN_SIGNED_VALUE           = -MAX_SIGNED_VALUE,
		MIN_UNSIGNED_VALUE         = 0,
	};

	// Data
	C           m_nValue;
};

template< typename C,int BITS,bool UNSIGNED >
inline int BitInteger<C,BITS,UNSIGNED>::GetBits() const
{
	return BITS;
}

template< typename C,int BITS,bool UNSIGNED >
inline int BitInteger<C,BITS,UNSIGNED>::GetBytes() const
{
	return (BITS + 7) / 8;
}

template< typename C,int BITS,bool UNSIGNED >
inline int BitInteger<C,BITS,UNSIGNED>::ActualBits() const
{
	return sizeof(C) * 8;
}

template< typename C,int BITS,bool UNSIGNED >
inline bool BitInteger<C,BITS,UNSIGNED>::IsUnsigned() const
{
	return UNSIGNED;
}

template< typename C,int BITS,bool UNSIGNED >
inline C BitInteger<C,BITS,UNSIGNED>::GetMinValue() const
{
	return UNSIGNED ? MIN_UNSIGNED_VALUE : MIN_SIGNED_VALUE;
}

template< typename C,int BITS,bool UNSIGNED >
inline C BitInteger<C,BITS,UNSIGNED>::GetMaxValue() const
{
	return UNSIGNED ? MAX_UNSIGNED_VALUE : MAX_SIGNED_VALUE;
}

template< typename C,int BITS,bool UNSIGNED >
inline BitInteger<C,BITS,UNSIGNED>::operator const C &() const
{
	return m_nValue;
}

template< typename C,int BITS,bool UNSIGNED >
inline BitInteger<C,BITS,UNSIGNED>::operator C &()
{
	return m_nValue;
}

//---------------------------------------------------------------------------------------------------

template<typename C,int BITS,bool UNSIGNED>
inline const InStream & operator>>( const InStream & input, BitInteger<C,BITS,UNSIGNED> & a_nValue )
{				
	C nValue = 0;
	input.read( &nValue, (BITS + 7) / 8 );

	// If this is a signed value, and the high bit was set, turn on the remaining bits..
	if (!UNSIGNED && (nValue & (1 << (BITS - 1))) != 0 )
		nValue |= ~a_nValue.GetMaxValue();

	a_nValue = nValue;
	return input;																	
}																						

template<typename C,int BITS,bool UNSIGNED>
inline const OutStream & operator<<( const OutStream & output, const BitInteger<C,BITS,UNSIGNED> & a_nValue )
{		
	C nValue = a_nValue;
	if ( nValue > a_nValue.GetMaxValue() )
		nValue = a_nValue.GetMaxValue();
	else if ( nValue < a_nValue.GetMinValue() )
		nValue = a_nValue.GetMinValue();

	output.write( &nValue, (BITS + 7) / 8 );
	return output;																				
}

//---------------------------------------------------------------------------------------------------

// We cannot template with floats, so we have to do int for the MIN and MAX values of the float..
// MIN should always be <=0 and MAX should always be > 0

template< int BITS, int MIN, int MAX >
class BitFloat
{
public:
	BitFloat();
	BitFloat( const float & a_fValue );

	int         GetBits() const;
	int			GetBytes() const;
	float       GetMinValue() const;
	float       GetMaxValue() const;
	float       GetStep() const;

	operator const float &() const;
	operator float &();

private:
	float       m_fValue;
};

//---------------------------------------------------------------------------------------------------

template< int BITS, int MIN, int MAX >
inline BitFloat<BITS,MIN,MAX>::BitFloat() : m_fValue( 0.0f )
{}

template< int BITS, int MIN, int MAX >
inline BitFloat<BITS,MIN,MAX>::BitFloat( const float & a_fValue ) : m_fValue( a_fValue )
{}

template< int BITS, int MIN, int MAX >
inline int BitFloat<BITS,MIN,MAX>::GetBits() const
{
	return BITS;
}

template< int BITS, int MIN, int MAX >
inline int BitFloat<BITS,MIN,MAX>::GetBytes() const
{
	return (BITS + 7) / 8;
}

template< int BITS, int MIN, int MAX >
inline float BitFloat<BITS,MIN,MAX>::GetMinValue() const
{
	return (float)MIN;
}

template< int BITS, int MIN, int MAX >
inline float BitFloat<BITS,MIN,MAX>::GetMaxValue() const
{
	return (float)MAX;
}

template< int BITS, int MIN, int MAX >
inline float BitFloat<BITS,MIN,MAX>::GetStep() const
{
	return ((float)MAX - (float)MIN) / ( (1 << (BITS - 1)) - 1);
}

template< int BITS, int MIN, int MAX >
inline BitFloat<BITS,MIN,MAX>::operator const float &() const
{
	return m_fValue;
}

template< int BITS, int MIN, int MAX >
inline BitFloat<BITS,MIN,MAX>::operator float &()
{
	return m_fValue;
}

//---------------------------------------------------------------------------------------------------

template<int BITS,int MIN,int MAX>
inline const InStream & operator>>( const InStream & input, BitFloat<BITS,MIN,MAX> & a_fValue )
{			
	BitInteger< int, BITS, false > nValue;
	input >> nValue;

	a_fValue = a_fValue.GetStep() * nValue;
	return input;																	
}																						

template<int BITS,int MIN,int MAX>
inline const OutStream & operator<<( const OutStream & output, const BitFloat<BITS,MIN,MAX> & a_fValue )
{		
	BitInteger< int, BITS, false > nValue = (int)(a_fValue / a_fValue.GetStep());
	if ( nValue > nValue.GetMaxValue() )
		nValue = nValue.GetMaxValue();
	else if ( nValue < nValue.GetMinValue() )
		nValue = nValue.GetMinValue();
	output << nValue;

	return output;																				
}

//---------------------------------------------------------------------------------------------------

typedef BitInteger<int,2,false>              bi2;          // -1 to 1
typedef BitInteger<unsigned int,2,true>      bui2;         // 0 to 3
typedef BitInteger<int,3,false>              bi3;          // -3 to 3
typedef BitInteger<unsigned int,3,true>      bui3;         // 0 to 7
typedef BitInteger<int,4,false>              bi4;          // -7 to 7
typedef BitInteger<unsigned int,4,true>      bui4;         // 0 to 15
typedef BitInteger<int,5,false>              bi5;          // -15 to 15
typedef BitInteger<unsigned int,5,true>      bui5;         // 0 to 31
typedef BitInteger<int,6,false>              bi6;          // -31 to 31
typedef BitInteger<unsigned int,6,true>      bui6;         // 0 to 63
typedef BitInteger<int,7,false>              bi7;          // -63 to 64
typedef BitInteger<unsigned int,7,true>      bui7;         // 0 to 128
typedef BitInteger<int,8,false>              bi8;          // -127 to 127
typedef BitInteger<unsigned int,8,true>      bui8;         // 0 to 255
typedef BitInteger<int,9,false>              bi9;          // -255 to 255
typedef BitInteger<unsigned int,9,true>      bui9;         // 0 to 511
typedef BitInteger<int,10,false>             bi10;         // -511 to 512
typedef BitInteger<unsigned int,10,true>     bui10;        // 0 to 1023
typedef BitInteger<int,11,false>             bi11;         // -1023 to 1023
typedef BitInteger<unsigned int,11,true>     bui11;        // 0 to 2047
typedef BitInteger<int,12,false>             bi12;         // -2047 to 2047
typedef BitInteger<unsigned int,12,true>     bui12;        // 0 to 4095
typedef BitInteger<int,13,false>             bi13;         // -4095 to 4095
typedef BitInteger<unsigned int,13,true>     bui13;        // 0 to 8191
typedef BitInteger<int,14,false>             bi14;         // -8191 to 8191
typedef BitInteger<unsigned int,14,true>     bui14;        // 0 to 16383
typedef BitInteger<int,15,false>             bi15;         // -16383 to 16383
typedef BitInteger<unsigned int,15,true>     bui15;        // 0 to 32767
typedef BitInteger<int,16,false>             bi16;         // -32767 to 32767
typedef BitInteger<unsigned int,16,true>     bui16;        // 0 to 65535
typedef BitInteger<int,17,false>             bi17;         // -65535 to 65535
typedef BitInteger<unsigned int,17,true>     bui17;        // 0 to 131071
typedef BitInteger<int,18,false>             bi18;         // -131071 to 131071
typedef BitInteger<unsigned int,18,true>     bui18;        // 0 to 262143
typedef BitInteger<int,19,false>             bi19;         // -262143 to 262143
typedef BitInteger<unsigned int,19,true>     bui19;        // 0 to 524287
typedef BitInteger<int,20,false>             bi20;         // -524287 to 524287
typedef BitInteger<unsigned int,20,true>     bui20;        // 0 to 1048575
typedef BitInteger<int,21,false>             bi21;         // -1048575 to 1048575
typedef BitInteger<unsigned int,21,true>     bui21;        // 0 to 2097151
typedef BitInteger<int,22,false>             bi22;         // -2097151 to 2097151
typedef BitInteger<unsigned int,22,true>     bui22;        // 0 to 4194303
typedef BitInteger<int,23,false>             bi23;         // -4194303 to 4194303
typedef BitInteger<unsigned int,23,true>     bui23;        // 0 to 8388607
typedef BitInteger<int,24,false>             bi24;         // -8388607 to 8388607
typedef BitInteger<unsigned int,24,true>     bui24;        // 0 to 16777215
typedef BitInteger<int,25,false>             bi25;         // -16777215 to 16777215
typedef BitInteger<unsigned int,25,true>     bui25;        // 0 to 33554431
typedef BitInteger<int,26,false>             bi26;         // -33554431 to 33554431
typedef BitInteger<unsigned int,26,true>     bui26;        // 0 to 67108863
typedef BitInteger<int,27,false>             bi27;         // -67108863 to 67108863
typedef BitInteger<unsigned int,27,true>     bui27;        // 0 to 134217727
typedef BitInteger<int,28,false>             bi28;         // -134217727 to 134217727
typedef BitInteger<unsigned int,28,true>     bui28;        // 0 to 268435455
typedef BitInteger<int,29,false>             bi29;         // -268435455 to 268435455
typedef BitInteger<unsigned int,29,true>     bui29;        // 0 to 536870911
typedef BitInteger<int,30,false>             bi30;         // -536870911 to 536870911
typedef BitInteger<unsigned int,30,true>     bui30;        // 0 to 1073741823
typedef BitInteger<int,31,false>             bi31;         // -1073741823 to 1073741823
typedef BitInteger<unsigned int,31,true>     bui31;        // 0 to 2147483647

//---------------------------------------------------------------------------------------------------

typedef BitFloat<4,-1,1>		            bf4;   // uses 4-bits to represent a floating point number between -1.0 and 1.0
typedef BitFloat<8,-1,1>				    bf8;
typedef BitFloat<16,-1,1>					bf16;

//---------------------------------------------------------------------------------------------------

template< int BITS, int MIN, int MAX >
class BitVector3 : public Vector3
{
public:
	BitVector3() : Vector3( 0.0f, 0.0f, 0.0f )
	{}
	BitVector3( const BitVector3 & copy ) : Vector3( copy )
	{}
	BitVector3( const Vector3 & copy ) : Vector3( copy )
	{}
};

//---------------------------------------------------------------------------------------------------

template<int BITS,int MIN,int MAX>
inline const InStream & operator>>( const InStream & input, BitVector3<BITS,MIN,MAX> & a_vValue )
{		
	BitFloat<BITS,MIN,MAX> x,y,z;
	input >> x >> y >> z;

	a_vValue = Vector3( x, y, z );
	return input;																	
}																						

template<int BITS,int MIN,int MAX>
inline const OutStream & operator<<( const OutStream & output, const BitVector3<BITS,MIN,MAX> & a_vValue )
{		
	output << BitFloat<BITS,MIN,MAX>( a_vValue.x ) << 
		BitFloat<BITS,MIN,MAX>( a_vValue.y ) <<
		BitFloat<BITS,MIN,MAX>( a_vValue.z );
	return output;																				
}

//---------------------------------------------------------------------------------------------------

#endif

//---------------------------------------------------------------------------------------------------
//EOF
