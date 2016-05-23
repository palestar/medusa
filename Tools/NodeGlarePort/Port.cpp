/*
	Port.cpp
	(c)2005 Palestar, Richard Lyle
*/

#define NODEGLAREPORT_DLL
#include "stdafx.h"
#include "Port.h"

#include "Render3d/NodeGlare.h"



//----------------------------------------------------------------------------

IMPLEMENT_FACTORY( NodeGlarePort, BaseNodePort );
REGISTER_FACTORY_KEY(  NodeGlarePort, 4095682706931415090 );

BEGIN_PROPERTY_LIST( NodeGlarePort, BaseNodePort );
	ADD_COLOR_PROPERTY( m_Color );
	ADD_PROPERTY( m_Intensity );
	ADD_PROPERTY( m_IntensityV );
	ADD_PROPERTY( m_IntensityA );
	ADD_PROPERTY( m_Focus );
	ADD_PROPERTY( m_Range );
END_PROPERTY_LIST();

NodeGlarePort::NodeGlarePort() : BaseNodePort()
{
	m_Class = m_Type = CLASS_KEY(NodeGlare);
	m_Color = Color(255,255,255).RGB();
	m_Intensity = 1.0f;
	m_IntensityV = m_IntensityA = 0.0f;
	m_Focus = 0.0f;
	m_Range = 32000.0f;
}

//-------------------------------------------------------------------------------

const int VERSION_032000 = 32000;	// TODO: set the current date as the version
const int VERSION_020101 = 20101;

bool NodeGlarePort::read( const InStream & input )
{
	if (! BaseNodePort::read( input ) )
	{
		int version;
		input >> version;

		switch( version )
		{
		case VERSION_020101:
			input >> m_Color;
			input >> m_Intensity;
			input >> m_IntensityV;
			input >> m_IntensityA;
			input >> m_Focus;
			input >> m_Range;
			break;
		case VERSION_032000:
			input >> m_Color;
			input >> m_Intensity;
			input >> m_IntensityV;
			input >> m_IntensityA;
			input >> m_Focus;
			m_Range = 32000.0f;
			break;
		}
	}
	return true;
}

//-------------------------------------------------------------------------------

void NodeGlarePort::dependencies( DependentArray & dep )
{}

CFrameWnd * NodeGlarePort::createView()
{
	return BaseNodePort::createView();
}

BaseNode * NodeGlarePort::createNode()
{
	return BaseNodePort::createNode();
}

void NodeGlarePort::initializeNode( BaseNode * pNode )
{
	BaseNodePort::initializeNode( pNode );

	NodeGlare * pGlare = dynamic_cast<NodeGlare *>( pNode );
	if ( pGlare != NULL )
	{
		pGlare->setColor( m_Color );
		pGlare->setIntensity( m_Intensity, m_IntensityV, m_IntensityA );
		pGlare->setFocus( m_Focus );
		pGlare->setFalloff( 1.0f / m_Range );
	}
}


//-------------------------------------------------------------------------------
// EOF

