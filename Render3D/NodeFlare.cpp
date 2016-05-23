/*
	NodeFlare.cpp

	This object renders a len's flare from it's location
	(c)2005 Palestar, Richard Lyle
*/

#define RENDER3D_DLL
#include "Debug/Assert.h"
#include "Display/PrimitiveSetTransform.h"
#include "Display/PrimitiveTriangleFan.h"
#include "Render3D/NodeFlare.h"

//----------------------------------------------------------------------------

bool NodeFlare::sm_bFlareEnabled = true;

IMPLEMENT_FACTORY( NodeFlare, NodeTransform );
REGISTER_FACTORY_KEY(  NodeFlare, 4015435704486993582LL );

BEGIN_PROPERTY_LIST( NodeFlare, NodeTransform )
	ADD_PROPERTY( m_Material )
	ADD_PROPERTY( m_Scale )
END_PROPERTY_LIST()

//----------------------------------------------------------------------------

NodeFlare::NodeFlare() : m_Scale( 1.0f )
{}

//----------------------------------------------------------------------------

const int	FLARES = 10;
const float FLARE_FAR = 3.0f;
const float FLARE_NEAR = 2.0f;

const float FLARE_TIME[ FLARES ] =
{
	0.8f, 0.6f,	0.4f, 0.2f, 0.0f, 0.0f, 0.2f, 0.4f, 0.6f, 0.8f
};
const float FLARE_LOCATION[ FLARES ] =
{
	2.3f,	2.0f,	1.25f,	1.1f,	1.05f,	0.95f, 0.88f,	0.5f,	0.25f,	-0.2f
};
const float FLARE_SCALE[ FLARES ] =
{
	0.1f,	0.2f,	0.05f,	0.04f,	0.025f,	0.04f, 0.08f,	0.1f,	0.4f,	0.2f
};

//----------------------------------------------------------------------------

void NodeFlare::render( RenderContext &context, 
					const Matrix33 & frame, 
					const Vector3 & position )
{
	if ( context.isShadowPass() )
		return;	
	if (! sm_bFlareEnabled )
		return;

	Vector3 positionVS( context.worldToView( position ) );
	if ( positionVS.z >= FLARE_FAR && m_Material.valid( true, false ) )
	{
		DisplayDevice * pDevice = context.device();
		ASSERT( pDevice );

		float lightScale = FLARE_FAR / position.z;
		Vector3 light( positionVS.x * lightScale, 
			positionVS.y * lightScale, FLARE_FAR );

		Vector3 center( 0,0,FLARE_NEAR );
		Vector3 ray( light - center );

		// create the transform
		context.pushIdentity();

		const Vector3 dx(1,0,0);
		const Vector3 dy(0,-1,0);

		// save the actual context time, so we can restore it later..
		float fContextTime = context.time();

		// draw the flares
		for(int f=FLARES-1;f>=0;f--)
		{
			Vector3 flareCenter( center + (ray * FLARE_LOCATION[f]) );

			float scale = FLARE_SCALE[f] * m_Scale * (flareCenter.z / 2.0f);
			Vector3 sx( dx * scale );
			Vector3 sy( dy * scale );

			const Vector3 flareNormal( 0,0,-1);
			
			Vertex flareFan[4] =
			{
				Vertex( flareCenter + sx + sy, flareNormal, 0, 0 ),
				Vertex( flareCenter - sx + sy, flareNormal, 1, 0 ),
				Vertex( flareCenter - sx - sy, flareNormal, 1, 1 ),
				Vertex( flareCenter + sx - sy, flareNormal, 0, 1 ),
			};

			// set the context time, so the correct frame of the material will be rendered
			context.setTime( FLARE_TIME[f] );
			// flush the material primitive only, no need to flush the surfaces as well
			m_Material->flushMaterial();

			// push the material, then the triangle fan		
			Material::push( context, m_Material );
			PrimitiveTriangleFanD::push( pDevice, 4, flareFan );
		}

		// restore the context time
		context.setTime( fContextTime );
	}
}

//----------------------------------------------------------------------------

Material * NodeFlare::material() const
{
	return( m_Material );
}

float NodeFlare::scale() const
{
	return( m_Scale );
}

void NodeFlare::setFlare( Material * pMaterial, float scale )
{
	m_Material = pMaterial;
	m_Scale = scale;
}

//----------------------------------------------------------------------------
// EOF
