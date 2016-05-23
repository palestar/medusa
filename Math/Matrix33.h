/*
	Matrix33.h
	(c)2005 Palestar, Richard Lyle
*/

#ifndef MATRIX_33_H
#define MATRIX_33_H

#include "Vector3.h"
#include "Euler.h"
#include "MedusaDll.h"

//----------------------------------------------------------------------------

class DLL Matrix33
{
public:
	// Constants
	static const Matrix33	IDENTITY;

	// Construction
	Matrix33();
	Matrix33( const Matrix33 &m);
	Matrix33( const Vector3 & i,const Vector3 & j,const Vector3 & k );
	Matrix33( float fPitch, float fYaw, float fRoll );
	Matrix33( const Euler & e );
	Matrix33( const Vector3 & k );
	Matrix33( bool bIdentity );

	Matrix33( const CharString & sIJK );									// convert from matrix in string form..

	// Accessors
	const Vector3 &		operator[](int n ) const;

	Matrix33			operator*(const Matrix33 & m) const;		// FrameToFrame
	Matrix33			operator%(const Matrix33 & m) const;		// FrameFromFrame
	Vector3				operator*(const Vector3 & v) const;			// TransToFrame
	Vector3				operator%(const Vector3 & v ) const;		// TransFromFrame, same as ~frame * vector
	Matrix33			operator*(float scalar ) const;				// scale frame
	Matrix33			operator~() const;							// transpose()
	Matrix33			operator-(const Matrix33 & m) const;		// subtract
	Matrix33			operator+(const Matrix33 & m) const;		// add

	float				scale() const;								// get the scale of the matrix
	bool				isIdentity() const;							// is this matrix an identity

	Euler				getEuler() const;							// convert to euler coordinates, x = Pitch, y = Yaw, z = Roll (in radians)

	operator CharString() const;									// convert into string 

	// Mutators
	Vector3 &			operator[]( int n );
	Matrix33 &			operator=(const Matrix33 & m );
	Matrix33 &			operator*=(const Matrix33 & m);
	Matrix33 &			operator+=(const Matrix33 & m);
	Matrix33 &			operator-=(const Matrix33 & m);

	Matrix33 &			identity();									// set this matrix to identity
	Matrix33 &			transpose();								// swap Columns with Rows	
	
	Matrix33 &			orthoNormalizeXY();							// K is true direction
	Matrix33 &			orthoNormalizeXZ();							// J is true
	Matrix33 &			orthoNormalizeYZ();							// I is true

	Matrix33 &			setRotateX( float a_fRadians );				// build a matrix rotated along the X axis
	Matrix33 &			setRotateY( float a_fRadians );				// Y
	Matrix33 &			setRotateZ( float a_fRadians );				// Z
	Matrix33 &			setRotate( const Vector3 & a_vAxis,			// build a matrix rotated along the given axis
							float a_fRadians );						
	Matrix33 &			setEuler( const Euler & e );				// build this matrix from euler coordinates, e.x = Pitch, e.y = Yaw, e.z = Roll

	Matrix33 &			normalize();								// same as orthoNormalizeXY()

	Matrix33 &			rotate( float fPitch, 
							float fYaw, float fRoll, 
							bool bNormalize = true );				// incrementally rotate this matrix
	Matrix33 &			build( const Vector3 &v );					// build a matrix, k = v

	// Data
	Vector3				i;		
	Vector3				j;		
	Vector3				k;	
};

IMPLEMENT_RAW_STREAMING( Matrix33 );
IMPLEMENT_STREAMING( Matrix33 );					// bit streaming operators

//----------------------------------------------------------------------------

inline Matrix33::Matrix33() 
{}

inline Matrix33::Matrix33( const Matrix33 & m)
{
	i = m.i;
	j = m.j;
	k = m.k;
}

inline Matrix33::Matrix33( const Vector3 &ii,const Vector3 &jj,const Vector3 &kk)
{
	i = ii;
	j = jj;
	k = kk;
}

inline Matrix33::Matrix33( float fPitch, float fYaw, float fRoll )
{
	setEuler( Euler( fPitch, fYaw, fRoll ) );
}

