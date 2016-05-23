/*
	NodeParticleField.h

	This node can be used to produce rain, snow, or star fields.. this type of particle system
	always remains the camera view

	(c)2005 Palestar, Richard Lyle
*/

#define RENDER3D_DLL
#include "Debug/Assert.h"
#include "Render3D/NodeParticleField.h"
#include "Display/PrimitiveTriangleList.h"
#include "Display/PrimitiveSetTransform.h"

//----------------------------------------------------------------------------

IMPLEMENT_FACTORY( NodeParticleField, BaseNode );
REGISTER_FACTORY_KEY(  NodeParticleField, 4039837800374425578LL );

BEGIN_PROPERTY_LIST( NodeParticleField, BaseNode )
	ADD_PROPERTY( m_Material );
	ADD_PROPERTY( m_ParticleCount );
	ADD_PROPERTY( m_Scale );
	ADD_PROPERTY( m_Front );
	ADD_PROPERTY( m_Back );
	ADD_PROPERTY( m_Velocity );
END_PROPERTY_LIST();

//----------------------------------------------------------------------------

NodeParticleField::NodeParticleField()
{
	m_ParticleCount = 50;
	m_Scale = 1.0f;
	m_Front = 1.0f;
	m_Back = 100.0f;
	m_Velocity = Vector3( 0.0f );
}

//----------------------------------------------------------------------------

void NodeParticleField::render( RenderContext &context, 
		const Matrix33 & frame, 
		const Vector3 & position )
{
	if ( m_Material.valid() )
	{
		DisplayDevice * pDisplay = context.display();
		ASSERT( pDisplay );

		if ( m_Particles.size() == 0 )
		{
			m_Particles.allocate( m_ParticleCount );
			for(int i=0;i<m_ParticleCount;i++)
				createParticle( context, i );
		}

		// tranpose the frame
		Matrix33 vsf( ~frame );

		// create triangle buffer
		PrimitiveTriangleListDL::Ref pTriangleList = 
			PrimitiveTriangleListDL::create( pDisplay, m_Particles.size() );

		const Vector3 X(m_Scale,0,0);
		const Vector3 Y(0,m_Scale,0);
		const Vector3 N(0,0,-1.0f);

		// update the triangle list
		VertexL * pVertex = (VertexL *)pTriangleList->lock();
		for(int i=0;i<m_Particles.size();i++)
		{
			// transform the position into view space
			Vector3 vsp( (vsf * m_Particles[ i ]) + position );
			if ( fabs( vsp.x ) > vsp.z || fabs( vsp.y ) > vsp.z || vsp.z > m_Back || vsp.z < m_Front )
			{
				switch( rand() % 5 )
				{
				case 0:
					vsp = Vector3( 1.0f, randomFloat( -1.0f, 1.0f ), 1.0f ) * randomFloat( m_Front, m_Back );
					break;
				case 1:	
					vsp = Vector3( -1.0f, randomFloat( -1.0f, 1.0f ), 1.0f ) * randomFloat( m_Front, m_Back );
					break;
				case 2:
					vsp = Vector3( randomFloat( -1.0f, 1.0f ), 1.0f, 1.0f ) * randomFloat( m_Front, m_Back );
					break;
				case 3:
					vsp = Vector3( randomFloat( -1.0f, 1.0f ), -1.0f, 1.0f ) * randomFloat( m_Front, m_Back );
					break;
				case 4:
					vsp = Vector3( randomFloat( -1.0f, 1.0f ), randomFloat( -1.0f, 1.0f ), 1.0f ) * m_Back;
					break;
				}

				//vsp.z = randomFloat( m_Front, m_Back );

				// convert back to object space
				m_Particles[ i ] = frame * (vsp - position);
			}

			Color diffuse(255,255,255,(u8)((1.0f - (vsp.z / m_Back)) * 255) );

			pVertex->position = vsp + Y;
			pVertex->normal = N;
			pVertex->diffuse = diffuse;
			pVertex->u = 0.5f;
			pVertex->v = 0.0f;
			pVertex++;

			pVertex->position = vsp + X - Y;
			pVertex->normal = N;
			pVertex->diffuse = diffuse;
			pVertex->u = 1.0f;
			pVertex->v = 1.0f;
			pVertex++;

			pVertex->position = vsp - X - Y;
			pVertex->normal = N;
			pVertex->diffuse = diffuse;
			pVertex->u = 0.0f;
			pVertex->v = 1.0f;
			pVertex++;
		}
		pTriangleList->unlock();

		// push the primitives
		context.push( m_Material );
		context.pushIdentity();
		context.push( pTriangleList );
	}
}

//----------------------------------------------------------------------------

void NodeParticleField::initialize( Material * pMaterial, int particles, float scale, 
								   float front, float back, const Vector3 & velocity )
{
	m_Material = pMaterial;
	m_ParticleCount = particles;
	m_Scale = scale;
	m_Front = front;
	m_Back = back;
	m_Velocity = velocity;
}

void NodeParticleField::update( float t )
{
	Vector3 v( m_Velocity * t );
	for(int i=0;i<m_Particles.size();i++)
		m_Particles[ i ] += v;
}

//----------------------------------------------------------------------------

void NodeParticleField::createParticle( RenderContext & context, int n )
{
	// create particle somewhere in the viewspace of the context
	Vector3 vsp( randomFloat( -1.0f, 1.0f ), randomFloat( -1.0f, 1.0f ), 1.0f );
	vsp *= randomFloat( m_Front, m_Back );

	// transform from viewspace to objectspace
	m_Particles[ n ] = context.frame() * (vsp + context.position());
}

//----------------------------------------------------------------------------

float NodeParticleField::randomFloat( float low, float high )
{
	if ( low > high )
	{
		float swap = low;
		low = high;
		high = low;
	}

	float d = (high - low) / float(RAND_MAX);
	return( low + (rand() * d) );
}

//----------------------------------------------------------------------------
// EOF
