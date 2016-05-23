/*
	Port.cpp
	(c)2005 Palestar, Richard Lyle
*/

#define NODEPARTICLESYSTEMPORT_DLL
#include "stdafx.h"

#include "Port.h"
#include "PortView.h"

#include "Render3d/NodeParticleSystem.h"
#include "Tools/ScenePort/ChildFrame.h"
#include "Tools/ResourcerDoc/Port.h"

const int VERSION_051199 = 1;
const int VERSION_072799 = 2;
const int VERSION_080399 = 3;
const int VERSION_081099 = 4;
const int VERSION_083199 = 5;
const int VERSION_122199 = 122199;

//----------------------------------------------------------------------------

IMPLEMENT_FACTORY( NodeParticleSystemPort, NodePort );
REGISTER_FACTORY_KEY(  NodeParticleSystemPort, 3978642704838358366 );

BEGIN_PROPERTY_LIST( NodeParticleSystemPort, NodePort );
	ADD_PORT_PROPERTY( m_MaterialPort, MaterialPort );
    ADD_PROPERTY( m_SystemDuration );
    ADD_PROPERTY( m_bContinuous );
	ADD_PROPERTY( m_bLocalSpace );
    ADD_PROPERTY( m_ParticleRadius );
    ADD_PROPERTY( m_percentRadiusRandomness );
    ADD_PROPERTY( m_maxNumParticles );                   
    ADD_PROPERTY( m_MeanInitialSpeed );
    ADD_PROPERTY( m_InitialPositionMaxRadius );
    ADD_PROPERTY( m_percentInitialPositionRandomness );
    ADD_PROPERTY( m_percentInitialSpeedRandomness );
    ADD_PROPERTY( m_ParticleEmissionRate );
    ADD_PROPERTY( m_percentEmissionRateRandomness );
    ADD_PROPERTY( m_ParticleLifeRandomness );
    ADD_PROPERTY( m_ParticleLifetime );
    ADD_PROPERTY( m_FadeInDuration );
    ADD_PROPERTY( m_FadeOutDuration );
    ADD_PROPERTY( m_ParticleMass );                 
    ADD_PROPERTY( m_BouyantForceMagnitude );
    ADD_PROPERTY( m_bAlphaFadeIn );
    ADD_PROPERTY( m_bAlphaFadeOut );
    ADD_PROPERTY( m_bScaleIn );
    ADD_PROPERTY( m_bScaleOut );
    ADD_PROPERTY( m_bRadialVelocity );
    ADD_PROPERTY( m_percentInitialDirectionRandomness );
    ADD_PROPERTY( m_InitVelocity );
    ADD_PROPERTY( m_bAnimateColor );
    ADD_COLOR_PROPERTY( m_ColorGradStart );
    ADD_COLOR_PROPERTY( m_ColorGradEnd );
	ADD_PROPERTY( m_bCreateOnSurface );
	ADD_PROPERTY( m_BoxDiagonal );
	ADD_PROPERTY( m_bCreateInBox );
	ADD_PROPERTY( m_bLiveInBox );
END_PROPERTY_LIST();

