/*
	Port.cpp
	(c)2005 Palestar, Richard Lyle
*/

#define WINDOWBUTTONPORT_DLL
#include "stdafx.h"

#include "Port.h"
#include "PortView.h"

#include "Gui3d/WindowButton.h"
#include "Gui3d/TypeHotKey.h"
#include "Tools/ScenePort/ChildFrame.h"
#include "Tools/ResourcerDoc/Port.h"


//----------------------------------------------------------------------------

IMPLEMENT_FACTORY( WindowButtonPort, NodeWindowPort );
REGISTER_FACTORY_KEY(  WindowButtonPort, 3978715096304232210 );

BEGIN_PROPERTY_LIST( WindowButtonPort, NodeWindowPort );
	ADD_PROPERTY( m_Label );
	ADD_PORT_PROPERTY( m_Button, MaterialPort );
	ADD_PROPERTY( m_Toggle );
	ADD_PROPERTY( m_ButtonDown );
	ADD_PROPERTY( m_Tip );
	ADD_HOTKEY_PROPERTY( m_HotKey );
END_PROPERTY_LIST();

WindowButtonPort::WindowButtonPort() : NodeWindowPort()
{
	m_Class = m_Type = CLASS_KEY( WindowButton );
	m_Toggle = false;
	m_ButtonDown = false;
	m_HotKey = 0;
}

//-------------------------------------------------------------------------------

const int VERSION_121699 = 121699;
const int VERSION_121799 = 121799;
const int VERSION_031700 = 31700;
const int VERSION_042000 = 42000;
const int VERSION_080102 = 80102;

bool WindowButtonPort::read( const InStream & input )
{
	if (! NodeWindowPort::read( input ) )
	{
		int version;
		input >> version;

		switch( version )
		{
		case VERSION_080102:
			input >> m_Label;
			input >> m_Button;
			input >> m_Toggle;
			input >> m_ButtonDown;
			input >> m_Tip;
			input >> m_HotKey;
			break;
		case VERSION_042000:
			input >> m_Label;
			input >> m_Button;
			input >> m_Toggle;
			input >> m_Tip;
			input >> m_HotKey;
			m_ButtonDown = false;
			break;
		case VERSION_031700:
			input >> m_Label;
			input >> m_Button;
			input >> m_Toggle;
			input >> m_Tip;
			m_HotKey = 0;
			m_ButtonDown = false;
			break;
		case VERSION_121799:
			input >> m_Toggle;
			input >> m_Tip;
			m_HotKey = 0;
			m_ButtonDown = false;
			break;
		case VERSION_121699:
			input >> m_Toggle;
			input >> m_Tip;
			m_HotKey = 0;
			m_Class = m_Type = CLASS_KEY( WindowButton );
			m_ButtonDown = false;
			break;
		default:
			return false;
		}
	}
	return true;
}

//-------------------------------------------------------------------------------

void WindowButtonPort::dependencies( DependentArray & dep )
{
	NodeWindowPort::dependencies( dep );
	dep.push( m_Button );
}

CFrameWnd * WindowButtonPort::createView()
{
	return NodeWindowPort::createView();
}

BaseNode * WindowButtonPort::createNode()
{
	return NodeWindowPort::createNode();	
}

void WindowButtonPort::initializeNode( BaseNode * pNode )
{
	NodeWindowPort::initializeNode( pNode );

	WindowButton * pButton = WidgetCast<WindowButton>( pNode );
	if ( pButton != NULL )
	{
		pButton->setLabel( m_Label );
		pButton->setButton( WidgetCast<Material>( Port::portResource( m_Button ) ) );
		pButton->setToggle( m_Toggle );
		pButton->setButtonDown( m_ButtonDown );
		pButton->setTip( m_Tip );
		pButton->setHotKey( m_HotKey );
	}
}


//-------------------------------------------------------------------------------
// EOF

