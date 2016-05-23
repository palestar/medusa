/*
	NodeGlare.cpp

	This class is used to display a glare in the render, it can be used
	for bright lights, stars, or explosions.

	(c)2005 Palestar, Richard Lyle
*/

#define RENDER3D_DLL
#include "Debug/Assert.h"
#include "Display/PrimitiveWindow.h"
#include "Display/PrimitiveMaterial.h"
#include "Render3D/NodeGlare.h"

//----------------------------------------------------------------------------

IMPLEMENT_FACTORY( NodeGlare, NodeTransform );
REGISTER_FACTORY_KEY(  NodeGlare, 4095679464999974514LL );

BEGIN_PROPERTY_LIST( NodeGlare, NodeTransform )
	ADD_PROPERTY( m_Color );
	ADD_PROPERTY( m_Intensity );
	ADD_PROPERTY( m_IntensityV );
	ADD_PROPERTY( m_IntensityA );
	ADD_PROPERTY( m_Focus );
	ADD_PROPERTY( m_Falloff );
END_PROPERTY_LIST();

//----------------------------------------------------------------------------

NodeGlare::NodeGlare()
{
	m_Color = Color(255,255,255,255);
	m_Intensity = 1.0f;
	m_IntensityV = m_IntensityA = 0.0f;
	m_Focus = 0.0f;
	m_Falloff = 0.0f;
}

//----------------------------------------------------------------------------

const Vector3	LOOK( 0,0,1 );
const RectFloat GLARE_UV(0,0,0,0);
 
void NodeGlare::render( RenderContext &context, const Matrix33 & frame, const Vector3 & position )
{
	if ( context.isShadowPass() )
		return;	

	NodeTransform::render( context, frame, position );

	Vector3 positionVS( context.worldToView( position ) );
	if ( positionVS.z >= 1.0f )
	{
		DisplayDevice * pDisplay = context.display();
		ASSERT( pDisplay );

		Vector3 ray( positionVS );
		float falloff = m_Falloff * ray.magnitude();

		ray.normalize();

		float t = context.time();
		float t2 = t * t;

		// calculate the intensity by the dot product of the look direction with the direction of the glare node
		float dot = ray | LOOK;
		dot -= m_Focus;

		float intensity = dot * ( m_Intensity + ( m_IntensityV * t ) + (m_IntensityA * t2 ) );
		if ( intensity > 0.0f )
		{
			int alpha = Clamp( int( (intensity * 255) - (falloff * 255) ), 0, 255 );
			if ( alpha > 0 )
			{
				Color glareColor( m_Color );
				glareColor.alpha = alpha;

				// set the material is no texture, but additive
				PrimitiveMaterial::push( pDisplay, PrimitiveMaterial::ADDITIVE );			
				// display a quad in the render window
				PrimitiveWindow::push( pDisplay, context.window(), GLARE_UV, glareColor );
			}
		}
	}
}

//----------------------------------------------------------------------------
// EOF
