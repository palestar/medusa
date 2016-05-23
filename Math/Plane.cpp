/*
	Plane.cpp
	(c)2005 Palestar, Richard Lyle
*/

#define MATH_DLL
#include "Math/Plane.h"
#include "Reflection/TypeTemplate.h"

//----------------------------------------------------------------------------

Plane::Plane( const CharString & sXYZD )
{
	sscanf( sXYZD, "%f,%f,%f,%f", &normal.x, &normal.y, &normal.z, &distance );
}

//----------------------------------------------------------------------------

Matrix33 Plane::matrix() const
{
	Vector3 up( (Vector3(1,0,0) & normal) );
	float magnitude = up.magnitude();

	if (magnitude > EPSILON )
	{
		up *= 1.0f / magnitude;
		return( Matrix33(normal & up,up,normal) );
	}

	return( Matrix33( Vector3(0,0,1), Vector3(0,1,0), normal/*Vector3(1,0,0)*/ ) );
}

Plane::operator CharString() const
{
	return CharString().format("%.2f,%.2f,%.2f,%.2f",normal.x,normal.y,normal.z,distance);
}

//----------------------------------------------------------------------------

Plane & Plane::transform( const Matrix33 &rotate, const Vector3 &translate )
{
	Vector3 center( ~matrix() * Vector3( 0, 0, -distance) );
	set( rotate * normal, (rotate * center) + translate );
	return( *this );
}

//----------------------------------------------------------------------------
// EOF