inline Matrix33::Matrix33( const Euler & e )
{
	setEuler( e );
}

inline Matrix33::Matrix33( const Vector3 &k )
{
	// build a matrix with the Z in the direction of the vector
	build( k );
}

inline Matrix33::Matrix33( bool bIdentity )
{
	// initialize the matrix to identity
	if ( bIdentity )
		identity();
}

//----------------------------------------------------------------------------

inline const Vector3 & Matrix33::operator[]( int n ) const
{
	return ((const Vector3 *)this)[ n ];
}

inline Matrix33 Matrix33::operator*(const Matrix33 &m) const			// FrameToFrame
{
	return Matrix33( (*this) * m.i, (*this) * m.j, (*this) * m.k );
}

inline Matrix33 Matrix33::operator%(const Matrix33 &m) const			// FrameToFrame
{
	return Matrix33( Vector3(i.x,j.x,k.x), Vector3(i.y,j.y,k.y), Vector3(i.z,j.z,k.z) ) * m;
}

inline Vector3 Matrix33::operator*(const Vector3 &v) const				// TransToFrame
{
	return Vector3( v | i, v | j, v | k );
}

inline Vector3 Matrix33::operator%(const Vector3 & v ) const
{
	return Matrix33( Vector3(i.x,j.x,k.x), Vector3(i.y,j.y,k.y), Vector3(i.z,j.z,k.z) ) * v;
}

inline Matrix33	Matrix33::operator*( float scalar ) const
{
	if ( scalar == 1.0f )
		return Matrix33( *this );	// scaler is 1.0f, skip math
	return Matrix33( i * scalar, j * scalar, k * scalar );
}

inline Matrix33 Matrix33::operator~() const							// transpose()
{
	return Matrix33( *this ).transpose();
}

inline Matrix33 Matrix33::operator-(const Matrix33 & m) const
{
	return Matrix33( i - m.i, j - m.j, k - m.k );
}

inline Matrix33 Matrix33::operator+(const Matrix33 & m) const
{
	return Matrix33( i + m.i, j + m.j, k + m.k );
}

inline float Matrix33::scale() const
{
	// NOTE: this only works for a matrix that has been scaled uniform
	return i.magnitude(); // Max( i.magnitude(), j.magnitude() ), k.magnitude() );
}

inline bool Matrix33::isIdentity() const
{
	// this logic assumes the matrix is normalized
	return i.x == 1.0f && j.y == 1.0f && k.z == 1.0f;
}

inline float ClampedASIN( float fValue )
{
    if ( -1.0f < fValue )
    {
        if ( fValue < 1.0f )
            return asinf(fValue);
        else
            return PI2;
    }
    else
    {
        return -PI2;
    }
}

inline Euler Matrix33::getEuler() const
{
	Euler e;
	if (i.y > 0.998f )
	{
		e.m_fYaw = atan2( k.x, k.z );
		e.m_fRoll = PI2;
		e.m_fPitch = 0.0f;
	}
	else if ( i.y < -0.998f )
	{
		e.m_fYaw = atan2( k.x, k.z );
		e.m_fRoll = -PI2;
		e.m_fPitch = 0.0f;
	}
	else
	{
		e.m_fPitch = atan2( -k.y, j.y );
		e.m_fYaw = atan2( -i.z, i.x );
		e.m_fRoll = asinf( i.y );
	}
	return e;
}

//----------------------------------------------------------------------------

inline Vector3 & Matrix33::operator[]( int n )
{
	return ((Vector3 *)this)[ n ];
}

inline Matrix33 & Matrix33::operator=(const Matrix33 & m )
{
	i = m.i;
	j = m.j;
	k = m.k;
	return *this;
}

inline Matrix33 & Matrix33::operator*=(const Matrix33 & m)
{
	*this = (*this) * m;
	return *this;
}

inline Matrix33 & Matrix33::operator+=(const Matrix33 & m)
{
	i += m.i;
	j += m.j;
	k += m.k;

	return *this;
}

