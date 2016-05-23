/*
	SphericalHull.h
	(c)2005 Palestar, Richard Lyle
*/

#ifndef SPHERICAL_HULL_H
#define SPHERICAL_HULL_H

#include "Math/Vector3.h"
#include "File/Stream.h"
#include "MedusaDll.h"

//----------------------------------------------------------------------------

class DLL SphericalHull
{
public:
	// Construction
	SphericalHull();
	SphericalHull( Vector3 & vCenter, float fRadius );

	// Accessors
	const Vector3 &		center() const;
	float				radius() const;

	// Mutators
	void				setSphere( const Vector3 & vCenter, float fRadius );
	// ray to sphere intersection test, returns true if the ray intersects this sphere.. the intersect point
	// is returned back in the intersect variable.
	bool				intersect( const Vector3 & from, 
							const Vector3 & ray, Vector3 & intersect );

private:
	// Data
	Vector3				m_vCenter;
	float				m_fRadius;
};

IMPLEMENT_RAW_STREAMING( SphericalHull );

//----------------------------------------------------------------------------

inline SphericalHull::SphericalHull() : m_vCenter( 0.0f ), m_fRadius( 0.0f )
{}

inline SphericalHull::SphericalHull( Vector3 & vCenter, float fRadius ) : m_vCenter( vCenter ), m_fRadius( fRadius )
{}

inline const Vector3 & SphericalHull::center() const
{
	return m_vCenter;
}

inline float SphericalHull::radius() const
{
	return m_fRadius;
}

inline void SphericalHull::setSphere( const Vector3 & vCenter, float fRadius )
{
	m_vCenter = vCenter;
	m_fRadius = fRadius;
}

//----------------------------------------------------------------------------



#endif

//----------------------------------------------------------------------------
// EOF
