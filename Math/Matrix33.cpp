/*
	Matrix33.cpp
	(c)2005 Palestar, Richard Lyle
*/

#define MATH_DLL
#include "Math/Matrix33.h"
#include "Reflection/TypeTemplate.h"

#pragma warning(disable:4244) // D:\Projects\Hydra\Math\Matrix33.cpp(32) : warning C4244: '=' : conversion from 'double' to 'float', possible loss of data

const Matrix33 Matrix33::IDENTITY( true );

//----------------------------------------------------------------------------

Matrix33::Matrix33( const CharString & sIJK ) 
{
	sscanf( sIJK, "(%f,%f,%f),(%f,%f,%f),(%f,%f,%f)", 
		&i.x, &i.y, &i.z,
		&j.x, &j.y, &j.z,
		&k.x, &k.y, &k.z );

	// re-normalized conversion may have introduced some error...
	orthoNormalizeXY();
}

//----------------------------------------------------------------------------

Matrix33::operator CharString() const
{
	return CharString().format("(%f,%f,%f),(%f,%f,%f),(%f,%f,%f)", 
		i.x, i.y, i.z, 
		j.x, j.y, j.z,
		k.x, k.y, k.z );
}

//-------------------------------------------------------------------------------

Matrix33 & Matrix33::rotate( float fPitch, float fYaw, float fRoll, bool bNormalize /*= true*/ )
{
	Vector3		v; 
	float		sine,cosine;


	if (fYaw != 0.0f)			// RotateY
	{
		sine = sin(fYaw);
		cosine = cos(fYaw);

		v = i;
		i = (i * cosine) - (k * sine);
		k = (k * cosine) + (v * sine);

		if (bNormalize)
			orthoNormalizeXZ();
	}
	if (fPitch != 0.0f)			// RotateX
	{
		sine = sin(fPitch);
		cosine = cos(fPitch);
		
		v = j;
		j = (j * cosine) + (k * sine);
		k = (k * cosine) - (v * sine);

		if (bNormalize)
			orthoNormalizeYZ();
	}
	if (fRoll)					// RotateZ
	{
		sine = sin(fRoll);
		cosine = cos(fRoll);
		
		v = i;
		i = (i * cosine) + (j * sine);
		j = (j * cosine) - (v * sine);
		
		if (bNormalize)
			orthoNormalizeXY();
	}

	return( *this );

	//operator*=( Matrix33( fPitch, fYaw, fRoll ) );
	//if ( bNormalize )
	//	orthoNormalizeXY();

	//return *this;
}

Matrix33 & Matrix33::build( const Vector3 &v )
{
	k = v;
	k.normalize();

	i = Vector3( k.z, 0, -k.x );	// no roll
	if ( i.magnitude() < EPSILON )
	{
		// this case only happens when looking down the true Y axis, k = 0,1,0
		i = Vector3(1,0,0);
		j = Vector3(0,0,-k.y);
		return( *this );
	}

	j = k & i;
	j.normalize();
	i = j & k;

	return( *this );
}

//----------------------------------------------------------------------------
// EOF