inline Matrix33 & Matrix33::operator-=(const Matrix33 & m)
{
	i -= m.i;
	j -= m.j;
	k -= m.k;

	return *this;
}

inline Matrix33 & Matrix33::identity()
{
	i.set(1.0f,0.0f,0.0f);
	j.set(0.0f,1.0f,0.0f);
	k.set(0.0f,0.0f,1.0f);
	return( *this );
}

inline Matrix33 & Matrix33::transpose()
{
	if ( isIdentity() )
		return *this;

	*this = Matrix33( Vector3(i.x,j.x,k.x), Vector3(i.y,j.y,k.y), Vector3(i.z,j.z,k.z) );
	return *this;
}

inline Matrix33 & Matrix33::orthoNormalizeXY()
{
	k.normalize();
	i = (j & k).normalize();
	j = (k & i);
	return *this;
}

inline Matrix33 & Matrix33::orthoNormalizeXZ()
{
	j.normalize();
	k = (i & j).normalize();
	i = (j & k);
	return *this;
}

inline Matrix33 & Matrix33::orthoNormalizeYZ()
{
	i.normalize();
	k = (i & j).normalize();
	j = (k & i);
	return *this;
}

inline Matrix33 & Matrix33::setRotateX( float a_fRadians )
{
    float fCos = cosf(a_fRadians);
    float fSin = sinf(a_fRadians);
    i = Vector3(1.0f,0.0f,0.0f);
	j = Vector3(0.0,fCos,-fSin);
    k = Vector3(0.0,fSin,fCos);
	return *this;
}

inline Matrix33 & Matrix33::setRotateY( float a_fRadians )
{
    float fCos = cosf(a_fRadians);
    float fSin = sinf(a_fRadians);
    i = Vector3(fCos,0.0f,fSin);
    j = Vector3(0.0f,1.0f,0.0f);
	k = Vector3(-fSin,0.0f,fCos);
	return *this;
}

inline Matrix33 & Matrix33::setRotateZ( float a_fRadians )
{
    float fCos = cosf(a_fRadians);
    float fSin = sinf(a_fRadians);
    i = Vector3(fCos,-fSin,0.0f);
	j = Vector3(fSin,fCos,0.0f);
	k = Vector3(0.0f,0.0f,1.0f);
	return *this;
}

inline Matrix33 & Matrix33::setRotate( const Vector3 & a_vAxis, float a_fRadians )
{
	Vector3 vAxis = a_vAxis;
	vAxis.normalize();

	float fSin = sinf( a_fRadians );
	float fCos = cosf( a_fRadians );
	float fInvCos = 1.0f - fCos;

	i.x = (fInvCos * vAxis.x * vAxis.x) + (fCos);
	i.y = (fInvCos * vAxis.x * vAxis.y) + (fSin * vAxis.z);
	i.z = (fInvCos * vAxis.x * vAxis.z) - (fSin * vAxis.y);

	j.x = (fInvCos * vAxis.x * vAxis.y) - (fSin * vAxis.z);
	j.y = (fInvCos * vAxis.y * vAxis.y) + (fCos);
	j.z = (fInvCos * vAxis.y * vAxis.z) + (fSin * vAxis.x);

	k.x = (fInvCos * vAxis.x * vAxis.z) + (fSin * vAxis.y);
	k.y = (fInvCos * vAxis.y * vAxis.z) - (fSin * vAxis.x);
	k.z = (fInvCos * vAxis.z * vAxis.z) + (fCos);

	return *this;
}

// set this matrix from euler coordinates, e.x = Pitch, e.y = Yaw, e.z = Roll
inline Matrix33 & Matrix33::setEuler( const Euler & e )
{
	Matrix33 mx, my, mz;
	mx.setRotateX( e.m_fPitch );
	my.setRotateY( e.m_fYaw );	
	mz.setRotateZ( e.m_fRoll );	

	return operator=( my * (mx * mz) );
}

inline Matrix33 & Matrix33::normalize()
{
	return orthoNormalizeXY();
}

//----------------------------------------------------------------------------



#endif

//----------------------------------------------------------------------------
// EOF
