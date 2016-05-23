/*
	Port.cpp
	(c)2005 Palestar, Richard Lyle
*/

#define WINDOWSTYLEPORT_DLL
#include "stdafx.h"

#include "Port.h"

#include "Gui3d/WindowStyle.h"

//----------------------------------------------------------------------------

IMPLEMENT_FACTORY( WindowStylePort, Port );
REGISTER_FACTORY_KEY(  WindowStylePort, 4094072449301428700 );

BEGIN_PROPERTY_LIST( WindowStylePort, Port );
	ADD_PORT_PROPERTY( m_Font, FontPort );
	ADD_PORT_PROPERTY( m_LargeFont, FontPort );
	ADD_PORT_PROPERTY( m_SmallFont, FontPort );
	ADD_COLOR_PROPERTY( m_Color );
	ADD_COLOR_PROPERTY( m_HighColor );
	ADD_COLOR_PROPERTY( m_BorderColor );
	ADD_COLOR_PROPERTY( m_BackColor );
	ADD_COLOR_PROPERTY( m_ScrollBarColor );
	ADD_COLOR_PROPERTY( m_ScrollTabColor );
	ADD_PORT_PROPERTY( m_Background, MaterialPort );
	ADD_PROPERTY( m_BorderSize );
	ADD_PROPERTY( m_ScrollSize );
	ADD_PORT_PROPERTY( m_HoverSound, SoundPort );
	ADD_PORT_PROPERTY( m_SelectSound, SoundPort );
END_PROPERTY_LIST();

WindowStylePort::WindowStylePort() : Port()
{
	m_BorderSize = 0;
	m_ScrollSize = 10;
	m_HighColor = Color(255,255,255,255);
	m_Color = m_BorderColor = m_BackColor = m_ScrollBarColor = m_ScrollTabColor = Color(128,128,128,128);
}

//-------------------------------------------------------------------------------

const int VERSION_031600 = 31600;
const int VERSION_032100 = 32100;
const int VERSION_061400 = 61400;
const int VERSION_020201 = 20201;
const int VERSION_072602 = 72602;

bool WindowStylePort::read( const InStream & input )
{
	if (! Port::read( input ) )
	{
		int version;
		input >> version;

		CharString	m_ScrollBar, m_ScrollTab;		// removed

		switch( version )
		{
		case VERSION_072602:
			input >> m_Font;
			input >> m_LargeFont;
			input >> m_SmallFont;
			input >> m_Color;
			input >> m_HighColor;
			input >> m_BorderColor;
			input >> m_BackColor;
			input >> m_ScrollBarColor;
			input >> m_ScrollTabColor;
			input >> m_Background;
			input >> m_BorderSize;
			input >> m_ScrollSize;
			input >> m_HoverSound;
			input >> m_SelectSound;
			break;
		case VERSION_020201:
			input >> m_Font;
			input >> m_Color;
			input >> m_HighColor;
			input >> m_BorderColor;
			input >> m_BackColor;
			input >> m_ScrollBarColor;
			input >> m_ScrollTabColor;
			input >> m_Background;
			input >> m_BorderSize;
			input >> m_ScrollSize;
			input >> m_HoverSound;
			input >> m_SelectSound;
			break;
		case VERSION_061400:
			input >> m_Font;
			input >> m_Color;
			input >> m_HighColor;
			input >> m_Background;
			input >> m_BorderSize;
			input >> m_ScrollBar;
			input >> m_ScrollTab;
			input >> m_ScrollSize;
			input >> m_HoverSound;
			input >> m_SelectSound;
			m_BorderColor = m_BackColor = m_ScrollBarColor = m_ScrollTabColor = Color(128,128,128,128);
			break;
		case VERSION_032100:
			input >> m_Font;
			input >> m_Color;
			input >> m_HighColor;
			input >> m_Background;
			input >> m_ScrollBar;
			input >> m_ScrollTab;
			input >> m_HoverSound;
			input >> m_SelectSound;
			m_BorderSize = 0;
			m_ScrollSize = 10;
			m_BorderColor = m_BackColor = m_ScrollBarColor = m_ScrollTabColor = Color(128,128,128,128);
			break;
		case VERSION_031600:
			input >> m_Font;
			input >> m_Background;
			input >> m_ScrollBar;
			input >> m_ScrollTab;
			m_BorderSize = 0;
			m_ScrollSize = 10;
			m_BorderColor = m_BackColor = m_ScrollBarColor = m_ScrollTabColor = Color(128,128,128,128);
			break;
		default:
			return false;
		}
	}
	return true;
}

//----------------------------------------------------------------------------

bool WindowStylePort::autoLoad()
{
	return false;
}

void WindowStylePort::dependencies( DependentArray & dep )
{
	dep.push( m_Font );
	dep.push( m_LargeFont );
	dep.push( m_SmallFont );
	dep.push( m_Background );
	dep.push( m_HoverSound );
	dep.push( m_SelectSound );
}

Resource::Ref WindowStylePort::createResource()
{
	WindowStyle * pStyle = new WindowStyle;
	
	pStyle->setFont( Font::Link( Port::portResource( m_Font ) ) );
	pStyle->setLargeFont( Font::Link( Port::portResource( m_LargeFont ) ) );
	pStyle->setSmallFont( Font::Link( Port::portResource( m_SmallFont ) ) );
	pStyle->setColor( m_Color );
	pStyle->setHighColor( m_HighColor );
	pStyle->setBorderColor( m_BorderColor );
	pStyle->setBackColor( m_BackColor );
	pStyle->setScrollColor( m_ScrollBarColor, m_ScrollTabColor );

	pStyle->setBackground( Material::Link( Port::portResource( m_Background ) ) );
	pStyle->setBorderSize( m_BorderSize );
	pStyle->setScrollSize( m_ScrollSize );
	pStyle->setHoverSound( Sound::Link( Port::portResource( m_HoverSound ) ) );
	pStyle->setSelectSound( Sound::Link( Port::portResource( m_SelectSound ) ) );

	// return a created resource
	return pStyle;
}

CFrameWnd * WindowStylePort::createView()
{
	return Port::createView();
}

bool WindowStylePort::importFile( const char * fileName )
{
	return true;	// return true of successful import, false on failure
}

void WindowStylePort::shellOpen() const
{
	MessageBox( NULL, _T("shellOpen() is not valid for this port"), NULL, MB_OK);
}

//-------------------------------------------------------------------------------
// EOF

