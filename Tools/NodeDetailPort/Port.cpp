/*
	Port.cpp
	(c)2005 Palestar, Richard Lyle
*/

#define NODEDETAILPORT_DLL
#include "stdafx.h"
#include "Port.h"

#include "Render3d/NodeDetail.h"

//----------------------------------------------------------------------------

IMPLEMENT_FACTORY( NodeDetailPort, BaseNodePort );
REGISTER_FACTORY_KEY(  NodeDetailPort, 4038744921858760898 );

BEGIN_PROPERTY_LIST( NodeDetailPort, BaseNodePort );
	ADD_PROPERTY( m_Distance );
END_PROPERTY_LIST();

NodeDetailPort::NodeDetailPort() : BaseNodePort()
{
	m_Type = m_Class = CLASS_KEY(NodeDetail);
	m_Distance = 500.0f;
}

//-------------------------------------------------------------------------------

const int VERSION_101999 = 101999;

bool NodeDetailPort::read( const InStream & input )
{
	if (! BaseNodePort::read( input ) )
	{
		int version;
		input >> version;

		switch( version )
		{
		case VERSION_101999:
			input >> m_Distance;
			break;
		default:
			return false;
		}
	}
	return true;
}

//-------------------------------------------------------------------------------

void NodeDetailPort::dependencies( DependentArray & dep )
{
	// TODO: push all ports this node is dependent on, for example:
	// dep.push("L:\\Projects\\Seeker\\Image1.Prt");
}

CFrameWnd * NodeDetailPort::createView()
{
	return BaseNodePort::createView();
}

BaseNode * NodeDetailPort::createNode()
{
	return BaseNodePort::createNode();
}

void NodeDetailPort::initializeNode( BaseNode * pNode )
{
	BaseNodePort::initializeNode( pNode );

	NodeDetail * pDetail = dynamic_cast<NodeDetail *>( pNode );
	if ( pDetail != NULL )
	{
		// initialize the node
		pDetail->setDistance( m_Distance );
	}
}


//-------------------------------------------------------------------------------
// EOF

