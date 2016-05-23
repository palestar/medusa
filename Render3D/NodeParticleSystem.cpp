/*
	NodeParticleSystem.cpp
	(c)2005 Palestar
*/

#define RENDER3D_DLL

#include "Debug/Assert.h"
#include "Render3D/NodeParticleSystem.h"
#include "Render3D/Material.h"
#include "Display/PrimitiveTriangleList.h"
#include "Display/PrimitiveSetTransform.h"
#include "Math/Helpers.h"

//-------------------------------------------------------------------------------

float NodeParticleSystem::sm_fDetail = 1.0f;
float NodeParticleSystem::sm_fGravity = 9.8f;

//---------------------------------------------------------------------------------------------------

#define ENABLE_HULL_CALCULATION			0
 
//! particle triangle as defined in screen space..
static const Vector3 PARTICLE_POINTS[3] =
{
	Vector3( 0.0f, 1.0f, 0.0f ),
	Vector3( 0.8660254f, -0.5f, 0.0f ),
	Vector3( -0.8660254f, -0.5f, 0.0f )
};
static const Vector3 PARTICLE_NORMAL( 0.0f, 0.0f, -1.0f );

//---------------------------------------------------------------------------------------------------

IMPLEMENT_NAMED_FACTORY( FACTORY_ParticleData, NodeParticleSystem::ParticleInstanceData, RenderContext::InstanceData, true );

//----------------------------------------------------------------------------

IMPLEMENT_FACTORY( NodeParticleSystem, NodeTransform );
REGISTER_FACTORY_KEY(  NodeParticleSystem, 3976326220507417898LL );

BEGIN_PROPERTY_LIST( NodeParticleSystem, NodeTransform )
	ADD_PROPERTY(m_Material);
    ADD_PROPERTY(m_SystemDuration);
    ADD_PROPERTY(m_bContinuous);
	ADD_PROPERTY(m_bLocalSpace);
    ADD_PROPERTY(m_ParticleRadius);
    ADD_PROPERTY(m_percentRadiusRandomness);
    ADD_PROPERTY(m_maxNumParticles);                   
    ADD_PROPERTY(m_MeanInitialSpeed);
    ADD_PROPERTY(m_InitialPositionMaxRadius);
    ADD_PROPERTY(m_percentInitialPositionRandomness);
    ADD_PROPERTY(m_percentInitialSpeedRandomness);
    ADD_PROPERTY(m_percentInitialDirectionRandomness);
    ADD_PROPERTY(m_ParticleEmissionRate);
    ADD_PROPERTY(m_percentEmissionRateRandomness);
    ADD_PROPERTY(m_ParticleLifeRandomness);
    ADD_PROPERTY(m_ParticleLifetime);
    ADD_PROPERTY(m_FadeInDuration);
    ADD_PROPERTY(m_FadeOutDuration);
    ADD_PROPERTY(m_ParticleMass);                 
    ADD_PROPERTY(m_BouyantForceMagnitude);
    ADD_PROPERTY(m_WindAcceleration);
    ADD_PROPERTY(m_bAlphaFadeIn);
    ADD_PROPERTY(m_bAlphaFadeOut);
    ADD_PROPERTY(m_bScaleIn);
    ADD_PROPERTY(m_bScaleOut);
    ADD_PROPERTY(m_bRadialVelocity);
    ADD_PROPERTY(m_InitVelocity);
    ADD_PROPERTY(m_bAnimateColor);
    ADD_PROPERTY(m_ColorGradStart);
    ADD_PROPERTY(m_ColorGradEnd);
	ADD_PROPERTY(m_bCreateOnSurface);
	ADD_PROPERTY(m_BoxDiagonal);
	ADD_PROPERTY(m_bCreateInBox);
	ADD_PROPERTY(m_bLiveInBox);
END_PROPERTY_LIST();

//-------------------------------------------------------------------------------

