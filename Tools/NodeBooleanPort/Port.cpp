/*
	Port.cpp
	(c)2005 Palestar, Richard Lyle
*/

#include "stdafx.h"

#include "Port.h"
#include "Render3d/NodeBoolean.h"
#include "Tools/ScenePort/ChildFrame.h"

//----------------------------------------------------------------------------

IMPLEMENT_FACTORY( NodeBooleanPort, BaseNodePort );
REGISTER_FACTORY_KEY(  NodeBooleanPort, 3956834164876440006 );

BEGIN_PROPERTY_LIST( NodeBooleanPort, BaseNodePort );
	ADD_PROPERTY( m_Bits );
	ADD_ENUM_PROPERTY( m_Type );
		ADD_ENUM_OPTION( m_Type, NodeBoolean::AND );
		ADD_ENUM_OPTION( m_Type, NodeBoolean::AND_ALL );
		ADD_ENUM_OPTION( m_Type, NodeBoolean::NOT );
		ADD_ENUM_OPTION( m_Type, NodeBoolean::EQUAL );
		ADD_ENUM_OPTION( m_Type, NodeBoolean::NOT_EQUAL );
	ADD_PROPERTY( m_MinDistance );
	ADD_PROPERTY( m_MaxDistance );
	ADD_PROPERTY( m_fFadePercent );
	ADD_ENUM_PROPERTY( m_MinDetailLevel );
		ADD_ENUM_OPTION( m_MinDetailLevel, NodeBoolean::LOW );
		ADD_ENUM_OPTION( m_MinDetailLevel, NodeBoolean::MEDIUM );
		ADD_ENUM_OPTION( m_MinDetailLevel, NodeBoolean::HIGH );
	ADD_ENUM_PROPERTY( m_MaxDetailLevel );
		ADD_ENUM_OPTION( m_MaxDetailLevel, NodeBoolean::LOW );
		ADD_ENUM_OPTION( m_MaxDetailLevel, NodeBoolean::MEDIUM );
		ADD_ENUM_OPTION( m_MaxDetailLevel, NodeBoolean::HIGH );
	ADD_PROPERTY( m_bZeroTime );
END_PROPERTY_LIST();

NodeBooleanPort::NodeBooleanPort() : BaseNodePort()
{
	m_Class = BaseNodePort::m_Type = CLASS_KEY( NodeBoolean );

	m_Type = NodeBoolean::AND_ALL;
	m_Bits = 0x0;
	m_MinDistance = 0.0f;
	m_MaxDistance = 32768.0f;
	m_fFadePercent = 0.1f;
	m_MinDetailLevel = NodeBoolean::LOW;
	m_MaxDetailLevel = NodeBoolean::HIGH;
	m_bZeroTime = false;
}

//-------------------------------------------------------------------------------

const int VERSION_081399 = 7777;
const int VERSION_091200 = 91200;

bool NodeBooleanPort::read( const InStream & input )
{
	if (! BaseNodePort::read( input ) )
	{
		int version;
		input >> version;

		switch( version )
		{
		case VERSION_091200:
			input >> m_Bits;
			input >> m_Type;
			input >> m_MinDistance;
			input >> m_MaxDistance;
			input >> m_MinDetailLevel;
			input >> m_MaxDetailLevel;
			return true;
		case VERSION_081399:
			input >> m_Bits;
			input >> m_Type;
			m_MinDistance = 0.0f;
			m_MaxDistance = 32768.0f;
			m_MinDetailLevel = NodeBoolean::LOW;
			m_MaxDetailLevel = NodeBoolean::HIGH;
			return true;
		default:
			m_Bits = version;
			m_Type = NodeBoolean::AND;
			m_MinDistance = 0.0f;
			m_MaxDistance = 32768.0f;
			m_MinDetailLevel = NodeBoolean::LOW;
			m_MaxDetailLevel = NodeBoolean::HIGH;
			return true;
		}
		return false;
	}

	return true;
}

//-------------------------------------------------------------------------------

CFrameWnd * NodeBooleanPort::createView()
{
	return BaseNodePort::createView();
}

BaseNode * NodeBooleanPort::createNode()
{
	return BaseNodePort::createNode();
}

void NodeBooleanPort::initializeNode( BaseNode * pNode )
{
	BaseNodePort::initializeNode( pNode );

	NodeBoolean * pBooleanNode = dynamic_cast< NodeBoolean *>( pNode );
	if ( pBooleanNode != NULL )
	{
		pBooleanNode->setBits( m_Bits );
		pBooleanNode->setType( m_Type );
		pBooleanNode->setDistance( m_MinDistance, m_MaxDistance );
		pBooleanNode->setFadePercent( m_fFadePercent );
		pBooleanNode->setDetailLevel( m_MinDetailLevel, m_MaxDetailLevel );
		pBooleanNode->setZeroTime( m_bZeroTime );
	}
}


//-------------------------------------------------------------------------------
// EOF

