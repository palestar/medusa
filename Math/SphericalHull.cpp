/**
	@file SphericalHull.cpp

	(c)2005 Palestar Inc
	@author Richard Lyle @date 12/14/2010 10:44:13 PM
*/

#include "SphericalHull.h"

//---------------------------------------------------------------------------------------------------

bool SphericalHull::intersect( const Vector3 & from, const Vector3 & ray, Vector3 & intersect )
{
	// perform ray-to-sphere intersection, from Graphic Gems, page 388
	Vector3 EO( m_vCenter - from );
	float r = m_fRadius;
	float r2 = r * r;
	
	float fRayLength = ray.magnitude();
	if ( EO.magnitude() > (fRayLength + r) )
		return false;	// not close enough to intersect

	Vector3 V( ray );
	V *= 1.0f / fRayLength;		// make it a unit vector

	float v = EO | V;
	float v2 = v * v;
	float disc = r2 - ((EO | EO) - v2);

	if ( disc < 0 )
		return false;	// does intersect the sphere

	float d = sqrt( disc );
	if ( v < d )
		return false;	// we are inside or the sphere is behind us

	// intersects the sphere 
	intersect = from + (V * (v - d));
	return true;
}

//---------------------------------------------------------------------------------------------------
//EOF