NodeParticleSystem::NodeParticleSystem() :
	m_Hull( Vector3(-1.0f,-1.0f,-1.0f), Vector3(1.0f,1.0f,1.0f) ),
    m_SystemDuration( 60.0f ),
    m_ParticleLifeRandomness( 0.0f ),
    m_bContinuous( false ),
	m_bLocalSpace( false ),
    m_ParticleRadius( 0.2f ),
    m_percentRadiusRandomness( 0.35f ),
	m_maxNumParticles( 256 ),
    m_MeanInitialSpeed( 0.0f ),
    m_InitialPositionMaxRadius( 30.0f ),
    m_percentInitialPositionRandomness( 1.0f, 1.0f, 1.0f ),
    m_percentInitialSpeedRandomness(  0.5f ),
    m_percentInitialDirectionRandomness( 1.0f ),
    m_ParticleEmissionRate( 57.0f ),
    m_percentEmissionRateRandomness( 0.9f ),
    m_ParticleLifetime( 3.5f ),
	m_FadeInDuration( 1.5f ),
	m_FadeOutDuration( 1.5f ),
    m_percentRandomBrownianMotion( 0.0f ),
    m_ParticleMass( 0.0f ),
    m_BouyantForceMagnitude( 4.3f ),
	m_WindAcceleration( Vector3::ZERO ),
    m_bAlphaFadeIn( false ),
    m_bAlphaFadeOut( true ),
    m_bScaleIn( false ),
    m_bScaleOut( false ),
    m_bAnimateColor( false ),
    m_ColorGradStart( WHITE ),
    m_ColorGradEnd( WHITE ),
    m_bRadialVelocity(false),
    m_bDetectCollision( false ),
    m_bElasticCollision( false ),
    m_bEndOnCollision( false ),
	m_bCreateOnSurface( false ),
	m_BoxDiagonal( Vector3::ZERO ),
	m_InitVelocity( Vector3( 0.0f, 0.0f, -1.0f ) ),
	m_bCreateInBox( false ),
	m_bLiveInBox( false )
{
	m_nNodeFlags |= NF_NOBOUNDS;
}

NodeParticleSystem::~NodeParticleSystem()
{}

//-------------------------------------------------------------------------------

bool NodeParticleSystem::read( const InStream & input )
{
	if (! NodeTransform::read( input ) )
		return false;

	return true;
}

//---------------------------------------------------------------------------------------------------

BoxHull NodeParticleSystem::hull() const
{
	return m_Hull;
}

