/*
	Port.cpp
	(c)2005 Palestar, Richard Lyle
*/

#define NODEBOOLEANTIMEPORT_DLL
#include "stdafx.h"

#include "Port.h"

#include "Tools/ScenePort/ChildFrame.h"
#include "Render3d/NodeBooleanTime.h"

//----------------------------------------------------------------------------

IMPLEMENT_FACTORY( NodeBooleanTimePort, BaseNodePort );
REGISTER_FACTORY_KEY(  NodeBooleanTimePort, 4013214877461020674 );

BEGIN_PROPERTY_LIST( NodeBooleanTimePort, BaseNodePort );
	ADD_PROPERTY( m_Begin );
	ADD_PROPERTY( m_End );
	ADD_PROPERTY( m_Invert );
	ADD_PROPERTY( m_WarpTime );
	ADD_PROPERTY( m_Loop );
	ADD_PROPERTY( m_LoopTime );
END_PROPERTY_LIST();

NodeBooleanTimePort::NodeBooleanTimePort() : BaseNodePort()
{
	m_Type = m_Class = CLASS_KEY( NodeBooleanTime );

	m_Begin = 0.0f;
	m_End = 1.0f;
	m_Invert = false;
	m_WarpTime = false;
	m_Loop = false;
	m_LoopTime = 0.0f;
}

//-------------------------------------------------------------------------------

const int VERSION_081199 = 1;
const int VERSION_061600 = 61600;

bool NodeBooleanTimePort::read( const InStream & input )
{
	if (! BaseNodePort::read( input ) )
	{
		int version;
		input >> version;

		switch( version )
		{
		case VERSION_061600:
			input >> m_Begin;
			input >> m_End;
			input >> m_Invert;
			input >> m_WarpTime;
			input >> m_Loop;
			input >> m_LoopTime;
			return true;
		case VERSION_081199:
			input >> m_Begin;
			input >> m_End;
			input >> m_Invert;
			input >> m_WarpTime;
			m_Loop = false;
			m_LoopTime = 0.0f;
			return true;
		}
		return false;
	}
	return true;
}

//-------------------------------------------------------------------------------

CFrameWnd * NodeBooleanTimePort::createView()
{
	return BaseNodePort::createView();
}

BaseNode * NodeBooleanTimePort::createNode()
{
	return BaseNodePort::createNode();
}

void NodeBooleanTimePort::initializeNode( BaseNode * pNode )
{
	BaseNodePort::initializeNode( pNode );

	NodeBooleanTime * pBooleanTime = dynamic_cast<NodeBooleanTime *>( pNode );
	if ( pBooleanTime != NULL )
	{
		pBooleanTime->setTime( m_Begin, m_End, m_Invert, m_WarpTime );
		pBooleanTime->setLoop( m_Loop, m_LoopTime );
	}
}


//-------------------------------------------------------------------------------
// EOF

