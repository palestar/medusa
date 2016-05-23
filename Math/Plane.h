/*
	Plane.h
	(c)2005 Palestar, Richard Lyle
*/

#ifndef PLANE_H
#define PLANE_H

#include "Math/Vector3.h"
#include "Math/Matrix33.h"
#include "MedusaDll.h"

//----------------------------------------------------------------------------

class DLL Plane
{
public:
	// Enumerations
	enum Side
	{
		FRONT		= 0x1,
		BACK		= 0x2,		
		ON			= 0x3,	// FRONT|BACK
	};

	// Construction
	Plane();
	Plane( const Plane &p );
	Plane( const Vector3 &n, float distance );
	Plane( const Vector3 &n, const Vector3 &c );
	Plane( const CharString & sXYZD );

	// Accessors
	bool				intersect( const Vector3 &origin, const Vector3 &ray, Vector3 &intersection ) const;
	Matrix33			matrix() const;							// build a matrix which can be used to transform points to/from the plane
	float				dot( const Vector3 &point ) const;
	Side				compare( const Vector3 &point ) const;
	Side				compare( const Vector3 & point, float fRadius ) const;

	operator			CharString() const;

	// Mutators
	Plane &				transform( const Matrix33 &rotate, const Vector3 &translate );
	Plane &				set( const Vector3 &n, float distance );
	Plane &				set( const Vector3 &n, const Vector3 &c );

	// Data
	Vector3				normal;
	float				distance;
};

IMPLEMENT_RAW_STREAMING( Plane );
IMPLEMENT_RAW_STREAMING( Plane::Side );
IMPLEMENT_STREAMING( Plane );					// bit streaming operators
IMPLEMENT_STREAMING( Plane::Side );		

//----------------------------------------------------------------------------

inline Plane::Plane()
{}

inline Plane::Plane( const Plane &p )
{
	*this = p;
}

inline Plane::Plane( const Vector3 &n, float distance )
{
	set( n, distance );
}

inline Plane::Plane( const Vector3 &n, const Vector3 &c )
{
	set( n, c );
}

//----------------------------------------------------------------------------

inline bool Plane::intersect( const Vector3 &origin, const Vector3 &ray, Vector3 &intersection ) const
{
	float t = normal | ray;	
	if (t < EPSILON && t > -EPSILON)		// ray & plane parallel
		return(false);

	t = -((normal | origin) + distance) / t;
	if (t < 0.0f)							// ray behind the plane
		return(false);

	intersection = origin + (ray * t);
	return(true);
}

inline float Plane::dot( const Vector3 &point ) const
{
	return( (point | normal) + distance );
}

inline Plane::Side Plane::compare( const Vector3 &point ) const
{
	float d = dot(point);

	if (d > EPSILON)
		return FRONT;
	if (d < -EPSILON)
		return BACK;

	return ON;
}

inline Plane::Side Plane::compare( const Vector3 & point, float fRadius ) const
{
	float d = dot( point );
	if ( d < -fRadius )
		return BACK;
	if ( d > fRadius )
		return FRONT;

	return ON;
}

//----------------------------------------------------------------------------

inline Plane & Plane::set( const Vector3 &n, float d )
{
	normal = n;
	distance = d;
	return( *this );
}

inline Plane & Plane::set( const Vector3 &n, const Vector3 &c )
{
	normal = n;
	distance = -(normal | c);
	return( *this );
}

//----------------------------------------------------------------------------



#endif

//----------------------------------------------------------------------------
// EOF
