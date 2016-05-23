/*
	Port.cpp
	(c)2005 Palestar, Richard Lyle
*/

#define NODEPARTICLESYSTEM2PORT_DLL
#include "stdafx.h"
#include "Port.h"

#include "Math/Helpers.h"
#include "Tools/ScenePort/ChildFrame.h"
#include "Tools/ResourcerDoc/Port.h"

//----------------------------------------------------------------------------

IMPLEMENT_FACTORY( NodeParticleSystem2Port, NodePort );
REGISTER_FACTORY_KEY(  NodeParticleSystem2Port, 4017674422569680758 );

BEGIN_PROPERTY_LIST( NodeParticleSystem2Port, NodePort );
	ADD_PROPERTY( m_Seed );
	ADD_PROPERTY( m_ParticleCount );
	ADD_ENUM_PROPERTY( m_OriginType );
		ADD_ENUM_OPTION( m_OriginType, BOX );
		ADD_ENUM_OPTION( m_OriginType, SPHERE );
		ADD_ENUM_OPTION( m_OriginType, LINE );
		ADD_ENUM_OPTION( m_OriginType, CIRCLE );
	ADD_PROPERTY( m_MinOrigin );
	ADD_PROPERTY( m_MaxOrigin );
	ADD_PROPERTY( m_MinVelocity );
	ADD_PROPERTY( m_MaxVelocity );
	ADD_PROPERTY( m_MinAcceleration );
	ADD_PROPERTY( m_MaxAcceleration );
	ADD_PROPERTY( m_MinLife );
	ADD_PROPERTY( m_MaxLife );
	ADD_PROPERTY( m_ReverseTime );
	ADD_PROPERTY( m_bLocalSpace );
	ADD_PROPERTY( m_Life );
	ADD_PORT_PROPERTY( m_MaterialPort, MaterialPort );
	ADD_PROPERTY( m_Visible );
	ADD_PROPERTY( m_VisibleV );
	ADD_PROPERTY( m_VisibleA );
	ADD_PROPERTY( m_Scale );
	ADD_PROPERTY( m_ScaleV );
	ADD_PROPERTY( m_ScaleA );
	ADD_PROPERTY( m_ScaleVar );
	ADD_PROPERTY( m_Alpha );
	ADD_PROPERTY( m_AlphaV );
	ADD_PROPERTY( m_AlphaA );
END_PROPERTY_LIST();

NodeParticleSystem2Port::NodeParticleSystem2Port() : NodePort()
{
	m_Class = m_Type = CLASS_KEY(NodeParticleSystem2);

	m_Seed = rand();
	m_ParticleCount = 100;
	m_OriginType = BOX;
	m_Life = 10.0f;
	m_Visible = 1.0f;
	m_VisibleV = m_VisibleA = 0.0f;
	m_Scale = 1.0f;
	m_ScaleV = m_ScaleA = 0.0f;
	m_ScaleVar = 0.0f;
	m_Alpha = 1.0f;
	m_AlphaV = m_AlphaA = 0.0f;
	m_bLocalSpace = false;

	m_MinOrigin = m_MinVelocity = m_MinAcceleration = Vector3(-1,-1,-1);
	m_MaxOrigin = m_MaxVelocity = m_MaxAcceleration = Vector3(1,1,1);
	m_MinLife = m_MaxLife = 1.0f;
	m_ReverseTime = false;
}

//-------------------------------------------------------------------------------

const int VERSION_082399 = 82399;
const int VERSION_020200 = 020200;
const int VERSION_102201 = 102201;

