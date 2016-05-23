/*
	Limits.h
	(c)2005 Palestar, Richard Lyle
*/

#ifndef LIMITS_H
#define LIMITS_H

//----------------------------------------------------------------------------

template<class T>
const T &	Max( const T &a, const T &b)
{
	return( a > b ? a : b );
}

template<class T>
const T &	Min( const T &a, const T &b)
{
	return( a > b ? b : a );
}

template<class T>
inline T ClampMax( T value, T max )
{
	return( value > max ? max : value );
}

template<class T>
inline T ClampMin( T value, T min )
{
	return( value < min ? min : value );
}

template<class T>
const T &	Clamp( const T &value, const T &min, const T &max)
{
	return( value < max ? value > min ? value : min : max );
}

template<class T>
void Swap( T & v1, T & v2 )
{
	T swap = v1;
	v1 = v2;
	v2 = swap;
}

//----------------------------------------------------------------------------


#endif

//----------------------------------------------------------------------------
// EOF
