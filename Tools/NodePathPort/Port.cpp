/*
	Port.cpp
	(c)2005 Palestar, Richard Lyle
*/

#define NODEPATHPORT_DLL
#include "stdafx.h"

#include "Port.h"
#include "PortView.h"

#include "Tools/ScenePort/ChildFrame.h"

//----------------------------------------------------------------------------

IMPLEMENT_FACTORY( NodePathPort, NodePort );
REGISTER_FACTORY_KEY(  NodePathPort, 4015898904329473734 );

BEGIN_PROPERTY_LIST( NodePathPort, NodePort );
	ADD_PROPERTY( m_Velocity );
	ADD_PROPERTY( m_Acceleration );
END_PROPERTY_LIST();

NodePathPort::NodePathPort() : NodePort()
{
	m_Class = m_Type = CLASS_KEY(NodePath);
	m_Velocity = Vector3( true );
	m_Acceleration = Vector3( true );
}

//-------------------------------------------------------------------------------

const int VERSION_081899 = 1;

bool NodePathPort::read( const InStream & input )
{
	if (! NodePort::read( input ) )
	{
		int version;
		input >> version;

		switch( version )
		{
		case VERSION_081899:
			input >> m_Velocity;
			input >> m_Acceleration;
			break;
		}
	}

	return true;
}

//-------------------------------------------------------------------------------

CFrameWnd * NodePathPort::createView()
{
	return NodePort::createView();
}

BaseNode * NodePathPort::createNode()
{
	return NodePort::createNode();
}

void NodePathPort::initializeNode( BaseNode * pNode )
{
	NodePort::initializeNode( pNode );

	NodePath * pPath = dynamic_cast<NodePath *>( pNode );
	if ( pPath != NULL )
		pPath->setPath( m_Velocity, m_Acceleration );
}


//-------------------------------------------------------------------------------
// EOF

