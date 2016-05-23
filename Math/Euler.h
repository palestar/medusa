/**
	@file Euler.h
	@brief Euler angle class.

	(c)2006 Palestar Inc
	@author Richard Lyle @date 7/20/2007 2:04:54 PM
*/

#ifndef EULER_H
#define EULER_H

#include "Helpers.h"
#include "Vector3.h"
#include "MedusaDll.h"

#include <stdio.h>		// sscanf

#pragma warning( disable : 4996 )		// C4996: 'sscanf': This function or variable may be unsafe. Consider using sscanf_s instead. To disable deprecation, use _CRT_SECURE_NO_WARNINGS. See online help for details.

//---------------------------------------------------------------------------------------------------

class DLL Euler 
{
public:
	// Data
	float		m_fPitch;
	float		m_fYaw;
	float		m_fRoll;

	// Construction
	Euler() : 
		m_fPitch( 0.0f ), 
		m_fYaw( 0.0f ), 
		m_fRoll( 0.0f )
	{}
	Euler( float a_fPitch, float a_fYaw, float a_fRoll ) : 
		m_fPitch( a_fPitch ), 
		m_fYaw( a_fYaw ), 
		m_fRoll( a_fRoll )
	{}
	Euler( const Euler & a_Copy ) : 
		m_fPitch( a_Copy.m_fPitch ), 
		m_fYaw( a_Copy.m_fYaw ), 
		m_fRoll( a_Copy.m_fRoll )
	{}
	Euler( const Vector3 & a_Vector ) : 
		m_fPitch( a_Vector.x ), 
		m_fYaw( a_Vector.y ), 
		m_fRoll( a_Vector.z )
	{}
	Euler( const char * pEuler )
	{
		operator=( pEuler );
	}

	// Accessors
	bool compare( const Euler & a_Compare, float fEpsilon = PI / 180.0f ) const
	{
		return AngleDelta( m_fPitch, a_Compare.m_fPitch ) <= fEpsilon &&
			AngleDelta( m_fYaw, a_Compare.m_fYaw ) <= fEpsilon &&
			AngleDelta( m_fRoll, a_Compare.m_fRoll ) <= fEpsilon;
	}

	bool operator==(const Euler & a_Compare ) const
	{
		return compare( a_Compare );
	}

	bool operator!=(const Euler & a_Compare ) const
	{
		return !compare( a_Compare );
	}

	Euler operator+(const Euler & a_Add ) const
	{
		Euler R( *this );
		R += a_Add;
		return R;
	}

	Euler operator-(const Euler & a_Sub ) const
	{
		Euler R( *this );
		R -= a_Sub;
		return R;
	}

	Euler operator*( float a_fScale ) const
	{
		Euler R( *this );
		R *= a_fScale;
		return R;
	}

	Euler operator/( float a_fScale ) const
	{
		Euler R( *this );
		R /= a_fScale;
		return R;
	}

	operator Vector3() const
	{
		return Vector3( m_fPitch, m_fYaw, m_fRoll );
	}

	operator CharString() const
	{
		return CharString().format( "%f,%f,%f", m_fPitch, m_fYaw, m_fRoll );
	}

	// Mutators
	Euler & normalize() 
	{
		m_fPitch = NormalizeAngle( m_fPitch );
		m_fYaw = NormalizeAngle( m_fYaw );
		m_fRoll = NormalizeAngle( m_fRoll );
		return *this;
	}
	Euler & zero()
	{
		m_fPitch = m_fYaw = m_fRoll = 0.0f;
		return *this;
	}

	Euler & operator=( const Euler & a_Copy )
	{
		m_fPitch = a_Copy.m_fPitch;
		m_fYaw = a_Copy.m_fYaw;
		m_fRoll = a_Copy.m_fRoll;
		return *this;
	}

	Euler & operator=( const char * pXYZ )
	{
		sscanf( pXYZ, "%f,%f,%f", &m_fPitch, &m_fYaw, &m_fRoll );
		return *this;
	}
	
	Euler & operator+=( const Euler & a_Add )
	{
		m_fPitch = NormalizeAngle( m_fPitch + a_Add.m_fPitch );
		m_fYaw = NormalizeAngle( m_fYaw + a_Add.m_fYaw );
		m_fRoll = NormalizeAngle( m_fRoll + a_Add.m_fRoll );
		return *this;
	}

	Euler & operator-=( const Euler & a_Sub )
	{
		m_fPitch = AngleDelta( a_Sub.m_fPitch, m_fPitch );
		m_fYaw = AngleDelta( a_Sub.m_fYaw, m_fYaw );
		m_fRoll = AngleDelta( a_Sub.m_fRoll, m_fRoll );
		return *this;
	}

	Euler & operator*=( float a_fScale )
	{
		m_fPitch *= a_fScale;
		m_fYaw *= a_fScale;
		m_fRoll *= a_fScale;
		return *this;
	}

	Euler & operator/=( float a_fScale )
	{
		m_fPitch /= a_fScale;
		m_fYaw /= a_fScale;
		m_fRoll /= a_fScale;
		return *this;
	}
};

inline const InStream & operator>>( const InStream & input, Euler & object )
{		
	if ( input.version() < SV_20090104 )
	{
		input >> object.m_fPitch 
			>> object.m_fYaw 
			>> object.m_fRoll;
		return input;
	}

	char nPitch, nYaw, nRoll;
	input >> nPitch >> nYaw >> nRoll;

	object.m_fPitch = nPitch * (PI / 127.0f);
	object.m_fYaw = nYaw * (PI / 127.0f);
	object.m_fRoll = nRoll * (PI / 127.0f);

	return input;
}																

inline const OutStream & operator<<( const OutStream & output, const Euler & object)
{			
	if ( output.version() < SV_20090104 )
	{
		output << object.m_fPitch
			<< object.m_fYaw
			<< object.m_fRoll;
		return output;
	}

	char nPitch = (char)(object.m_fPitch / (PI / 127.0f));
	char nYaw = (char)(object.m_fYaw / (PI / 127.0f));
	char nRoll = (char)(object.m_fRoll / (PI / 127.0f ));

	output << nPitch << nYaw << nRoll;
	return output;														
}

//IMPLEMENT_RAW_STREAMING( Euler );

#endif

//---------------------------------------------------------------------------------------------------
//EOF
