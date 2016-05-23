/**
	@file TestMath.cpp

	(c)2006 Palestar Inc
	@author Richard Lyle @date 7/22/2006 10:23:53 AM
*/

#include "UnitTest.h"
#include "Math/Matrix33.h"

//---------------------------------------------------------------------------------------------------

void EulerToMatrix( const Vector3 & v, Matrix33 & f )
{
	f.setEuler( v );
	/*
	Matrix33 mx, my, mz;
	mx.setRotateX( v.x );			// Pitch
	my.setRotateY( v.y );			// Yaw
	mz.setRotateZ( v.z );			// Roll
	f = my * (mx * mz);
	*/
}

void MatrixToEuler( const Matrix33 & f, Vector3 & e )
{
	e = f.getEuler();
	// f.i = right vector
	// f.j = Up vector
	// f.k = Look vector

#if 0
	if (f.i.y > 0.998f )
	{
		e.y = atan2( f.k.x, f.k.z );
		e.z = PI2;
		e.x = 0.0f;
	}
	else if ( f.i.y < -0.998f )
	{
		e.y = atan2( f.k.x, f.k.z );
		e.z = -PI2;
		e.x = 0.0f;
	}
	else
	{
		e.x = atan2( -f.k.y, f.j.y );
		e.y = atan2( -f.i.z, f.i.x );
		e.z = asinf( f.i.y );
	}
#endif

#if 0
	// XYZ
    // rot =  cy*cz          -cy*sz           sy
    //        cz*sx*sy+cx*sz  cx*cz-sx*sy*sz -cy*sx
    //       -cx*cz*sy+sx*sz  cz*sx+cx*sy*sz  cx*cy
	
    if (f.i.z < 1.0f)
    {
        if (f.i.z > -1.0f)
        {
            e.x = -atan2f(-f.j.z,f.k.z);
            e.y = -ClampedASIN(f.i.z);
            e.z = atan2f(-f.i.y,f.i.x);
        }
        else
        {
            // WARNING.  Not unique.  XA - ZA = -atan2(r10,r11)
            e.x = atan2f(f.j.x,f.j.y);
            e.y = PI2;
            e.z = 0.0f;
        }
    }
    else
    {
        // WARNING.  Not unique.  XAngle + ZAngle = atan2(r10,r11)
        e.x = -atan2f(f.j.x,f.j.y);
        e.y = -PI2;
        e.z = 0.0f;
    }
#endif

}

//---------------------------------------------------------------------------------------------------

class TestMath : public UnitTest
{
public:
	TestMath( const char * pName ) : UnitTest( pName )
	{}

	void test()
	{
		//OldEulerToMatrix( e1, m2 );

		Matrix33 identity( 0.0f, 0.0f, 0.0f );
		Matrix33 my( 0.0f, -1.5f, 0.0f );
		Matrix33 mx( 0.5f, 0.0f, 0.0f );
		Matrix33 mz( 0.0f, 0.0f, PI / 2.0f );

		Vector3 e1, e2;
		Matrix33 m1, m2;

		const float fStep = PI / 16.0f;
		for(float fYaw = -PI + fStep; fYaw < PI; fYaw += fStep )
		{
			for( float fPitch = -PI + fStep; fPitch < PI; fPitch += fStep )
			{
				e1 = Euler( fPitch, fYaw, 0.0f );
				EulerToMatrix( e1, m1 );
				MatrixToEuler( m1, e2 );
				Test( e1 == e2 );
			}
		}
	}

} TEST_MATH( "TestMath" );

//---------------------------------------------------------------------------------------------------
//EOF
