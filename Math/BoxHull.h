/*
	BoxHull.h
	(c)2005 Palestar, Richard Lyle
*/

#ifndef BOX_HULL_H
#define BOX_HULL_H

#include "Standard/Limits.h"
#include "File/Stream.h"
#include "Math/Vector3.h"
#include "Math/Matrix33.h"
#include "Math/SphericalHull.h"

#include "MedusaDll.h"

#include <float.h>
#include <stdio.h>		// sscanf

//----------------------------------------------------------------------------

#pragma warning(disable:4275) // non dll-interface struct '_BoxHull' used as base for dll-interface class 'BoxHull'

#undef min		// undefine windows macros
#undef max

struct _BoxHull
{
	Vector3		min;
	Vector3		max;
};

class DLL BoxHull : public _BoxHull
{
public:
	// Construction
	BoxHull();
	BoxHull( const Vector3 & v1, const Vector3 & v2 );
	BoxHull( bool smallBox );	// true = small box, false = zero sized box
	BoxHull( const BoxHull & b );
	BoxHull( const SphericalHull & sphere );
	BoxHull( float size, const Vector3 & position );	

	BoxHull( const BoxHull &copy, const Vector3 &translate );
	BoxHull( const BoxHull &copy, const Matrix33 &rotate, const Vector3 &translate );
	BoxHull( const char * a_pHull );

	// Accessors
	operator			CharString() const;
	SphericalHull		sphericalHull() const;
	float				radius() const;
	float				lengthX() const;
	float				lengthY() const;
	float				lengthZ() const;

	bool				intersect( const Vector3 &origin,		// intersect a ray with the box 
							const Vector3 &ray, 
							Vector3 & intersect ) const;
	bool				intersect( const Vector3 & v, 
							float fRadius );					// sphere intersects with this box hull
	bool				intersect( const BoxHull & h ) const;	// do these hulls touch 

	void				clamp( Vector3 & v ) const;				// clamp a vector within the box
	bool				inBox( const Vector3 & v ) const;		// is vector inside the box
	bool				inBox( const Vector3 & v, float fRadius );
	bool				inBox( const BoxHull & h ) const;		// returns true if box is inside the other box, false if any part is outside
	bool				isValid() const;						// if max >= min returns true

	BoxHull 			operator|( const BoxHull & combine ) const;
	BoxHull				operator*(float scalar ) const;

	// Mutators
	void				setBox( const BoxHull & b );
	void				setBox( const Vector3 & v1 ,const Vector3 & v2 );
	void				setBox( const BoxHull & b, const Vector3 & translate );
	void				setBox( const BoxHull & b, const Matrix33 &rotate, const Vector3 &translate );

	bool				clip( const BoxHull & clip );			// clip this box against another hull, returns true if this hull is completely inside the other hull
																// after clipping, this hull will contain no space occupied by the other hull
	void				invert();								// swap min and max values
	void				validate();								// force max >= min
	void				resetBox();

	BoxHull &			addVertex( const Vector3 &vertex );
	BoxHull &			addBox( const BoxHull &box );
	BoxHull &			move( const Vector3 & offset );

	BoxHull &			operator=( const BoxHull & a_Assign );
	BoxHull &			operator=( const char * a_pHull );

	// Helpers
	Vector3				center() const;
};

IMPLEMENT_RAW_STREAMING( BoxHull );
IMPLEMENT_STREAMING( BoxHull );					// bit streaming operators

//----------------------------------------------------------------------------

inline SphericalHull BoxHull::sphericalHull() const
{
	Vector3 vecCenter = center();
	return SphericalHull( vecCenter, radius() );
}

inline float BoxHull::radius() const
{
	return sqrtf( Max( min.magnitude2(), max.magnitude2() ) );
}

inline float BoxHull::lengthX() const
{
	return max.x - min.x;
}

inline float BoxHull::lengthY() const
{
	return max.y - min.y;
}

inline float BoxHull::lengthZ() const
{
	return max.z - min.z;
}

inline bool BoxHull::intersect( const Vector3 & v, float fRadius )
{
	if ( min.x > (v.x + fRadius) || min.y > (v.y + fRadius) || min.z > (v.z + fRadius) )
		return false;
	if ( max.x < (v.x - fRadius) || max.y < (v.y - fRadius) || max.z < (v.z - fRadius) )
		return false;
	return true;
}

