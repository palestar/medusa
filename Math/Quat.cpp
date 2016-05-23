/*
	Quat.cpp
	(c)2005 Palestar Inc, Richard Lyle
*/

#define MATH_DLL
#include "Math/Quat.h"
#include "Reflection/TypeTemplate.h"

#pragma warning( disable : 4996 )	// warning C4996: 'sscanf': This function or variable may be unsafe. Consider using sscanf_s instead. To disable deprecation, use _CRT_SECURE_NO_WARNINGS. See online help for details.

//----------------------------------------------------------------------------

Quat::Quat( const CharString & sQuat )
{
	sscanf( sQuat, "%f,%f,%f,%f", &x, &y, &z, &w );
}

//----------------------------------------------------------------------------

Quat::operator CharString() const
{
	return CharString().format("%.2f,%.2f,%.2f,%.2f",x,y,z,w);
}

//----------------------------------------------------------------------------

void Quat::setMatrix33( const Matrix33 & m )
{
	static int nxt[3] = { 1, 2, 0 };
	
	float tr = m[0][0] + m[1][1] + m[2][2];
	if ( tr > 0.0f )
	{
		float s = sqrtf( tr + 1.0 );
		w = s / 2.0f;

		s = 0.5f / s;
		x = (m[1][2] - m[2][1]) * s;
		y = (m[2][0] - m[0][2]) * s;
		z = (m[0][1] - m[1][0]) * s;
	}
	else
	{
		int i = 0;
		if (m[1][1] > m[0][0]) 
			i = 1;
		if (m[2][2] > m[i][i]) 
			i = 2;
		int j = nxt[i];
		int k = nxt[j];

		float s = sqrtf((m[i][i] - (m[j][j] + m[k][k])) + 1.0f);
      
		operator[]( i ) = s * 0.5f;
            
		if (s != 0.0) 
			s = 0.5f / s;

	    operator[]( 3 ) = (m[j][k] - m[k][j]) * s;
		operator[]( j ) = (m[i][j] + m[j][i]) * s;
        operator[]( k ) = (m[i][k] + m[k][i]) * s;
	}
}

void Quat::setVector3( const Vector3 & axis, float angle )
{
	float d = axis.magnitude();
	if ( d > 0.0f )
	{
		float s = sinf( angle * 0.5f) / d;

		x = axis.x * s;
		y = axis.y * s;
		z = axis.z * s;
		w = cosf(angle * 0.5f);
	}
}

void Quat::setEuler( float yaw, float pitch, float roll )
{
	float cosY = cosf(yaw / 2.0F);
	float sinY = sinf(yaw / 2.0F);
	float cosP = cosf(pitch / 2.0F);
	float sinP = sinf(pitch / 2.0F);
	float cosR = cosf(roll / 2.0F);
	float sinR = sinf(roll / 2.0F);

	x = cosR * sinP * cosY + sinR * cosP * sinY;
	y = cosR * cosP * sinY - sinR * sinP * cosY;
	z = sinR * cosP * cosY - cosR * sinP * sinY;
	w = cosR * cosP * cosY + sinR * sinP * sinY;
}

//----------------------------------------------------------------------------

Quat Quat::lerp( const Quat & a, const Quat & b, float u )
{
	Quat ret( a + (u * (b - a)) );
	ret.normalize();

	return ret;
}

Quat Quat::slerp( const Quat & a, const Quat & b, float u )
{
	const float theta = acosf( a.dot( b ) );
	const float t = 1 / sinf( theta );

	return t * (a * sinf( (1-u)*theta ) + b * sinf( u*theta ) );
}

//----------------------------------------------------------------------------
//EOF
