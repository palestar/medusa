/*
	Port.cpp
	(c)2005 Palestar, Richard Lyle
*/

#define NODEFLAREPORT_DLL
#include "stdafx.h"
#include "Port.h"

#include "Render3d/NodeFlare.h"

//----------------------------------------------------------------------------

IMPLEMENT_FACTORY( NodeFlarePort, NodePort );
REGISTER_FACTORY_KEY(  NodeFlarePort, 4015449771305557674 );

BEGIN_PROPERTY_LIST( NodeFlarePort, NodePort );
	ADD_PORT_PROPERTY( m_MaterialPort, MaterialPort );
	ADD_PROPERTY( m_Scale );
END_PROPERTY_LIST();

NodeFlarePort::NodeFlarePort() : NodePort()
{
	m_Class = m_Type = CLASS_KEY(NodeFlare);
	m_Scale = 1.0f;
}

//-------------------------------------------------------------------------------

const int VERSION_081799 = 81799;

bool NodeFlarePort::read( const InStream & input )
{
	if (! NodePort::read( input ) )
	{
		int version;
		input >> version;

		switch( version )
		{
		case VERSION_081799:
			input >> m_MaterialPort;
			input >> m_Scale;
			break;
		}
	}
	return true;
}

//-------------------------------------------------------------------------------

void NodeFlarePort::dependencies( DependentArray & dep )
{
	dep.push( m_MaterialPort );
}

CFrameWnd * NodeFlarePort::createView()
{
	return NodePort::createView();
}

BaseNode * NodeFlarePort::createNode()
{
	return NodePort::createNode();
}

void NodeFlarePort::initializeNode( BaseNode * pNode )
{
	NodePort::initializeNode( pNode );

	NodeFlare * pFlare = WidgetCast<NodeFlare>( pNode );
	if ( pFlare != NULL )
	{
		// initialize the node
		pFlare->setFlare( WidgetCast<Material>( Port::portResource( m_MaterialPort ) ), m_Scale );
	}
}


//-------------------------------------------------------------------------------
// EOF