void NodeParticleSystem::render( RenderContext & context,
                                 const Matrix33 & frame,
                                 const Vector3 & position )
{
	if ( context.isShadowPass() )
		return;	// no particles on shadow passes
	if (! m_Material.valid( true, false ) )
		return;
	ParticleInstanceData * pInstance = updateParticles( context, frame, position );
	if (! pInstance )
		return;
	Vector3 positionVS( context.worldToView( position ) );
	if (! context.sphereVisible( positionVS, m_Hull.radius() ) )
		return;
	DisplayDevice * pDevice = context.device();
	if (! pDevice )
		return;
	PrimitiveTriangleListDL::Ref pTriangleList = PrimitiveTriangleListDL::create( pDevice, pInstance->m_nVisibleParticles );
	if (! pTriangleList )
		return;
	VertexL * pVertex = (VertexL *)pTriangleList->lock();
	if (! pVertex )
		return;

	const Vector3 n( context.frame() % PARTICLE_NORMAL );
	const Vector3 p0( context.frame() % PARTICLE_POINTS[0] );
	const Vector3 p1( context.frame() % PARTICLE_POINTS[1] );
	const Vector3 p2( context.frame() % PARTICLE_POINTS[2] );

	float fFrameScale = frame.scale();
	int nParticlesRendered = 0;
    for( unsigned int i=0; i<m_maxNumParticles; ++i )
    {   
		Particle & particle = pInstance->m_pParticles[i];
		if ( particle.m_bAlive && nParticlesRendered < pInstance->m_nVisibleParticles )
		{
			Color color( particle.m_Color );
			color.a = 255 * particle.m_fSolidity;

			// convert from world to view space..
			Vector3 vParticlePosition( m_bLocalSpace ? (position + (frame % particle.m_vPosition)) : particle.m_vPosition );

			pVertex->position = vParticlePosition + (p0 * particle.m_fScale * fFrameScale);
			pVertex->normal = n;
			pVertex->diffuse = color;
			pVertex->u = 0.5f;
			pVertex->v = 0.0f;
			++pVertex;

			pVertex->position = vParticlePosition + (p1 * particle.m_fScale * fFrameScale);
			pVertex->normal = n;
			pVertex->diffuse = color;
			pVertex->u = 1.0f;
			pVertex->v = 1.0f;
			++pVertex;

			pVertex->position = vParticlePosition + (p2 * particle.m_fScale * fFrameScale);
			pVertex->normal = n;
			pVertex->diffuse = color;
			pVertex->u = 0.0f;
			pVertex->v = 1.0f;
			++pVertex;
			
			++nParticlesRendered;
		}
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

NodeParticleSystem::ParticleInstanceData * NodeParticleSystem::updateParticles( RenderContext & context,
								const Matrix33 & frame,
                                const Vector3 & position  )
{
	if (! m_bContinuous && context.time() > (m_SystemDuration + m_ParticleLifetime) )
		return NULL;		// system is done
	ParticleInstanceData * pInstance = context.instanceData<ParticleInstanceData>();
	if (! pInstance )
		return NULL;

	if (! pInstance->m_pParticles )
	{
		pInstance->m_fTime = 0.0f; //context.time();
		pInstance->m_pParticles = new Particle[m_maxNumParticles];
		if (! pInstance->m_pParticles )
		{
			TRACE( "Failed to allocate memory!" );
			return NULL;
		}
	}

	float fElapsed = context.time() - pInstance->m_fTime;
	pInstance->m_fTime = context.time();

	// calculate the number of particles to try to emit this frame..
	int nEmitCount = 0;
	if ( m_bContinuous || context.time() < m_SystemDuration )
	{
		float fEmit = RandomFloat( 0.0f, m_ParticleEmissionRate ) * m_percentEmissionRateRandomness;
		fEmit += m_ParticleEmissionRate * (1.0f - m_percentEmissionRateRandomness);
		fEmit *= sm_fDetail;

		pInstance->m_ParticleEmissionAccumulator += fEmit * fElapsed;
		nEmitCount = (int)pInstance->m_ParticleEmissionAccumulator;
		pInstance->m_ParticleEmissionAccumulator -= nEmitCount;
	}

	// calculate acceleration
	Vector3 vAcceleration( Vector3::ZERO );
    if( m_ParticleMass > 0.0f )
		vAcceleration.y -= sm_fGravity;
    if( m_BouyantForceMagnitude > 0.0f )
	    vAcceleration.y += m_BouyantForceMagnitude;
    if( m_WindAcceleration.magnitude() > 0.0f )
	    vAcceleration += m_WindAcceleration;

	if (! m_bLocalSpace )
		vAcceleration = frame % vAcceleration;

#if ENABLE_HULL_CALCULATION
	m_Hull.resetBox();
#endif
	// track the total number of visible particles, this is needed to allocate dynamic vertex buffer space..
	pInstance->m_nVisibleParticles = 0;
	// update, age, and emit particles in one pass of the particle array..
	for(unsigned int i=0;i<m_maxNumParticles;++i)
	{
		Particle & particle = pInstance->m_pParticles[i];
		if ( particle.m_bAlive )
		{
			particle.m_fAge += fElapsed;
			if ( particle.m_fAge < particle.m_fEndAge )
			{
				if ( m_bAlphaFadeIn && m_FadeInDuration > 0.0f && particle.m_fAge < m_FadeInDuration )
					particle.m_fSolidity = Clamp<float>( particle.m_fAge / m_FadeInDuration, 0.0f, 1.0f );
				else if ( m_bAlphaFadeOut && m_FadeOutDuration > 0.0f && particle.m_fAge > (particle.m_fEndAge - m_FadeOutDuration) )
					particle.m_fSolidity = Clamp<float>( (particle.m_fEndAge - particle.m_fAge) / m_FadeOutDuration, 0.0f, 1.0f );

				if ( m_bScaleIn && m_FadeInDuration > 0.0f && particle.m_fAge < m_FadeInDuration )
					particle.m_fScale = particle.m_fBaseScale * Clamp<float>( particle.m_fAge / m_FadeInDuration, 0.0f, 1.0f );
				else if ( m_bScaleOut && m_FadeOutDuration > 0.0f && particle.m_fAge > (particle.m_fEndAge - m_FadeOutDuration ) )
					particle.m_fScale = particle.m_fBaseScale * Clamp<float>( (particle.m_fEndAge - particle.m_fAge) / m_FadeOutDuration, 0.0f, 1.0f );

				if ( m_bAnimateColor )
					particle.m_Color = Color( m_ColorGradStart ).iterpolate( m_ColorGradEnd, particle.m_fAge / particle.m_fEndAge );
			
				particle.m_vVelocity += vAcceleration * fElapsed;
				particle.m_vPosition += particle.m_vVelocity * fElapsed;		

#if ENABLE_HULL_CALCULATION
				m_Hull.addVertex( particle.m_vPosition - position );
#endif
				++pInstance->m_nVisibleParticles;
			}
			else
			{
				// particle has expired, free the slot..
				particle.m_bAlive = false;
			}
		}
		else if ( nEmitCount > 0 )
		{
			--nEmitCount;

			// emit new particle using this slot..
			particle.m_bAlive = true;
			particle.m_fSolidity = m_bAlphaFadeIn ? 0.0f : 1.0f;

			particle.m_fAge = 0.0f;
			particle.m_fEndAge = RandomFloat( 0.0f, m_ParticleLifetime ) * m_ParticleLifeRandomness;
			particle.m_fEndAge += m_ParticleLifetime * (1.0f - m_ParticleLifeRandomness);

			particle.m_fBaseScale = RandomFloat( 0.0f, m_ParticleRadius ) * m_percentRadiusRandomness;
			particle.m_fBaseScale += (1.0f - m_percentRadiusRandomness) * m_ParticleRadius;
			particle.m_fScale = m_bScaleIn ? 0.0f : particle.m_fBaseScale;

			particle.m_vPosition = RandomVector( Vector3(-1.0f,-1.0f,-1.0f), Vector3(1.0f,1.0f,1.0f) );
			particle.m_vPosition.normalize();
			if (! m_bCreateOnSurface )
				particle.m_vPosition *= RandomFloat( 0.0f, m_InitialPositionMaxRadius );
			particle.m_vPosition *= m_InitialPositionMaxRadius;
			particle.m_vPosition *= m_percentInitialPositionRandomness;
#if ENABLE_HULL_CALCULATION
			m_Hull.addVertex( particle.m_vPosition );
#endif
			// convert position into world space
			if (! m_bLocalSpace )
				particle.m_vPosition = (frame % particle.m_vPosition) + position;	

			if (! m_bRadialVelocity )
			{
				particle.m_vVelocity = RandomVector( Vector3(-1.0f,-1.0f,-1.0f), Vector3(1.0f,1.0f,1.0f) );
				particle.m_vVelocity.normalize();
				particle.m_vVelocity *= m_percentInitialDirectionRandomness;
				particle.m_vVelocity += m_InitVelocity * (Vector3(1.0f,1.0f,1.0f) - m_percentInitialDirectionRandomness);
			}
			else
			{
				particle.m_vVelocity = particle.m_vPosition;
			}

			// convert velocity into world space
			if (! m_bLocalSpace )
				particle.m_vVelocity = frame % particle.m_vVelocity;		

			float fSpeed = (RandomFloat( 0.0f, m_MeanInitialSpeed ) + RandomFloat( 0.0f, m_MeanInitialSpeed )) / 2.0f;
			fSpeed *= m_percentInitialSpeedRandomness;
			fSpeed += (1.0f - m_percentInitialSpeedRandomness) * m_MeanInitialSpeed;
			// NOTE: The original code did all the above calculations for a particle speed, then tossed those calulations
			// out by then just using the m_MeanInitialSpeed as the multiplier.
			//fSpeed = m_MeanInitialSpeed;

			particle.m_vVelocity.normalize();
			particle.m_vVelocity *= fSpeed;

			++pInstance->m_nVisibleParticles;
		}
	}

	return pInstance;
}

//---------------------------------------------------------------------------------------------------
// EOF

