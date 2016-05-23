/*
	Port.cpp
	(c)2005 Palestar, Richard Lyle
*/

#define NODEDECALPORT_DLL
#include "stdafx.h"

#include "Port.h"
#include "PortView.h"
#include "Render3d/NodeDecal.h"

#include "Tools/ResourcerDoc/Port.h"
#include "Tools/ScenePort/ChildFrame.h"

//----------------------------------------------------------------------------

IMPLEMENT_FACTORY( NodeDecalPort, NodePort );
REGISTER_FACTORY_KEY(  NodeDecalPort, 4420272799238427825 );

BEGIN_PROPERTY_LIST( NodeDecalPort, NodePort );
	ADD_PORT_PROPERTY( m_Material, MaterialPort );
END_PROPERTY_LIST();

NodeDecalPort::NodeDecalPort() : NodePort()
{
	m_Class = m_Type = CLASS_KEY(NodeDecal );
}

//-------------------------------------------------------------------------------

const int VERSION_031600 = 31600;	// TODO: set the current date as the version

bool NodeDecalPort::read( const InStream & input )
{
	if (! NodePort::read( input ) )
	{
		int version;
		input >> version;

		switch( version )
		{
		case VERSION_031600:
			input >> m_Material;
			break;
		default:
			return false;
		}
	}
	return true;
}

//-------------------------------------------------------------------------------

void NodeDecalPort::dependencies( DependentArray & dep )
{
	dep.push( m_Material );
}

CFrameWnd * NodeDecalPort::createView()
{
	return NodePort::createView();
}

BaseNode * NodeDecalPort::createNode()
{
	return NodePort::createNode();
}

void NodeDecalPort::initializeNode( BaseNode * pNode )
{
	NodePort::initializeNode( pNode );

	NodeDecal * pDecal = WidgetCast<NodeDecal>( pNode );
	if ( pDecal != NULL )
		pDecal->setMaterial( WidgetCast<Material>( Port::portResource( m_Material ) ) );
}


//-------------------------------------------------------------------------------
// EOF

