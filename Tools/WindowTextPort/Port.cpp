/*
	Port.cpp
	(c)2005 Palestar, Richard Lyle
*/

#define WINDOWTEXTPORT_DLL
#include "stdafx.h"

#include "Port.h"
#include "PortView.h"

#include "Tools/ScenePort/ChildFrame.h"
#include "Gui3d/WindowText.h"

//----------------------------------------------------------------------------

IMPLEMENT_FACTORY( WindowTextPort, NodeWindowPort );
REGISTER_FACTORY_KEY(  WindowTextPort, 4094094801131221995 );

BEGIN_PROPERTY_LIST( WindowTextPort, NodeWindowPort );
	ADD_PROPERTY( m_Text );
END_PROPERTY_LIST();

WindowTextPort::WindowTextPort() : NodeWindowPort()
{
	m_Type = m_Class = CLASS_KEY( WindowText );
}

//-------------------------------------------------------------------------------

const int VERSION_031600 = 31600;

bool WindowTextPort::read( const InStream & input )
{
	if (! NodeWindowPort::read( input ) )
	{
		int version;
		input >> version;

		switch( version )
		{
		case VERSION_031600:
			input >> m_Text;
			break;
		default:
			return false;
		}
	}

	return true;
}

//-------------------------------------------------------------------------------

void WindowTextPort::dependencies( DependentArray & dep )
{
	NodeWindowPort::dependencies( dep );
}

CFrameWnd * WindowTextPort::createView()
{
	return NodeWindowPort::createView();
}

BaseNode * WindowTextPort::createNode()
{
	return NodeWindowPort::createNode();
}

void WindowTextPort::initializeNode( BaseNode * pNode )
{
	NodeWindowPort::initializeNode( pNode );

	WindowText * pWindow = dynamic_cast<WindowText *>( pNode );
	if ( pWindow != NULL )
		pWindow->setText( m_Text );
}


//-------------------------------------------------------------------------------
// EOF