bool NodeParticleSystem2Port::read( const InStream & input )
{
	if (! NodePort::read( input ) )
	{
		int version;
		input >> version;

		switch( version )
		{
		case VERSION_102201:
			input >> m_Seed;
			input >> m_ParticleCount;
			input >> m_OriginType;
			input >> m_MinOrigin;
			input >> m_MaxOrigin;
			input >> m_MinVelocity;
			input >> m_MaxVelocity;
			input >> m_MinAcceleration;
			input >> m_MaxAcceleration;
			input >> m_MinLife;
			input >> m_MaxLife;
			input >> m_ReverseTime;
			input >> m_Life;
			input >> m_MaterialPort;
			input >> m_Visible;
			input >> m_VisibleV;
			input >> m_VisibleA;
			input >> m_Scale;
			input >> m_ScaleV;
			input >> m_ScaleA;
			input >> m_ScaleV;
			input >> m_ScaleVar;
			input >> m_Alpha;
			input >> m_AlphaV;
			input >> m_AlphaA;
			break;
		case VERSION_020200:
			input >> m_ParticleCount;
			input >> m_OriginType;
			input >> m_MinOrigin;
			input >> m_MaxOrigin;
			input >> m_MinVelocity;
			input >> m_MaxVelocity;
			input >> m_MinAcceleration;
			input >> m_MaxAcceleration;
			input >> m_MinLife;
			input >> m_MaxLife;
			input >> m_ReverseTime;
			input >> m_Life;
			input >> m_MaterialPort;
			input >> m_Visible;
			input >> m_VisibleV;
			input >> m_VisibleA;
			input >> m_Scale;
			input >> m_ScaleV;
			input >> m_ScaleA;
			input >> m_ScaleV;
			input >> m_ScaleVar;
			input >> m_Alpha;
			input >> m_AlphaV;
			input >> m_AlphaA;
			m_Seed = rand();
			break;
		case VERSION_082399:
			input >> m_ParticleCount;
			input >> m_MinOrigin;
			input >> m_MaxOrigin;
			input >> m_MinVelocity;
			input >> m_MaxVelocity;
			input >> m_MinAcceleration;
			input >> m_MaxAcceleration;
			input >> m_MinLife;
			input >> m_MaxLife;
			input >> m_Life;
			input >> m_MaterialPort;
			input >> m_Visible;
			input >> m_VisibleV;
			input >> m_VisibleA;
			input >> m_Scale;
			input >> m_ScaleV;
			input >> m_ScaleA;
			input >> m_ScaleV;
			input >> m_Alpha;
			input >> m_AlphaV;
			input >> m_AlphaA;
			m_Seed = rand();
			m_OriginType = BOX;
			m_ScaleVar = 0.0f;
			m_ReverseTime = false;
			break;
		}
	}
	return true;
}

//-------------------------------------------------------------------------------

void NodeParticleSystem2Port::dependencies( DependentArray & dep )
{
	dep.push( m_MaterialPort );
}

CFrameWnd * NodeParticleSystem2Port::createView()
{
	return NodePort::createView();
}

BaseNode * NodeParticleSystem2Port::createNode()
{
	return NodePort::createNode();
}

void NodeParticleSystem2Port::initializeNode( BaseNode * pNode )
{
	NodePort::initializeNode( pNode );

	NodeParticleSystem2 * pSystem = dynamic_cast<NodeParticleSystem2 *>( pNode );
	if ( pSystem != NULL )
	{
		srand( m_Seed );

		// create all the particles
		for(int i=0;i<m_ParticleCount;i++)
		{
			Particle newParticle;

			switch( m_OriginType )
			{
			case BOX:
				newParticle.origin = RandomVector( m_MinOrigin, m_MaxOrigin );
				break;
			case SPHERE:
				{
					float minRadius = m_MinOrigin.magnitude();
					float maxRadius = m_MaxOrigin.magnitude();
					float radius = RandomFloat( minRadius, maxRadius );

					Vector3 direction( RandomFloat( -1.0f, 1.0f ), RandomFloat( -1.0f, 1.0f ), RandomFloat( -1.0f, 1.0f) );
					direction.normalize();

					newParticle.origin = direction * radius;
				}
				break;
			case LINE:
				{
					Vector3 delta( m_MinOrigin - m_MaxOrigin );
					float distance = delta.magnitude();
					float d = RandomFloat( 0.0f, distance );

					newParticle.origin = m_MaxOrigin + (delta * d);
				}
				break;
			case CIRCLE:
				{
					float minRadius = m_MinOrigin.magnitude();
					float maxRadius = m_MaxOrigin.magnitude();
					float radius = RandomFloat( minRadius, maxRadius );

					Vector3 direction( RandomFloat( -1.0f, 1.0f ), 0, RandomFloat( -1.0f, 1.0f) );
					direction.normalize();

					newParticle.origin = direction * radius;
				}
				break;
			}

			newParticle.velocity = RandomVector( m_MinVelocity, m_MaxVelocity );
			newParticle.acceleration = RandomVector( m_MinAcceleration, m_MaxAcceleration );
			newParticle.life = RandomFloat( m_MinLife, m_MaxLife );
			newParticle.scale = RandomFloat( -m_ScaleVar, m_ScaleVar );

			pSystem->addParticle( newParticle );
		}

		pSystem->setLife( m_Life );
		pSystem->setMaterial( WidgetCast<Material>( Port::portResource( m_MaterialPort ) ) );
		pSystem->setVisible( m_Visible, m_VisibleV, m_VisibleA );
		pSystem->setScale( m_Scale, m_ScaleV, m_ScaleA );
		pSystem->setAlpha( m_Alpha, m_AlphaV, m_AlphaA );
		pSystem->setReverseTime( m_ReverseTime );
		pSystem->setLocalSpace( m_bLocalSpace );
	}
}


//-------------------------------------------------------------------------------
// EOF

