/*
	Port.cpp
	(c)2005 Palestar, Richard Lyle
*/

#define NODEPARTICLEFIELD_DLL
#include "stdafx.h"
#include "Port.h"

#include "Render3d/NodeParticleField.h"
#include "Tools/ResourcerDoc/Port.h"
#include "Tools/ScenePort/ChildFrame.h"

//----------------------------------------------------------------------------

IMPLEMENT_FACTORY( NodeParticleFieldPort, BaseNodePort );
REGISTER_FACTORY_KEY(  NodeParticleFieldPort, 4087921939007759360 );

BEGIN_PROPERTY_LIST( NodeParticleFieldPort, BaseNodePort );
	ADD_PORT_PROPERTY( m_MaterialPort, MaterialPort );
	ADD_PROPERTY( m_ParticleCount );
	ADD_PROPERTY( m_Scale );
	ADD_PROPERTY( m_Front );
	ADD_PROPERTY( m_Back );
	ADD_PROPERTY( m_Velocity );
END_PROPERTY_LIST();

NodeParticleFieldPort::NodeParticleFieldPort() : BaseNodePort()
{
	m_Class = m_Type = CLASS_KEY(NodeParticleField);

	m_ParticleCount = 100;
	m_Scale = 1.0f;
	m_Front = 1.0f;
	m_Back = 100.0f;
	m_Velocity = Vector3( true );
}

//-------------------------------------------------------------------------------

const int VERSION_022900 = 22900;

bool NodeParticleFieldPort::read( const InStream & input )
{
	if (! BaseNodePort::read( input ) )
	{
		int version;
		input >> version;

		switch( version )
		{
		case VERSION_022900:
			input >> m_MaterialPort;
			input >> m_ParticleCount;
			input >> m_Scale;
			input >> m_Front;
			input >> m_Back;
			input >> m_Velocity;
			break;
		}
	}
	return true;
}

//-------------------------------------------------------------------------------

void NodeParticleFieldPort::dependencies( DependentArray & dep )
{
	dep.push( m_MaterialPort );
}

CFrameWnd * NodeParticleFieldPort::createView()
{
	return BaseNodePort::createView();
}

BaseNode * NodeParticleFieldPort::createNode()
{
	return BaseNodePort::createNode();
}

void NodeParticleFieldPort::initializeNode( BaseNode * pNode )
{
	BaseNodePort::initializeNode( pNode );

	NodeParticleField * pField = WidgetCast<NodeParticleField>( pNode );
	if ( pField != NULL )
	{
		pField->initialize(  WidgetCast<Material>( Port::portResource( m_MaterialPort ) ),
			m_ParticleCount, m_Scale, m_Front, m_Back, m_Velocity );
	}
}


//-------------------------------------------------------------------------------
// EOF

