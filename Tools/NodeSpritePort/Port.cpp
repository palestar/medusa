/*
	Port.cpp
	(c)2005 Palestar, Richard Lyle
*/

#define NODESPRITEPORT_DLL
#include "stdafx.h"
#include "Port.h"
#include "Tools/ResourcerDoc/Port.h"

//----------------------------------------------------------------------------

IMPLEMENT_FACTORY( NodeSpritePort, NodePort );
REGISTER_FACTORY_KEY(  NodeSpritePort, 3974556940033159447 );

BEGIN_PROPERTY_LIST( NodeSpritePort, NodePort );
	ADD_PROPERTY( m_Scale );
	ADD_PROPERTY( m_ScaleLock );
	ADD_PROPERTY( m_ClampBack );
	ADD_PROPERTY( m_EnableAlpha );
	ADD_PROPERTY( m_Alpha );
	ADD_PROPERTY( m_AlphaV );
	ADD_PROPERTY( m_AlphaA );
	ADD_ENUM_PROPERTY( m_AxisLock );
		ADD_ENUM_OPTION( m_AxisLock, NodeSprite::LOCKNONE );
		ADD_ENUM_OPTION( m_AxisLock, NodeSprite::LOCKX );
		ADD_ENUM_OPTION( m_AxisLock, NodeSprite::LOCKY );
		ADD_ENUM_OPTION( m_AxisLock, NodeSprite::LOCKZ );
		ADD_ENUM_OPTION( m_AxisLock, NodeSprite::LOCKXY );
		ADD_ENUM_OPTION( m_AxisLock, NodeSprite::LOCKXZ );
		ADD_ENUM_OPTION( m_AxisLock, NodeSprite::LOCKYZ );
		ADD_ENUM_OPTION( m_AxisLock, NodeSprite::LOCKXYZ );
	ADD_PORT_PROPERTY( m_MaterialPort, MaterialPort );
END_PROPERTY_LIST();

NodeSpritePort::NodeSpritePort() : NodePort()
{
	m_Class = m_Type = CLASS_KEY(NodeSprite);
	m_Scale = 1.0f;
	m_ScaleLock = false;
	m_ClampBack = false;
	m_AxisLock = NodeSprite::LOCKNONE;
	m_EnableAlpha = false;
	m_Alpha = 1.0f;
	m_AlphaV = m_AlphaA = 0.0f;
}

//-------------------------------------------------------------------------------

const int VERSION_101999 = 101999;
const int VERSION_110299 = 110299;
const int VERSION_092001 = 92001;

bool NodeSpritePort::read( const InStream & input )
{
	if (! NodePort::read( input ) )
	{
		int version;
		input >> version;

		switch( version )
		{
		case VERSION_092001:
			input >> m_Scale;
			input >> m_ScaleLock;
			input >> m_ClampBack;
			input >> m_EnableAlpha;
			input >> m_Alpha;
			input >> m_AlphaV;
			input >> m_AlphaA;
			input >> m_AxisLock;
			input >> m_MaterialPort;
			break;
		case VERSION_110299:
			input >> m_Scale;
			input >> m_ScaleLock;
			input >> m_EnableAlpha;
			input >> m_Alpha;
			input >> m_AlphaV;
			input >> m_AlphaA;
			input >> m_AxisLock;
			input >> m_MaterialPort;
			m_ClampBack = false;
			break;
		case VERSION_101999:
			input >> m_Scale;
			input >> m_ScaleLock;
			input >> m_AxisLock;
			input >> m_MaterialPort;
			m_EnableAlpha = false;
			m_Alpha = 1.0f;
			m_AlphaV = m_AlphaA = 0.0f;
			m_ClampBack = false;
			break;
		}
	}
	return true;
}

//-------------------------------------------------------------------------------

void NodeSpritePort::dependencies( DependentArray & dep )
{
	dep.push( m_MaterialPort );
}

CFrameWnd * NodeSpritePort::createView()
{
	return NodePort::createView();
}

BaseNode * NodeSpritePort::createNode()
{
	return NodePort::createNode();
}

void NodeSpritePort::initializeNode( BaseNode * pNode )
{
	// call the base class
	NodePort::initializeNode( pNode );

	NodeSprite * pSprite = WidgetCast<NodeSprite>( pNode );
	if ( pSprite != NULL )
	{
		// get the material resource
		Material::Link material = 
			WidgetCast<Material>( Port::portResource( m_MaterialPort ) );

		pSprite->setScale( m_Scale );
		pSprite->setScaleLock( m_ScaleLock );
		pSprite->setClampBack( m_ClampBack );
		pSprite->setAxisLock( m_AxisLock );
		pSprite->setMaterial( material );
		pSprite->setAlpha( m_EnableAlpha, m_Alpha, m_AlphaV, m_AlphaA );
	}
}

bool NodeSpritePort::canScale()
{
	return true;
}

void NodeSpritePort::scale( int x, int y )
{
	m_Scale += float(x) / 20.0f;

	NodeSprite * pSprite = dynamic_cast<NodeSprite *>( getCachedNode() );
	if ( pSprite != NULL )
		pSprite->setScale( m_Scale );
}

//-------------------------------------------------------------------------------
// EOF

