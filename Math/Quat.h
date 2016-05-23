/*
	Quat.h - Quaternion class
	(c)2005 Palestar, Richard Lyle
*/

#ifndef QUAT_H
#define QUAT_H

#include "Math/Matrix33.h"
#include "MedusaDll.h"

//-------------------------------------------------------------------------------

class DLL Quat
{
public:
	// Construction
	Quat();
	Quat( float x, float y, float z, float w );
	Quat( const CharString & sQuat );
	Quat( const Quat & copy );
	Quat( const Matrix33 & matrix );
	Quat( const Vector3 & axis, float angle );
	Quat( float yaw, float pitch, float roll );

	// Accessors
	float				operator[]( int n ) const;
	int					operator==( const Quat & q ) const;			// equal
	int					operator!=( const Quat & q ) const;			// not equal

	Quat				operator+(const Quat & v) const;	
	Quat				operator-(const Quat & v) const;
	Quat				operator*(const Quat & v) const;
	Quat				operator*(float scalar ) const;
	Quat				operator/(float divisor ) const;

	float				operator|(const Quat & q ) const;			// Dot
	Quat				operator-() const;							// Negate

	operator			Matrix33() const;							// convert to matrix
	Matrix33			getMatrix33() const;						// convert to a matrix
	Vector3				getEuler() const;							// returns the euler values in a vector, x = yaw, y = pitch, z = roll
	float				magnitude() const;							// length
	float				dot( const Quat & q ) const;				// dot product with another quaternion

	operator			CharString() const;

	// Mutators
	float &				operator[]( int n );

	Quat & 				operator+=( const Quat & q );				// add
	Quat &				operator-=( const Quat & q );				// subtract
	Quat &				operator*=( const Quat & q );				// multiply
	Quat &				operator*=( float scalar );					// 
	Quat &				operator/=( float divisor );				// 

	void				setMatrix33( const Matrix33 & m );
	void				setVector3( const Vector3 & axis, float angle );
	void				setEuler( float yaw, float pitch, float roll );
	void				setIdentity();
	void				normalize();

	// Data
	float				x, y, z, w;

	// Static
	static Quat			lerp( const Quat & a, const Quat & b, float u );	// Linear interpolation
	static Quat			slerp( const Quat & a, const Quat & b, float u );	// Spherical linear interpolation
};

IMPLEMENT_RAW_STREAMING( Quat );
IMPLEMENT_STREAMING( Quat );					// bit streaming operators

//----------------------------------------------------------------------------

inline Quat::Quat()
{}

inline Quat::Quat( float xx, float yy, float zz, float ww )
{
	x = xx;
	y = yy;
	z = zz;
	w = ww;
}

inline Quat::Quat( const Quat & q )
{
	x = q.x;
	y = q.y;
	z = q.z;
	w = q.w;
}

inline Quat::Quat( const Matrix33 & m )
{
	setMatrix33( m );
}

inline Quat::Quat( const Vector3 & axis, float angle )
{
	setVector3( axis, angle );
}

inline Quat::Quat( float yaw, float pitch, float roll )
{
	setEuler( yaw, pitch, roll );
}

//----------------------------------------------------------------------------

inline float Quat::operator[]( int n ) const
{
	return ((float *)this)[ n ];
}

inline int Quat::operator==( const Quat & q ) const
{
	return q.x == x && q.y == y && q.z == z && q.w == w;
}

inline int Quat::operator!=( const Quat & q ) const
{
	return q.x != x || q.y != y || q.z != z || q.w != w;
}

inline Quat Quat::operator+(const Quat & v) const
{
	return Quat( *this ) += v;
}

inline Quat Quat::operator-(const Quat & v) const
{
	return Quat( *this ) -= v;
}

inline Quat Quat::operator*(const Quat & v) const
{
	return Quat( *this ) *= v;
}

inline Quat Quat::operator*(float scalar ) const
{
	return Quat( *this ) *= scalar;
}

inline Quat Quat::operator/(float divisor ) const
{
	return Quat( *this ) /= divisor;
}

inline float Quat::operator|(const Quat & q ) const
{
	return dot( q );
}

inline Quat Quat::operator-() const
{
	return Quat( -x, -y, -z, -w );
}

inline Quat::operator Matrix33() const
{
	return getMatrix33();
}

inline Matrix33 Quat::getMatrix33() const
{
	return Matrix33( Vector3( 1 - 2*(y*y) - 2*(z*z), 2*(x*y+w*z), 2*(x*z-w*y)),
		Vector3(2*(x*y-w*z), 1 - 2*(x*x) - 2*(z*z), 2*(y*z+w*x)),
		Vector3(2*(x*z+w*y), 2*(y*z-w*x), 1 - 2*(x*x) - 2*(y*y)) );
}

inline Vector3 Quat::getEuler() const
{
	double sqw = w * w;
	double sqx = x * x;
	double sqy = y * y;
	double sqz = z * z;

	return Vector3( atan( 2.0 * (x * y + z * w) / (sqx - sqy - sqz + sqw)),	// yaw
		asin(-2.0 * (x * z - y * w) ),										// pitch
		atan( 2.0 * (y * z + x * w) / (-sqx - sqy + sqz + sqw)) );			// roll
}

inline float Quat::magnitude() const
{
	return sqrtf(w * w + x * x + y * y + z * z);
}

inline float Quat::dot( const Quat & q ) const
{
	return x * q.x + y * q.y + z * q.z + w * q.w;
}

//----------------------------------------------------------------------------

inline float & Quat::operator[]( int n )
{
	return ((float *)this)[ n ];
}

inline Quat & Quat::operator+=( const Quat & q )
{
	x += q.x;
	y += q.y;
	z += q.z;
	w += q.w;
	return *this;
}

inline Quat & Quat::operator-=( const Quat & q )
{
	x -= q.x;
	y -= q.y;
	z -= q.z;
	w -= q.w;
	return *this;
}

inline Quat & Quat::operator*=( const Quat & q )
{
	return operator=( Quat( w * q.x + x * q.w + y * q.z - z * q.y,
		w * q.y + y * q.w + z * q.x - x * q.z,
		w * q.z + z * q.w + x * q.y - y * q.x,
		w * q.w - x * q.x - y * q.y - z * q.z ) );
}

inline Quat & Quat::operator*=( float scalar )
{
	x *= scalar;
	y *= scalar;
	z *= scalar;
	w *= scalar;
	return *this;
}

inline Quat & Quat::operator/=( float scalar )
{
	float r = 1.0f / scalar;
	return operator*=( r );
}

inline void Quat::setIdentity()
{
	x = 0.0f;
	y = 0.0f;
	z = 0.0f;
	w = 1.0f;
}

inline void Quat::normalize()
{
	operator*=( 1.0f / magnitude() );
}

//----------------------------------------------------------------------------

inline Quat operator*( float s, const Quat & q )
{
	return q * s;
}

//-------------------------------------------------------------------------------



#endif

//-------------------------------------------------------------------------------
// EOF
