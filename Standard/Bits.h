/*
	Bits.h

	Bit Template routines
	(c)2005 Palestar, Richard Lyle
*/

#ifndef BITS_H
#define BITS_H

#include "Standard/Types.h"
#include "MedusaDll.h"

//-------------------------------------------------------------------------------

template<class T> 
inline T BitSet(T &a, const T b)
{
	return( a |= b );
}

template<class T> 
inline T BitClear(T &a,const T b)
{
	return( a &= ~b );
}

template<class T> 
inline T BitToggle(T &a,const T b)
{
	return( a ^= b );
}

template<class T> 
inline bool	BitTrue(const T a,const T b)
{
	return( a & b ? true : false );
}

template<class T> 
inline T BitTest(const T a,const T b)
{
	return( a & b);
}

template<class T> 
inline T BitTestAny(const T a,const T b)
{
	return( BitTest(a,b) );
}

template<class T>
inline T BitTestAll(const T a,const T b)
{
	return( BitTest(a,b) == b );
}

template<class T>
inline T BitTestNot(const T a,const T b)
{
	return( BitTest(a,b) == 0 );
}

template<class T>
inline T BitTestAllNot(const T a,const T b)
{
	return( BitTestNot(a,b) );
}

template<class T>
inline T BitTestAnyNot(const T a,const T b)
{
	return( BitTest(a,b) != b);
}

template<class T>
inline T BitTestExclusive(const T a,const T b)
{
	return( a == b );
}

//---------------------------------------------------------------------------------------------

template<class T>
inline int CountBitsInMask( T mask )
{
	int bitCount = 0;
	while( mask )
	{
		mask = mask & (mask - 1);
		bitCount++;
	}

	return( bitCount );
}

template<class T> 
inline int GetFirstBit( T mask )		// used for finding a shift value for the mask
{
	T i = 0;
	for(;i<(sizeof(T) * 8);i++)
		if ( (1 << i) & mask)
			break;

	return i;	// if not found, this should return 32 in the case of an 'int' 
}

template<class T>
inline int GetLastBit( T mask )
{
	T i = (sizeof(T) * 8) - 1;
	for(;i>=0;i--)
		if ( (1 << i) & mask)
			break;

	return i;	// if not found will return -1
}

//----------------------------------------------------------------------------



#endif

//----------------------------------------------------------------------------
// EOF
