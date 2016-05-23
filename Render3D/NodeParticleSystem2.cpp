/*
	NodeParticleSystem2.cpp
	(c)2005 Palestar, Richard Lyle
*/

#define RENDER3D_DLL
#include "Debug/Assert.h"
#include "Display/PrimitiveTriangleList.h"
#include "Display/PrimitiveSetTransform.h"
#include "Render3D/NodeParticleSystem.h"
#include "Render3D/NodeParticleSystem2.h"
#include "Standard/Limits.h"

//---------------------------------------------------------------------------------------------------

IMPLEMENT_NAMED_FACTORY( FACTORY_ParticleInstanceData, NodeParticleSystem2::ParticleInstanceData, RenderContext::InstanceData, true );

//-------------------------------------------------------------------------------

IMPLEMENT_FACTORY( NodeParticleSystem2, NodeTransform );
REGISTER_FACTORY_KEY(  NodeParticleSystem2, 4017665146841403990LL );

BEGIN_PROPERTY_LIST( NodeParticleSystem2, NodeTransform )
	ADD_PROPERTY(m_Particles);
	ADD_PROPERTY(m_Life);
	ADD_PROPERTY(m_ReverseTime);
	ADD_PROPERTY(m_Material);
	ADD_PROPERTY(m_Visible);
	ADD_PROPERTY(m_VisibleV);
	ADD_PROPERTY(m_VisibleA);
	ADD_PROPERTY(m_Scale);
	ADD_PROPERTY(m_ScaleV);
	ADD_PROPERTY(m_ScaleA);
	ADD_PROPERTY(m_Alpha);
	ADD_PROPERTY(m_AlphaV);
	ADD_PROPERTY(m_AlphaA);
	ADD_PROPERTY(m_bLocalSpace);
END_PROPERTY_LIST();

//----------------------------------------------------------------------------

NodeParticleSystem2::NodeParticleSystem2() :
	m_Life( 1.0f ),
	m_ReverseTime( false ),
	m_Visible( 1.0f ),
	m_VisibleA( 0.0f ),
	m_VisibleV( 0.0f ),
	m_Scale( 1.0f ),
	m_ScaleV( 0.0f ),
	m_ScaleA( 0.0f ),
	m_Alpha( 0.5f ),
	m_AlphaV( 0.0f ),
	m_AlphaA( 0.0f ),
	m_bLocalSpace( false ),
	m_Hull( Vector3(-1,-1,-1), Vector3(1,1,1) )
{
	m_nNodeFlags |= NF_NOBOUNDS;
}

//---------------------------------------------------------------------------------------------------

bool NodeParticleSystem2::read( const InStream & input ) 
{
	if (! NodeTransform::read( input ) )
		return false;
	return true;
}

//-------------------------------------------------------------------------------

BoxHull NodeParticleSystem2::hull() const
{
	return m_Hull;
}

