/*
	Port.cpp
	(c)2005 Palestar, Richard Lyle
*/

#define NODEBLURPORT_DLL
#include "stdafx.h"
#include "Port.h"
#include "Tools/ScenePort/ChildFrame.h"

//----------------------------------------------------------------------------

IMPLEMENT_FACTORY( NodeBlurPort, BaseNodePort );
REGISTER_FACTORY_KEY(  NodeBlurPort, 4013927722214315786 );

BEGIN_PROPERTY_LIST( NodeBlurPort, BaseNodePort );
	ADD_PROPERTY( m_Time );
	ADD_PROPERTY( m_Steps );
	ADD_ENUM_PROPERTY( m_Blending );
		ADD_ENUM_OPTION( m_Blending, PrimitiveMaterial::NONE );
		ADD_ENUM_OPTION( m_Blending, PrimitiveMaterial::ALPHA );
		ADD_ENUM_OPTION( m_Blending, PrimitiveMaterial::ALPHA_INV );
		ADD_ENUM_OPTION( m_Blending, PrimitiveMaterial::ADDITIVE );
		ADD_ENUM_OPTION( m_Blending, PrimitiveMaterial::ADDITIVE_INV );
END_PROPERTY_LIST();

NodeBlurPort::NodeBlurPort() : BaseNodePort()
{
	m_Class = m_Type = CLASS_KEY( NodeBlur );
	m_Time = 1.0f;
	m_Steps = 4;
	m_Blending = PrimitiveMaterial::ADDITIVE;
}

//-------------------------------------------------------------------------------

const int VERSION_081399 = 1;

bool NodeBlurPort::read( const InStream & input )
{
	if (! BaseNodePort::read( input ) )
	{
		int version;
		input >> version;

		switch( version )
		{
		case VERSION_081399:
			input >> m_Time;
			input >> m_Steps;
			input >> m_Blending;
			return true;
		}
		return false;
	}

	return true;
}

//-------------------------------------------------------------------------------

CFrameWnd * NodeBlurPort::createView()
{
	return BaseNodePort::createView();
}

BaseNode * NodeBlurPort::createNode()
{
	return BaseNodePort::createNode();
}

void NodeBlurPort::initializeNode( BaseNode * pNode )
{
	BaseNodePort::initializeNode( pNode );

	NodeBlur * pBlur = dynamic_cast<NodeBlur *>( pNode );
	if ( pBlur != NULL )
		pBlur->setBlur( m_Time, m_Steps, m_Blending );
}

//-------------------------------------------------------------------------------
// EOF