inline bool BoxHull::intersect( const BoxHull & h ) const
{
    if ( min.x > h.max.x || min.y > h.max.y || min.z > h.max.z )
        return false;
    if ( max.x < h.min.x || max.y < h.min.y || max.z < h.min.z )
        return false;
	return true;
}

inline bool BoxHull::inBox( const Vector3 & v) const
{
	return( v.x <= max.x && v.x >= min.x &&
		v.y <= max.y && v.y >= min.y &&
		v.z <= max.z && v.z >= min.z );
}

inline bool BoxHull::inBox( const Vector3 & v, float fRadius )
{
	return (v.x + fRadius) <= max.x &&
		(v.x - fRadius) >= min.x &&
		(v.y + fRadius) <= max.y &&
		(v.y - fRadius) >= min.y &&
		(v.z + fRadius) <= max.z &&
		(v.z - fRadius) >= min.z;
}

inline bool BoxHull::inBox( const BoxHull & h ) const
{
	// returns true if this hull is completely inside the other hull
	return( min.x > h.min.x && max.x < h.max.x &&
		min.y > h.min.y && max.y < h.max.y &&
		min.z > h.min.z && max.z < h.max.z );
}

inline bool BoxHull::isValid() const
{
	return max.x >= min.x && max.y >= min.y && max.z >= min.z;
}

inline BoxHull BoxHull::operator|( const BoxHull & combine ) const
{
	BoxHull	result( *this );
	result.addBox( combine );

	return( result );
}

inline BoxHull BoxHull::operator*(float scalar ) const
{
	return( BoxHull( min * scalar, max * scalar ) );
}

//-------------------------------------------------------------------------------

inline void BoxHull::setBox( const BoxHull &copy )
{
	min = copy.min;
	max = copy.max;
}

inline void BoxHull::setBox( const Vector3 & v1, const Vector3 & v2 )
{
	min.set( Min( v1.x, v2.x ), Min( v1.y, v2.y ), Min( v1.z, v2.z ) );
	max.set( Max( v1.x, v2.x ), Max( v1.y, v2.y ), Max( v1.z, v2.z ) );
}

inline void BoxHull::setBox( const BoxHull & copy, const Vector3 & translate )
{
	min = copy.min + translate;
	max = copy.max + translate;
}

inline void BoxHull::resetBox()
{
	const float RESET_VALUE = 10000000.0f;	
	min.set( RESET_VALUE, RESET_VALUE, RESET_VALUE );
	max.set( -RESET_VALUE, -RESET_VALUE, -RESET_VALUE );
}

inline BoxHull & BoxHull::addVertex( const Vector3 &vertex )
{
	if ( vertex.x < min.x )
		min.x = vertex.x;
	if ( vertex.x > max.x )
		max.x = vertex.x;

	if ( vertex.y < min.y )
		min.y = vertex.y;
	if ( vertex.y > max.y )
		max.y = vertex.y;

	if ( vertex.z < min.z )
		min.z = vertex.z;
	if ( vertex.z > max.z )
		max.z = vertex.z;

	return *this;
}

inline BoxHull & BoxHull::addBox( const BoxHull &box )
{
	addVertex( box.min );
	addVertex( box.max );
	return *this;
}

inline BoxHull & BoxHull::move( const Vector3 & offset )
{
	min += offset;
	max += offset;
	return *this;
}

inline BoxHull & BoxHull::operator=( const BoxHull & a_Assign )
{
	max = a_Assign.max;
	min = a_Assign.min;
	return *this;
}

inline BoxHull & BoxHull::operator=( const char * a_pHull )
{
	sscanf( a_pHull, "%f,%f,%f,%f,%f,%f", 
		&min.x, &min.y, &min.z,
		&max.x, &max.y, &max.z);
	return *this;
}

//----------------------------------------------------------------------------

inline Vector3 BoxHull::center() const
{
	return( (max + min) * 0.5f );
}

//----------------------------------------------------------------------------

#endif

//----------------------------------------------------------------------------
// EOF
