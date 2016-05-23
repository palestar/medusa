/*
	Vector3.cpp

	3D Vector class
	(c)2005 Palestar Inc, Richard Lyle
*/

#define MATH_DLL
#include "Math/Vector3.h"
#include "Reflection/TypeTemplate.h"

//----------------------------------------------------------------------------

const Vector3 Vector3::ZERO( 0.0f, 0.0f, 0.0f );

Vector3::operator CharString() const
{
	return CharString().format("%.2f,%.2f,%.2f",x,y,z);
}

//----------------------------------------------------------------------------
//EOF

