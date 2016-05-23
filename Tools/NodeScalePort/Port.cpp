/*
	Port.cpp
	(c)2005 Palestar, Richard Lyle
*/

#define NODESCALEPORT_DLL
#include "stdafx.h"

#include "Port.h"
#include "PortView.h"

#include "Tools/ScenePort/ChildFrame.h"
#include "Render3d/NodeScale.h"

//----------------------------------------------------------------------------

IMPLEMENT_FACTORY( NodeScalePort, NodePort );
REGISTER_FACTORY_KEY(  NodeScalePort, 4013155070838887270 );

BEGIN_PROPERTY_LIST( NodeScalePort, NodePort );
	ADD_PROPERTY( m_XV );
	ADD_PROPERTY( m_XA );
	ADD_PROPERTY( m_YV );
	ADD_PROPERTY( m_YA );
	ADD_PROPERTY( m_ZV );
	ADD_PROPERTY( m_ZA );
END_PROPERTY_LIST();

NodeScalePort::NodeScalePort() : NodePort()
{
	m_Class = m_Type = CLASS_KEY(NodeScale);
	m_XV = m_XA = m_YV = m_YA = m_ZV = m_ZA = 0.0f;
}

//-------------------------------------------------------------------------------

const int VERSION_081199 = 1;

bool NodeScalePort::read( const InStream & input )
{
	if (! NodePort::read( input ) )
	{
		int version;
		input >> version;

		switch( version )
		{
		case VERSION_081199:
			input >> m_XV;
			input >> m_XA;
			input >> m_YV;
			input >> m_YA;
			input >> m_ZV;
			input >> m_ZA;
			break;
		}
	}

	return true;
}

//-------------------------------------------------------------------------------

CFrameWnd * NodeScalePort::createView()
{
	return NodePort::createView();
}

BaseNode * NodeScalePort::createNode()
{
	return NodePort::createNode();
}

void NodeScalePort::initializeNode( BaseNode * pNode )
{
	NodePort::initializeNode( pNode );

	NodeScale * pScale = dynamic_cast<NodeScale *>( pNode );
	if ( pScale != NULL )
		pScale->setScale( m_XV, m_XA, m_YV, m_YA, m_ZV, m_ZA );
}


//-------------------------------------------------------------------------------
// EOF

