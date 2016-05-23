/*
	WindowStyle.cpp

	Window style class, NodeWindow objects link to this object to get the resources for their style
	(c)2005 Palestar, Richard Lyle
*/

#define GUI3D_DLL
#include "Debug/Trace.h"
#include "Gui3d/WindowStyle.h"

//----------------------------------------------------------------------------

IMPLEMENT_RESOURCE_FACTORY( WindowStyle, Resource );

BEGIN_PROPERTY_LIST( WindowStyle, Resource );
	ADD_PROPERTY( m_Font );
	ADD_PROPERTY( m_LargeFont );
	ADD_PROPERTY( m_SmallFont );
	ADD_PROPERTY( m_Color );
	ADD_PROPERTY( m_HighColor );
	ADD_PROPERTY( m_BorderColor );
	ADD_PROPERTY( m_BackColor );
	ADD_PROPERTY( m_ScrollBarColor );
	ADD_PROPERTY( m_ScrollTabColor );
	ADD_PROPERTY( m_Background );
	ADD_PROPERTY( m_BorderSize );
	ADD_PROPERTY( m_ScrollSize );
	ADD_PROPERTY( m_HoverSound );
	ADD_PROPERTY( m_SelectSound );
END_PROPERTY_LIST();

WindowStyle::WindowStyle()
{
	m_Color = Color(128,128,128,128);
	m_HighColor = Color(255,255,255,255);
	m_BorderSize = 0;
	m_ScrollSize = 10;
}

WindowStyle::WindowStyle( const WindowStyle & copy ) : Resource()
{
	m_Font = copy.m_Font;
	m_LargeFont = copy.m_LargeFont;
	m_SmallFont = copy.m_SmallFont;
	m_Color = copy.m_Color;
	m_HighColor = copy.m_HighColor;
	m_BorderColor = copy.m_BorderColor;
	m_BackColor = copy.m_BackColor;
	m_ScrollBarColor = copy.m_ScrollBarColor;
	m_ScrollTabColor = copy.m_ScrollTabColor;
	m_Background = copy.m_Background;
	m_BorderSize = copy.m_BorderSize;
	m_ScrollSize = copy.m_ScrollSize;
	m_HoverSound = copy.m_HoverSound;
	m_SelectSound = copy.m_SelectSound;
}

//---------------------------------------------------------------------------------------------------

bool WindowStyle::read( const InStream & input )
{
	if (! Resource::read( input ) )
		return false;

	// background load all our assets...
	m_Font.load( false );
	m_LargeFont.load( false );
	m_SmallFont.load( false );
	m_HoverSound.load( false );
	m_SelectSound.load( false );

	return true;
}

//----------------------------------------------------------------------------
// EOF
