/*
	Port.cpp
	(c)2005 Palestar, Richard Lyle
*/

#define NODESCENEPORT_DLL
#include "stdafx.h"

#include "Port.h"
#include "PortView.h"
#include "Render3d/NodeScene.h"
#include "Render3d/Scene.h"

#include "Tools/ScenePort/ChildFrame.h"
#include "Tools/ResourcerDoc/Port.h"

//----------------------------------------------------------------------------

IMPLEMENT_FACTORY( NodeScenePort, NodePort );
REGISTER_FACTORY_KEY(  NodeScenePort, 3930450796192381662 );

BEGIN_PROPERTY_LIST( NodeScenePort, NodePort );
	ADD_PORT_PROPERTY( m_ScenePort, ScenePort );
	ADD_PROPERTY( m_NormalizeFrame );
END_PROPERTY_LIST();

NodeScenePort::NodeScenePort() : NodePort()
{
	m_Class = m_Type = CLASS_KEY(NodeScene);
	m_NormalizeFrame = true;
}

//-------------------------------------------------------------------------------

const int VERSION_051199 = 1;
const int VERSION_082099 = 82099;

bool NodeScenePort::read( const InStream & input )
{
	if (! NodePort::read( input ) )
	{
		int version;
		input >> version;

		switch( version )
		{
		case VERSION_082099:
			input >> m_ScenePort;
			input >> m_NormalizeFrame;
			break;
		case VERSION_051199:
			input >> m_ScenePort;
			m_NormalizeFrame = true;
			break;
		default:

			// rewind and read old format
			input.file()->setPosition( input.file()->position() - sizeof(int) );

			dword bits;
			float time, fps;

			input >> m_ScenePort;
			input >> bits;
			input >> time;
			input >> fps;

			m_NormalizeFrame = true;
			break;
		}
	}

	return true;
}

//-------------------------------------------------------------------------------

void NodeScenePort::dependencies( DependentArray & dep )
{
	dep.push( m_ScenePort );
}

CFrameWnd * NodeScenePort::createView()
{
	return NodePort::createView();
}

BaseNode * NodeScenePort::createNode()
{
	return NodePort::createNode();
}

void NodeScenePort::initializeNode( BaseNode * pNode )
{
	NodePort::initializeNode( pNode );

	NodeScene * pSceneNode = WidgetCast<NodeScene>( pNode );
	if ( pSceneNode != NULL )
	{
		Scene::Link scene = WidgetCast<Scene>( Port::portResource( m_ScenePort ) );
		if ( scene.valid() )
			pSceneNode->setScene( scene, m_NormalizeFrame );
	}
}

//-------------------------------------------------------------------------------
// EOF

