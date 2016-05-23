/*
	Helpers.h
	(c)2005 Palestar, Richard Lyle
*/

#ifndef MATH_HELPERS_H
#define MATH_HELPERS_H

#include "Constants.h"
#include "Vector3.h"

// Fix for Max SDK...
#undef RadToDeg
#undef DegToRad

//-------------------------------------------------------------------------------

inline float RadToDeg( float radians )				// convert from radians to degrees
{
	return (radians * ( 180 / PI ) ); 
}

inline float DegToRad( float degrees )				// convert from degrees to radians
{
	return( degrees * ( PI / 180 ) );
}

inline float NormalizeAngle( float fRadians )		// normalize an angle to be between -PI to PI
{
   int nCycles = (int)(fRadians / TwoPI);
   fRadians -= nCycles * TwoPI;

   if( fRadians < -PI )
      fRadians += TwoPI;

   return fRadians;
}

inline float AngleDelta( float lhs, float rhs )		// determine the difference between two angles (normalized)
													// the result is the direction lhs needs to move to equal rhs
{
	if ( lhs < rhs )
	{
		if ( lhs < (rhs - PI) )
			return( rhs - (lhs + TwoPI) );
		return( rhs - lhs );
	}

	// else lhs > rhs
	if ( lhs > (rhs + PI ) )
		return( rhs - (lhs - TwoPI) );
	return rhs - lhs;
}

inline float RandomFloat( float low, float high )
{
	if ( low > high )
	{
		float swap = low;
		low = high;
		high = low;
	}

	float d = (high - low) / float(RAND_MAX);
	return( low + (rand() * d) );
}

inline Vector3 RandomVector( float low, float high )
{
	return Vector3( RandomFloat( low, high ), RandomFloat( low, high ), RandomFloat( low, high ) );
}

inline Vector3 RandomVector( const Vector3 & min, const Vector3 & max )
{
	return( Vector3( RandomFloat( min.x, max.x),
		RandomFloat( min.y, max.y),
		RandomFloat( min.z, max.z) ) );
}

//-------------------------------------------------------------------------------

#endif

//-------------------------------------------------------------------------------
// EOF
