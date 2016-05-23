/*
	Port.cpp
	(c)2005 Palestar, Richard Lyle
*/

#define NODESPINPORT_DLL
#include "stdafx.h"
#include "Port.h"

#include "Tools/ScenePort/ChildFrame.h"

//----------------------------------------------------------------------------

IMPLEMENT_FACTORY( NodeSpinPort, NodePort );
REGISTER_FACTORY_KEY(  NodeSpinPort, 3937499180624796536 );

BEGIN_PROPERTY_LIST( NodeSpinPort, NodePort );
	ADD_PROPERTY( m_Pitch );
	ADD_PROPERTY( m_Yaw );
	ADD_PROPERTY( m_Roll );
END_PROPERTY_LIST();

NodeSpinPort::NodeSpinPort() : NodePort()
{
	m_Class = m_Type = CLASS_KEY(NodeSpin);
	m_Yaw = m_Pitch = m_Roll = 0.0f;
}

//-------------------------------------------------------------------------------

bool NodeSpinPort::read( const InStream & input )
{
	if (! NodePort::read( input ) )
	{
		input >> m_Pitch;
		input >> m_Yaw;
		input >> m_Roll;
	}
	return true;
}

//-------------------------------------------------------------------------------

CFrameWnd * NodeSpinPort::createView()
{
	return NodePort::createView();
}

BaseNode * NodeSpinPort::createNode()
{
	return NodePort::createNode();
}

void NodeSpinPort::initializeNode( BaseNode * pNode )
{
	NodePort::initializeNode( pNode );

	NodeSpin * pNodeSpin = dynamic_cast<NodeSpin *>( pNode );
	if ( pNodeSpin != NULL )
		pNodeSpin->setSpin( m_Pitch, m_Yaw, m_Roll );
}


//-------------------------------------------------------------------------------
// EOF

