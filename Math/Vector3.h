/*
	Vector3.h
	(c)2005 Palestar, Richard Lyle
*/

#ifndef VECTOR_3_H
#define VECTOR_3_H

#include "Standard/Exception.h"
#include "Standard/Types.h"
#include "Standard/String.h"
#include "Standard/BitIterator.h"
#include "File/Stream.h"

#include "Constants.h"
#include "MedusaDll.h"

#include <math.h>
#include <stdio.h>		// sscanf

#pragma warning( disable: 4244 )	// conversion from 'double' to 'float', possible loss of data
#pragma warning( disable : 4996 )		// C4996: 'sscanf': This function or variable may be unsafe. Consider using sscanf_s instead. To disable deprecation, use _CRT_SECURE_NO_WARNINGS. See online help for details.

//-------------------------------------------------------------------------------

class DLL Vector3
{
public:
	//! Constants
	static const Vector3	ZERO;

	// Exceptions
	DECLARE_EXCEPTION( BadMagnitude );

	// Construction
	Vector3();
	Vector3( const Vector3 &v );
	Vector3( float x, float y, float z );
	Vector3( float fValue );
	Vector3( bool bZero );

	Vector3( const char * pXYZ );		// load from string in X,Y,Z format
	Vector3( const wchar * pXYZ );

	// Accessors
	float				operator[](int n ) const;

	float				magnitude() const;					// get vector magnitude
	float				magnitude2() const;					// get squared vector magnitude
	float				magnitudeXZ() const;				// get the magnitude of the XZ components
	float				angleXY() const;					// get yaw
	float				angleXZ() const;					// get roll
	float				angleYZ() const;					// get pitch

	bool				compare( const Vector3 &v, float epsilon = EPSILON ) const;

	bool				operator==(const Vector3 &v) const;
	bool				operator!=(const Vector3 &v) const;

	Vector3				operator+(const Vector3 &v) const;	
	Vector3				operator-(const Vector3 &v) const;
	Vector3				operator*(const Vector3 &v) const;
	Vector3				operator*(float scalar) const;
	Vector3				operator/(const Vector3 &v) const;
	Vector3				operator/(float divisor) const;

	Vector3				operator&(const Vector3 &v) const;	// Cross Product
	float				operator|(const Vector3 &v) const;	// Dot Product
	Vector3				operator-() const;					// Negate
	Vector3				operator~() const;					// Normalize

	operator			CharString() const;

	// Mutators
	float &				operator[](int n );

	Vector3 &			set(float x,float y,float z);		// set this vector
	Vector3 &			set(float v);						// set all components to value
	Vector3 &			zero();								// set all components to zero
	Vector3 &			normalize();						// throws BadMagnitude()
	Vector3 &			rotateXY( float angle );			// yaw the vector
	Vector3 & 			rotateXZ( float angle );			// roll the vector
	Vector3 & 			rotateYZ( float angle );			// pitch the vector

	Vector3 &			operator+=(const Vector3 &v);
	Vector3 &			operator-=(const Vector3 &v);
	Vector3 &			operator*=(const Vector3 &v);
	Vector3 &			operator*=(float scalar);
	Vector3 &			operator/=(const Vector3 &v);
	Vector3 &			operator/=(float divisor);
	Vector3 &			operator&=(const Vector3 &v);		// Cross Product

	Vector3 &			operator=( const Vector3 & copy );
	Vector3 &			operator=( const char * pXYZ );
	Vector3 &			operator=( const wchar * pXYZ );

	// Data
	float				x;
	float				y;
	float				z;
};

IMPLEMENT_STREAMING( Vector3 );					// bit streaming operators
IMPLEMENT_RAW_STREAMING( Vector3 );

//----------------------------------------------------------------------------
// Helpers

inline Vector3 Cross( const Vector3 &a, const Vector3 &b )
{
	return( a & b );
}

inline float Dot( const Vector3 &a, const Vector3 &b )
{
	return( a | b );
}

//-------------------------------------------------------------------------------

inline Vector3::Vector3() 
{}

inline Vector3::Vector3(const Vector3 &v)
{
	(*this) = v;
}

inline Vector3::Vector3(float xx,float yy,float zz)
{
	set( xx,yy,zz );
}

inline Vector3::Vector3( float value )
{
	x = y = z = value;
}

inline Vector3::Vector3( bool zero )
{
	if ( zero )
		x = y = z = 0.0f;
}

inline Vector3::Vector3( const char * pXYZ )
{
	operator=( pXYZ );
}

inline Vector3::Vector3( const wchar * pXYZ )
{
	operator=( pXYZ );
}

//-------------------------------------------------------------------------------

inline float Vector3::operator[](int n ) const
{
	return( ((float *)this)[ n ] );
}

inline float Vector3::magnitude() const
{
	return( sqrtf( (x * x) + (y * y) + (z * z) ) );
}

inline float Vector3::magnitude2() const
{
	return( (x * x) + (y * y) + (z * z) );
}

inline float Vector3::magnitudeXZ() const
{
	return( sqrtf( (x * x) + (z * z) ) );
}

inline float Vector3::angleXY() const
{
	return( atan2( x, y ) );
}

inline float Vector3::angleXZ() const
{
	return( atan2( x, z ) );
}

