/*
	Port.cpp
	(c)2005 Palestar, Richard Lyle
*/

#define NODELIGHTPORT_DLL
#include "stdafx.h"
#include "Port.h"

#include "Tools/ScenePort/ChildFrame.h"

//----------------------------------------------------------------------------

IMPLEMENT_FACTORY( NodeLightPort, NodePort );
REGISTER_FACTORY_KEY(  NodeLightPort, 3963943979066761755 );

BEGIN_PROPERTY_LIST( NodeLightPort, NodePort );
	ADD_ENUM_PROPERTY( m_Type );
		ADD_ENUM_OPTION( m_Type, NodeLight::DIRECTIONAL );
		ADD_ENUM_OPTION( m_Type, NodeLight::POINT );
		ADD_ENUM_OPTION( m_Type, NodeLight::PARALLEL_POINT );
		ADD_ENUM_OPTION( m_Type, NodeLight::SPOT );
	ADD_COLOR_PROPERTY( m_Color );
	ADD_PROPERTY( m_Distance );
	ADD_ENUM_PROPERTY( m_Effect );
		ADD_ENUM_OPTION( m_Effect, NodeLight::NONE );
		ADD_ENUM_OPTION( m_Effect, NodeLight::GLOW );
		ADD_ENUM_OPTION( m_Effect, NodeLight::FLICKER );
		ADD_ENUM_OPTION( m_Effect, NodeLight::STROBE );
	ADD_PROPERTY( m_EffectInterval );
END_PROPERTY_LIST();

NodeLightPort::NodeLightPort() : NodePort()
{
	m_Class = BaseNodePort::m_Type = CLASS_KEY(NodeLight);

	m_Type = NodeLight::DIRECTIONAL;
	m_Color = Color(128,128,128).RGB();
	m_Distance = 100.0f;
	m_Effect = NodeLight::NONE;
	m_EffectInterval = 1.0f;
}

//-------------------------------------------------------------------------------

const int VERSION_050499 = 20;
const int VERSION_091200 = 91200;

bool NodeLightPort::read( const InStream & input ) 
{
	if (! NodePort::read( input ) )
	{
		int version;
		input >> version;

		switch( version )
		{
		case VERSION_091200:
			input >> m_Type;
			input >> m_Color;
			input >> m_Distance;
			input >> m_Effect;
			input >> m_EffectInterval;
			break;
		case VERSION_050499:
			input >> m_Type;
			input >> m_Color;
			input >> m_Distance;
			m_Effect = NodeLight::NONE;
			m_EffectInterval = 1.0f;
			break;
		default:
			m_Type = (Type)version;	// I didn't version the first revision of this object, thus cludge <rol>
			input >> m_Color;
			m_Distance = 100.0f;
			break;
		}
	}
	return true;
}

//-------------------------------------------------------------------------------

CFrameWnd * NodeLightPort::createView()
{
	return NodePort::createView();
}

BaseNode * NodeLightPort::createNode()
{
	return NodePort::createNode();
}

void NodeLightPort::initializeNode( BaseNode * pNode )
{
	NodePort::initializeNode( pNode );

	NodeLight * pLight = dynamic_cast<NodeLight *>( pNode );
	if ( pLight != NULL )
	{
		pLight->setType( m_Type );
		pLight->setColor( m_Color );
		pLight->setDistance( m_Distance );
		pLight->setEffect( m_Effect, m_EffectInterval );
	}
}


//-------------------------------------------------------------------------------
// EOF

