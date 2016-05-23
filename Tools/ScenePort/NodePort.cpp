/*
	NodePort.cpp
	(c)2005 Palestar, Richard Lyle
*/

#pragma warning( disable: 4786 )	// identifier was truncated to '255' characters in the browser information

#include "StdAfx.h"
#include "ChildFrame.h"
#include "NodePort.h"
#include "NodeView.h" 

//----------------------------------------------------------------------------

IMPLEMENT_FACTORY( NodePort, BaseNodePort );
REGISTER_FACTORY_KEY(  NodePort, 3929515797207620102 );

BEGIN_PROPERTY_LIST( NodePort, BaseNodePort )
	ADD_PROPERTY( m_Position )
	ADD_PROPERTY( m_Frame )
	ADD_PROPERTY( m_fFrameScale )
END_PROPERTY_LIST()

//----------------------------------------------------------------------------

NodePort::NodePort() : BaseNodePort()
{
	m_Type = m_Class = CLASS_KEY(NodeTransform);
	m_Position.zero();
	m_Frame.identity();
	m_fFrameScale = 1.0f;
}

//-------------------------------------------------------------------------------

const int VERSION_020100 = 020100;
const int VERSION_031203 = 31203;

bool NodePort::read( const InStream & input )
{
	if (! BaseNodePort::read( input ) )
	{
		int version;
		input >> version;

		switch( version )
		{
		case VERSION_031203:
			input >> m_Position;
			input >> m_Frame;
			input >> m_fFrameScale;
			break;
		case VERSION_020100:
			m_Class = CLASS_KEY(NodeTransform);
			input >> m_Position;
			input >> m_Frame;
			input >> m_fFrameScale;
			break;
		default:
			m_Class = CLASS_KEY(NodeTransform);
			// pre-version
			input.file()->setPosition( input.file()->position() - sizeof(int) );
			input >> m_Position;
			input >> m_Frame;
			m_fFrameScale = 1.0f;
			break;
		}
		return false;
	}

	return true;
}

//----------------------------------------------------------------------------

void NodePort::initializeNode( BaseNode * pNode )
{
	BaseNodePort::initializeNode( pNode );

	NodeTransform * pCastedNode = dynamic_cast<NodeTransform *>( pNode );
	if ( pCastedNode != NULL )
	{
		pCastedNode->setPosition( m_Position );
		pCastedNode->setFrame( m_Frame * m_fFrameScale );
	}
}

bool NodePort::canScale()
{
	return true;
}

void NodePort::scale( int x, int y )
{
	m_fFrameScale += (1.0f / 200.0f) * y;
	NodeTransform * pNode = dynamic_cast<NodeTransform *>( getCachedNode() );
	if ( pNode != NULL )
		pNode->setFrame( m_Frame * m_fFrameScale );
}

bool NodePort::canMove()
{
	return true;
}

void NodePort::move( Vector3 & move )
{
	m_Position += move;

	NodeTransform * pNode = dynamic_cast<NodeTransform *>( getCachedNode() );
	if ( pNode != NULL )
		pNode->setPosition( m_Position );
}

bool NodePort::canPitch()
{
	return true;
}

void NodePort::pitch( float r )
{
	m_Frame.rotate( r, 0, 0 );

	NodeTransform * pNode = dynamic_cast<NodeTransform *>( getCachedNode() );
	if ( pNode != NULL )
		pNode->setFrame( m_Frame * m_fFrameScale );
}

bool NodePort::canYaw()
{
	return true;
}

void NodePort::yaw( float r )
{
	m_Frame.rotate( 0, r, 0 );

	NodeTransform * pNode = dynamic_cast<NodeTransform *>( getCachedNode() );
	if ( pNode != NULL )
		pNode->setFrame( m_Frame * m_fFrameScale );
}

bool NodePort::canRoll()
{
	return true;
}

void NodePort::roll( float r )
{
	m_Frame.rotate( 0, 0, r );

	NodeTransform * pNode = dynamic_cast<NodeTransform *>( getCachedNode() );
	if ( pNode != NULL )
		pNode->setFrame( m_Frame * m_fFrameScale );
}

void NodePort::resetPosition( Vector3 & pos )
{
	m_Position = pos;

	NodeTransform * pNode = dynamic_cast<NodeTransform *>( getCachedNode() );
	if ( pNode != NULL )
		pNode->setPosition( m_Position );

	updateAllViews();
}

void NodePort::resetFrame()
{
	m_Frame.identity();
	m_fFrameScale = 1.0f;

	NodeTransform * pNode = dynamic_cast<NodeTransform *>( getCachedNode() );
	if ( pNode != NULL )
		pNode->setFrame( m_Frame * m_fFrameScale );
	updateAllViews();
}

//----------------------------------------------------------------------------
// EOF
