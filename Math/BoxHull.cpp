/*
	BoxHull.cpp
	(c)2005 Palestar, Richard Lyle
*/

#define MATH_DLL
#include "Math/BoxHull.h"

//----------------------------------------------------------------------------

BoxHull::BoxHull()
{
	resetBox();
}

BoxHull::BoxHull( const Vector3 & vMin, const Vector3 & vMax )
{
	setBox( vMin, vMax );
}

BoxHull::BoxHull( bool bSmallBox )
{
	min = bSmallBox ? Vector3( -1.0f ) : Vector3( 0.0f );
	max = bSmallBox ? Vector3( 1.0f ) : Vector3( 0.0f );
}

BoxHull::BoxHull( const BoxHull & b ) : _BoxHull()
{
	setBox( b );
}

BoxHull::BoxHull( const SphericalHull & hull )
{
	min = Vector3( -hull.radius() * 0.5f ) + hull.center();
	max = Vector3( hull.radius() * 0.5f ) + hull.center();
}

BoxHull::BoxHull( float size, const Vector3 & position )
{
	min = Vector3( -size * 0.5f ) + position;
	max = Vector3( size * 0.5f ) + position;
}

BoxHull::BoxHull( const BoxHull & b, const Matrix33 &rotate, const Vector3 &translate )
{
	setBox( b, rotate, translate );
}

BoxHull::BoxHull( const BoxHull & b, const Vector3 &translate )
{
	setBox( b, translate );
}

BoxHull::BoxHull( const char * a_pHull )
{
	operator=( a_pHull );
}

//---------------------------------------------------------------------------------------------------

BoxHull::operator CharString() const
{
	return CharString().format( "%.2f,%.2f,%.2f,%.2f,%.2f,%.2f",
		min.x, min.y, min.z, max.x, max.y, max.z );
}

//----------------------------------------------------------------------------

bool BoxHull::intersect( const Vector3 &origin, const Vector3 &ray, Vector3 &intersect ) const
{
	// Fast Ray-Box Intersection
	// by Andrew Woo
	// from "Graphics Gems I", Academic Press, 1990

	enum Quadrant
	{
		RIGHT,
		LEFT,
		MIDDLE
	} quadrant[ 3 ];

	bool		inside = true;
	float		candidatePlane[ 3 ];

	// Find candidate planes; this loop can be avoided if
   	// rays cast all from the eye(assume perpsective view) 
	int i =0;
	for (;i<3;i++)
	{
		if ( origin[i] < min[i] )
		{
			quadrant[i] = LEFT;
			candidatePlane[i] = min[i];
			inside = false;
		}
		else if ( origin[i] > max[i] )
		{
			quadrant[i] = RIGHT;
			candidatePlane[i] = max[i];
			inside = false;
		}
		else
			quadrant[i] = MIDDLE;
	}

	// Ray origin inside bounding box 
	if( inside )	
	{
		intersect = origin;
		return true;
	}

	// Calculate T distances to candidate planes 
	float maxT[ 3 ];
	for (i=0;i<3;i++)
	{
		if (quadrant[i] != MIDDLE && ray[i] != 0.0f )
			maxT[i] = (candidatePlane[i] - origin[i] ) / ray[i];
		else
			maxT[i] = -1.0f;
	}

	// Get largest of the maxT's for final choice of intersection 
	int whichPlane = 0;
	for (i=1;i<3;i++)
		if (maxT[whichPlane] < maxT[i])
			whichPlane = i;

	// Check final candidate actually inside box 
	if (maxT[whichPlane] < 0.0f) 
		return false;
	
	for (i=0;i<3;i++)
	{
		if (whichPlane != i) 
		{
			intersect[i] = origin[i] + (maxT[whichPlane] * ray[i]);
			if ( intersect[i] < min[i] || intersect[i] > max[i] )
				return false;

		} 
		else 
			intersect[i] = candidatePlane[i];
	}

	return true;
}

void BoxHull::clamp( Vector3 & v ) const
{
	if ( v.x > max.x )
		v.x = max.x;
	if ( v.z > max.z )
		v.z = max.z;
	if ( v.x < min.x )
		v.x = min.x;
	if ( v.z < min.z )
		v.z = min.z;
}

//----------------------------------------------------------------------------

void BoxHull::setBox( const BoxHull &copy, const Matrix33 &rotate, const Vector3 &translate )
{
	resetBox();

	Vector3 v1( copy.min );
	Vector3 v2( copy.max );

	addVertex( (rotate * Vector3( v1.x,v1.y,v1.z )) + translate );
	addVertex( (rotate * Vector3( v2.x,v1.y,v1.z )) + translate );
	addVertex( (rotate * Vector3( v1.x,v2.y,v1.z )) + translate );
	addVertex( (rotate * Vector3( v2.x,v2.y,v1.z )) + translate );
	addVertex( (rotate * Vector3( v1.x,v1.y,v2.z )) + translate );
	addVertex( (rotate * Vector3( v2.x,v1.y,v2.z )) + translate );
	addVertex( (rotate * Vector3( v1.x,v2.y,v2.z )) + translate );
	addVertex( (rotate * Vector3( v2.x,v2.y,v2.z )) + translate );
}

bool BoxHull::clip( const BoxHull & h )
{
	if ( inBox( h ) )
	{
		// box is completeld clipped, zero this hull
		min = max = Vector3( 0.0f );
		return true;
	}

	if (! intersect( h ) )
		return false;		// boxes do not intersect

	// clip this box so no part of this hull is inside the other hull
	if ( min.x < h.max.x && max.x > h.max.x )
		min.x = h.max.x;
	else if ( max.x > h.min.x && min.x < h.min.x )
		max.x = h.min.x;

	if ( min.y < h.max.y && max.y > h.max.y )
		min.y = h.max.y;
	else if ( max.y > h.min.y && min.y < h.min.y )
		max.y = h.min.y;

	if ( min.z < h.max.z && max.z > h.max.z )
		min.z = h.max.z;
	else if ( max.z > h.min.z && min.z < h.min.z )
		max.z = h.min.z;

	return false;
}

void BoxHull::invert()
{
	Swap( min, max );
}

void BoxHull::validate()
{
	if ( min.x > max.x )
		Swap( min.x, max.x );
	if ( min.y > max.y )
		Swap( min.y, max.y );
	if ( min.z > max.z )
		Swap( min.z, max.z );
}

//----------------------------------------------------------------------------
// EOF