inline float Vector3::angleYZ() const
{
	return( atan2( y, z ) );
}

inline bool Vector3::compare( const Vector3 &v, float epsilon /*= EPSILON*/ ) const
{
	return( ((*this) - v).magnitude2() < epsilon );
}

inline bool Vector3::operator==(const Vector3 &v) const
{
	return( compare( v ) );
}

inline bool Vector3::operator!=(const Vector3 &v) const
{
	return( ! compare( v ) );
}

inline Vector3 Vector3::operator+(const Vector3 &v) const
{
	return( Vector3( x + v.x, y + v.y, z + v.z ) );
}

inline Vector3 Vector3::operator-(const Vector3 &v) const
{
	return( Vector3( x - v.x, y - v.y, z - v.z ) );
}

inline Vector3 Vector3::operator*(const Vector3 &v) const
{
	return( Vector3( x * v.x, y * v.y, z * v.z ) );
}

inline Vector3 Vector3::operator*(float scalar) const
{
	return( Vector3( x * scalar, y * scalar, z * scalar ) );
}

inline Vector3 Vector3::operator/(const Vector3 &v) const
{
	return( Vector3( x / v.x, y / v.y, z / v.z) );
}

inline Vector3 Vector3::operator/(float divisor) const
{
	return( Vector3( x / divisor, y / divisor, z / divisor ) );
}

inline Vector3 Vector3::operator&(const Vector3 &v) const	// Cross
{
	return( Vector3( (y * v.z) - (z * v.y),
		(z * v.x) - (x * v.z),
		(x * v.y) - (y * v.x) ) );
}

inline float Vector3::operator|(const Vector3 &v) const		// Dot
{
	return( (x * v.x) + (y * v.y) + (z * v.z) );
}

inline Vector3 Vector3::operator-() const					// Negate
{
	return( Vector3( -x, -y, -z ) );
}

inline Vector3 Vector3::operator~() const
{
	float mag = magnitude();
	if ( mag > 0.0f )
		return operator*( 1.0f / mag );

	return Vector3(0,1,0);
}

//-------------------------------------------------------------------------------

inline float & Vector3::operator[](int n )
{
	return( ((float *)this)[ n ] );
}

inline Vector3 & Vector3::set(float xx,float yy,float zz)
{
	x = xx;
	y = yy;
	z = zz;

	return( *this );
}

inline Vector3 & Vector3::set(float v)
{
	x = y = z = v;
	return( *this );
}

inline Vector3 & Vector3::zero()
{
	set( 0.0f );
	return( *this );
}

inline Vector3 & Vector3::normalize()
{
	float mag = magnitude();
	if ( mag > 0.0f )
		return operator*=( 1.0f / mag );

	set(0,1,0);
	return( *this );
}

inline Vector3 & Vector3::rotateXY( float angle )
{
	float sine = sin(angle);
	float cosine = cos(angle);
	float vx = x;

	x = x * cosine;
	x += y * sine;
	y = y * cosine;
	y -= vx * sine;
	return *this;
}

inline Vector3 & Vector3::rotateXZ( float angle )
{
	float sine = sin(angle);
	float cosine = cos(angle);	
	float vx = x;

	x = x * cosine;
	x -= z * sine;
	z = z * cosine;
	z += vx * sine;
	return *this;
}

inline Vector3 & Vector3::rotateYZ( float angle )
{
	float sine = sin(angle);
	float cosine = cos(angle);	
	float vy = y;
	
	y = y * cosine;
	y += z * sine;
	z = z * cosine;
	z -= vy * sine;
	return *this;
}

inline Vector3 & Vector3::operator+=(const Vector3 &v)
{
	x += v.x;
	y += v.y;
	z += v.z;

	return( *this );
}

inline Vector3 & Vector3::operator-=(const Vector3 &v)
{
	x -= v.x;
	y -= v.y;
	z -= v.z;

	return( *this );
}

inline Vector3 & Vector3::operator*=(const Vector3 &v)
{
	x *= v.x;
	y *= v.y;
	z *= v.z;

	return( *this );
}

inline Vector3 & Vector3::operator*=(float scalar)
{
	x *= scalar;
	y *= scalar;
	z *= scalar;

	return( *this );
}

inline Vector3 & Vector3::operator/=(const Vector3 &v)
{
	x /= v.x;
	y /= v.y;
	z /= v.z;

	return( *this );
}

inline Vector3 & Vector3::operator/=(float divisor)
{
	x /= divisor;
	y /= divisor;
	z /= divisor;

	return( *this );
}

inline Vector3 & Vector3::operator&=(const Vector3 &v)	// Cross
{
	(*this) = (*this) & v;
	return( *this );
}

inline Vector3 & Vector3::operator=( const Vector3 & copy )
{
	x = copy.x;
	y = copy.y;
	z = copy.z;
	return *this;
}

inline Vector3 & Vector3::operator=( const char * pXYZ )
{
	sscanf( pXYZ, "%f,%f,%f", &x, &y, &z );
	return *this;
}

inline Vector3 & Vector3::operator=( const wchar * pXYZ )
{
	swscanf( pXYZ,L"%f,%f,%f", &x, &y, &z );
	return *this;
}

//---------------------------------------------------------------------------------------------------

#endif

//----------------------------------------------------------------------------
// EOF
