/*
	WindowMenu.cpp

	GUI object for a menu
	(c)2005 Palestar, Richard Lyle
*/

#define GUI3D_DLL
#include "Debug/Assert.h"
#include "System/Keyboard.h"
#include "Gui3d/WindowButton.h"
#include "Gui3d/WindowMenu.h"

//----------------------------------------------------------------------------

IMPLEMENT_FACTORY( WindowMenu, WindowText );
REGISTER_FACTORY_KEY(  WindowMenu, 4124869357847020576 );

BEGIN_PROPERTY_LIST( WindowMenu, WindowText );
	ADD_PROPERTY( m_Items );
END_PROPERTY_LIST();

WindowMenu::WindowMenu()
{
	m_Highlighted = -1;
	m_Selected = -1;
	m_MenuInvalid = true;
}

//----------------------------------------------------------------------------

const RectFloat WINDOW_UV(0,0,1,1);

void WindowMenu::onRender( RenderContext & context, const RectInt & window )
{
	// draw highlighted selection
	if ( m_Highlighted >= 0 )
	{
		Font * pFont = windowStyle()->font();
		ASSERT( pFont );

		int height = pFont->size().height;
		int offset = m_Highlighted * height;

		RectInt highWindow( window.m_Left, window.m_Top + offset, window.m_Right, window.m_Top + offset + height );

		PrimitiveMaterial::push( context.display(), PrimitiveMaterial::ADDITIVE );
		PrimitiveWindow::push( context.display(), highWindow, WINDOW_UV, Color(128,128,128,64) );
	}

	// draw highlighted selection
	if ( m_Selected >= 0 )
	{
		Font * pFont = windowStyle()->font();
		ASSERT( pFont );

		int height = pFont->size().height;
		int offset = m_Selected * height;

		RectInt highWindow( window.m_Left, window.m_Top + offset, window.m_Right, window.m_Top + offset + height );

		PrimitiveMaterial::push( context.display(), PrimitiveMaterial::ADDITIVE );
		PrimitiveWindow::push( context.display(), highWindow, WINDOW_UV, Color(128,128,128,128) );
	}

	// call the base class
	WindowText::onRender( context, window );
}

//----------------------------------------------------------------------------

void WindowMenu::onUpdate( float t )
{
	WindowText::onUpdate( t );

	// update the menu
	updateMenu();
}

bool WindowMenu::onMessage( const Message & msg )
{
	switch( msg.message )
	{
	case HM_KEYDOWN:
		{
			for(int i=0;i<m_Items.size();i++)
				if ( msg.wparam == m_Items[i].key )
				{
					m_Highlighted = i;
					onSelectChange( i ); 
					return true;
				}
		}
		break;
	case HM_KEYUP:
		{
			for(int i=0;i<m_Items.size();i++)
				if ( msg.wparam == m_Items[i].key )
				{
					m_Selected = m_Highlighted = i;
					onSelect( i ); 
					return true;
				}
		}
		break;
	}

	return WindowText::onMessage( msg );
}

//----------------------------------------------------------------------------

bool WindowMenu::onCursorMove( const RectInt & window, const PointInt & position, const PointInt & delta )
{
	updateMenu();

	if ( (m_Flags & VISIBLE) && window.inRect( position ) )
	{
		setCursorState( SELECT );
		// determine highlighted item
		Font * pFont = windowStyle()->font();
		ASSERT( pFont );

		int item = position.m_Y / pFont->size().height;
		if ( item >= 0 && item < itemCount() )
		{
			if ( m_Highlighted != item )
			{
				m_Highlighted = item;
				onSelectChange( item ); 
			}
		}
		else if ( m_Highlighted != -1 )
		{
			m_Highlighted = -1;
			onSelectChange( -1 );
		}

		return true;
	}

	return WindowText::onCursorMove( window, position, delta );
}

bool WindowMenu::onLeftDown( const RectInt & window, const PointInt & position )
{
	if ( (m_Flags & VISIBLE) && window.inRect( position) )
		return true;

	return WindowText::onLeftDown( window, position );
}

bool WindowMenu::onLeftUp( const RectInt & window, const PointInt & position )
{
	if ( (m_Flags & VISIBLE) && window.inRect( position ) )
	{
		if ( m_Highlighted >= 0 )
		{
			m_Selected = m_Highlighted;
			onSelect( m_Selected );
		}
		else
		{
			m_Selected = -1;
			onSelect( -1 );
		}
	}

	return WindowText::onLeftUp( window, position );
}

//----------------------------------------------------------------------------

int WindowMenu::addItem( const char * string, dword key )
{
	Item newItem;
	newItem.text = string;
	newItem.key = key;

	return addItem( newItem );
}

int WindowMenu::addItem( const Item & item )
{
	m_MenuInvalid = true;
	m_Items.push( item );
	return m_Items.size() - 1;
}

void WindowMenu::setItem( int i, const Item & item )
{
	m_MenuInvalid = true;
	m_Items[ i ] = item;
}

void WindowMenu::removeItem( int i )
{
	m_MenuInvalid = true;
	m_Items.remove( i );
}

void WindowMenu::removeAllItems()
{
	m_MenuInvalid = true;
	m_Items.release();
}

//----------------------------------------------------------------------------

void WindowMenu::onSelectChange( int item )
{
	// make sound
	hoverSound();
	// post message
	postMessage( WM_SELECT_CHANGE, (dword)this, item );
}

void WindowMenu::onSelect( int item )
{
	// make sound
	selectSound();
	// post message
	postMessage( WM_SELECT, (dword)this, item );
}

//----------------------------------------------------------------------------

const int TAB_WIDTH = 10;
const int KEY_WIDTH = 20;

void WindowMenu::updateMenu()
{
	if ( m_MenuInvalid )
	{
		Font * pFont = windowStyle()->font();
		ASSERT( pFont );

		SizeInt menuSize( 0, pFont->size().m_Height * m_Items.size() );
		int i = 0;
		for(;i<m_Items.size();i++)
			menuSize.m_Width = Max( menuSize.m_Width, pFont->size( m_Items[ i ].text.string() ).m_Width );
		menuSize.m_Width += TAB_WIDTH + KEY_WIDTH;

		// build the menu
		CharString sMenu;
		for(i=0;i<m_Items.size();i++)
		{
			sMenu += m_Items[i].text;
			if ( m_Items[i].key != 0 )
				sMenu += CharString().format("<X;%d>%s", 
				menuSize.m_Width - KEY_WIDTH, WindowButton::keyText( Keyboard::unmap( m_Items[i].key ) ) );
			
			sMenu += "\n";	// newline
		}

		setText( sMenu );
		setWindow( RectInt( m_Window.upperLeft(), menuSize ) );

		m_MenuInvalid = false;
 	}
}

//----------------------------------------------------------------------------
// EOF