NodeParticleSystemPort::NodeParticleSystemPort() : NodePort()
{
	NodePort::m_Type = m_Class = CLASS_KEY(NodeParticleSystem);
    m_SystemDuration = 60.0f;
    m_bContinuous = false;
	m_bLocalSpace = false;
    m_ParticleRadius = 0.4f;
    m_percentRadiusRandomness = 0.35f;
	m_maxNumParticles = 512;
    m_MeanInitialSpeed = 1.0f;
    m_InitialPositionMaxRadius = 1.0f;
    m_percentInitialPositionRandomness.x = 1.0f;
    m_percentInitialPositionRandomness.y = 1.0f;
    m_percentInitialPositionRandomness.z = 1.0f;
    m_percentInitialSpeedRandomness = 0.5f;
    m_percentInitialDirectionRandomness = 1.0f;
    m_ParticleEmissionRate = 32.0f;
    m_percentEmissionRateRandomness = 0.9f;
    m_ParticleLifeRandomness = 0.0f;
    m_ParticleLifetime = 3.0f;
	m_FadeInDuration = 0.5f;
	m_FadeOutDuration = 0.5f;
    m_ParticleMass = 0.0f;
    m_BouyantForceMagnitude = 0.0f;
    m_WindAcceleration.zero();
    m_bAlphaFadeIn = false;
    m_bAlphaFadeOut = true;
    m_bScaleIn = false;
    m_bScaleOut = false;
    m_bRadialVelocity = false;
    Color init_color( 255.0f, 255.0f, 255.0, 255.0f );
    m_bAnimateColor = false;
    m_ColorGradStart = init_color.RGB();
    m_ColorGradEnd = init_color.RGB();
    m_InitVelocity.x = 0.0f;
    m_InitVelocity.y = 0.0f;
    m_InitVelocity.z = 0.0f;
	m_bCreateOnSurface = false;
	m_BoxDiagonal = Vector3( 0.0f,0.0f,0.0f );
	m_bCreateInBox = false;
	m_bLiveInBox = false;
}

//-------------------------------------------------------------------------------

bool NodeParticleSystemPort::read( const InStream & input )
{
	if (! NodePort::read( input ) )
		return false;

	return true;
}

//-------------------------------------------------------------------------------

void NodeParticleSystemPort::dependencies( DependentArray & dep )
{
	dep.push( m_MaterialPort );
}

CFrameWnd * NodeParticleSystemPort::createView()
{
	return NodePort::createView();
}

BaseNode * NodeParticleSystemPort::createNode()
{
	return NodePort::createNode();
}

void NodeParticleSystemPort::initializeNode( BaseNode * pNode )
{
	NodePort::initializeNode( pNode );

	NodeParticleSystem * pParticles = dynamic_cast<NodeParticleSystem *>( pNode );
	if ( pParticles != NULL )
	{
		pParticles->setMaterial( WidgetCast<Material>( Port::portResource( m_MaterialPort ) ) );
		pParticles->setDuration( m_SystemDuration );
		pParticles->setContinuous( m_bContinuous );
		pParticles->setLocalSpace( m_bLocalSpace );
		pParticles->setRadius( m_ParticleRadius );
		pParticles->setRadiusRand( m_percentRadiusRandomness );
		pParticles->setMaxNum( m_maxNumParticles );                   
		pParticles->setSpeed( m_MeanInitialSpeed );
		pParticles->setPosRadius( m_InitialPositionMaxRadius );
		pParticles->setPosRand( m_percentInitialPositionRandomness );
		pParticles->setSpeedRand( m_percentInitialSpeedRandomness );
		pParticles->setRate( m_ParticleEmissionRate );
		pParticles->setRateRand( m_percentEmissionRateRandomness );
		pParticles->setLifeRand( m_ParticleLifeRandomness );
		pParticles->setLife( m_ParticleLifetime );
		pParticles->setFadeInTime( m_FadeInDuration );
		pParticles->setFadeOutTime( m_FadeOutDuration );
		pParticles->setMass( m_ParticleMass );                 
		pParticles->setBouyant( m_BouyantForceMagnitude );
		pParticles->setFadeIn( m_bAlphaFadeIn );
		pParticles->setFadeOut( m_bAlphaFadeOut );
		pParticles->setScaleIn( m_bScaleIn );
		pParticles->setScaleOut( m_bScaleOut );
		pParticles->setRadialVelocity( m_bRadialVelocity );
		pParticles->setDirRand( m_percentInitialDirectionRandomness );
		pParticles->setInitVelocity( m_InitVelocity );
		pParticles->setAnimateColor( m_bAnimateColor );
		pParticles->setColorStart( m_ColorGradStart );
		pParticles->setColorEnd( m_ColorGradEnd );
		pParticles->setCreateOnSurface( m_bCreateOnSurface );
		pParticles->setBoxValues( m_BoxDiagonal,
								  m_bCreateInBox,
								  m_bLiveInBox);
	}
}


//-------------------------------------------------------------------------------
// EOF