void NodeParticleSystem2::render( RenderContext &context, 
									const Matrix33 & frame, 
									const Vector3 & position )
{
	if ( context.isShadowPass() )
		return;	// no particles on shadow passes
	if ( context.time() > m_Life )				
		return;
	if (! m_Material.valid( true, false ) )
		return;

	// we create the instance data before we check visiblity, I don't want to lose all data
	// just because the root of this item goes out of view.
	ParticleInstanceData * pParticleData = context.instanceData<ParticleInstanceData>();
	if ( pParticleData->m_Particles.size() != m_Particles.size() )
		pParticleData->m_Particles.allocate( m_Particles.size() );

	Vector3 positionVS( context.worldToView( position ) );
	if (! context.sphereVisible( positionVS, m_Hull.radius() ) )
		return;

	float t = context.time();
	float t2 = t * t;

	float visible = Clamp( m_Visible + ( m_VisibleV * t ) + ( m_VisibleA * t2 ), 0.0f, 1.0f );
	int visibleParticles = (int)( m_Particles.size() * visible * NodeParticleSystem::sm_fDetail );
	if ( visibleParticles <= 0 )
		return;

	// get the display device
	DisplayDevice * pDevice = context.device();
	ASSERT( pDevice );

	// create triangle buffer
	PrimitiveTriangleListDL::Ref pTriangleList = PrimitiveTriangleListDL::create( pDevice, visibleParticles );
	if (! pTriangleList )
		return;

	const Vector3 N(0,0,0);

	float fFrameScale = frame.scale();

	m_Hull.resetBox();

	const Matrix33 mParticleFrame( context.viewToWorld( Matrix33::IDENTITY ) );

	// update the triangle list
	VertexL * pVertex = (VertexL *)pTriangleList->lock();
	for(int i=0;i<visibleParticles;i++)
	{
		Particle & particle = m_Particles[ i ];
		ParticleInstance & instance = pParticleData->m_Particles[ i ];

		float pt = t - (int(t / particle.life) * particle.life);	// particle time
		if ( pt >= instance.m_fLastTime )
		{
			// update velocity and origin..
			float fElapsed = pt - instance.m_fLastTime;
			instance.m_vOrigin += instance.m_vVelocity * fElapsed;
			instance.m_vVelocity += instance.m_vAcceleration * fElapsed;
		}
		else
		{
			// particle has reset, update new starting position..
			instance.m_vOrigin = m_bLocalSpace ? particle.origin : position + (frame % particle.origin);		
			instance.m_vVelocity = m_bLocalSpace ? particle.velocity : frame % particle.velocity;
			instance.m_vAcceleration = m_bLocalSpace ? particle.acceleration : frame % particle.acceleration;

			if ( m_ReverseTime )
			{
				instance.m_vVelocity += instance.m_vAcceleration * (particle.life * particle.life);
				instance.m_vOrigin += instance.m_vVelocity * particle.life;
				instance.m_vVelocity = -instance.m_vVelocity;
				instance.m_vAcceleration = -instance.m_vAcceleration;
			}
		}
		instance.m_fLastTime = pt;

		float pt2 = pt * pt;
		float scale = (particle.scale + (m_Scale + (m_ScaleV * pt) + (m_ScaleA * pt2)));
		float alpha = Clamp( m_Alpha + (m_AlphaV * pt) + (m_AlphaA * pt2), 0.0f, 1.0f );
		
		const Vector3 X( mParticleFrame.i * (scale * fFrameScale) );
		const Vector3 Y( mParticleFrame.j * (scale * fFrameScale) );
		Color color( 255,255,255,(u8)(255 * alpha) );

		m_Hull.addVertex( m_bLocalSpace ? instance.m_vOrigin : instance.m_vOrigin - position );

		Vector3 vParticlePosition( m_bLocalSpace ? (position + (frame % instance.m_vOrigin)) : instance.m_vOrigin );
		pVertex->position = vParticlePosition + Y;
		pVertex->normal = N;
		pVertex->diffuse = color;
		pVertex->u = 0.5f;
		pVertex->v = 0.0f;
		pVertex++;

		pVertex->position = vParticlePosition + X - Y;
		pVertex->normal = N;
		pVertex->diffuse = color;
		pVertex->u = 1.0f;
		pVertex->v = 1.0f;
		pVertex++;

		pVertex->position = vParticlePosition - X - Y;
		pVertex->normal = N;
		pVertex->diffuse = color;
		pVertex->u = 0.0f;
		pVertex->v = 1.0f;
		pVertex++;
	}
	pTriangleList->unlock();

	// push the primitives
	context.push( m_Material );
	context.pushIdentity();
	context.push( pTriangleList );

	// call the base class
	NodeTransform::render( context, frame, position );
}

//-------------------------------------------------------------------------------

int NodeParticleSystem2::particleCount() const
{
	return( m_Particles.size() );
}

const NodeParticleSystem2::Particle & NodeParticleSystem2::particle( int n ) const
{
	return( m_Particles[ n ] );
}

Material * NodeParticleSystem2::material() const
{
	return( m_Material );
}

//-------------------------------------------------------------------------------

int NodeParticleSystem2::addParticle( const Particle & p )
{
	m_Particles.push( p );	
	return( m_Particles.size() - 1 );
}

void NodeParticleSystem2::removeParticle( int n )
{
	m_Particles.remove( n );
}

void NodeParticleSystem2::setLife( float life )
{
	m_Life = life;
}

void NodeParticleSystem2::setReverseTime( bool reverseTime )
{
	m_ReverseTime = reverseTime;
}

void NodeParticleSystem2::setLocalSpace( bool a_bLocalSpace )
{
	m_bLocalSpace = a_bLocalSpace;
}

void NodeParticleSystem2::setMaterial( Material * pMaterial )
{
	m_Material = pMaterial;
}

void NodeParticleSystem2::setVisible( float visible, float visibleV, float visibleA )
{
	m_Visible = visible;
	m_VisibleV = visibleV;
	m_VisibleA = visibleA;
}

void NodeParticleSystem2::setScale( float scale, float scaleV, float scaleA )
{
	m_Scale = scale;
	m_ScaleV = scaleV;
	m_ScaleA = scaleA;
}

void NodeParticleSystem2::setAlpha( float alpha, float alphaV, float alphaA )
{
	m_Alpha = alpha;
	m_AlphaV = alphaV;
	m_AlphaA = alphaA;
}

//-------------------------------------------------------------------------------
// EOF

