/*
	WindowEdit.cpp
	(c)2005 Palestar, Richard Lyle
*/

#define GUI3D_DLL
#include "Debug/Assert.h"
#include "Draw/Draw.h"
#include "System/Messages.h"

#include "WindowEdit.h"
#include "InterfaceContext.h"

#include <ctype.h>

//-------------------------------------------------------------------------------

IMPLEMENT_FACTORY( WindowEdit, NodeWindow );
REGISTER_FACTORY_KEY(  WindowEdit, 3982375192364194554 );

BEGIN_PROPERTY_LIST( WindowEdit, NodeWindow );
END_PROPERTY_LIST();

WindowEdit::WindowEdit()
{
	m_Editing = false;
	m_FirstChar = false;
	m_EditBegin = 0;
	m_Cursor = 0;
	m_EditEnd = 0;
}

//-------------------------------------------------------------------------------

void WindowEdit::onActivate()
{
	NodeWindow::onActivate();
}

void WindowEdit::onDeactivate()
{
	NodeWindow::onDeactivate();
}

bool WindowEdit::onMessage( const Message & msg )
{
	if ( m_Editing )
	{
		if ( msg.message == HM_CHAR )
		{
			if ( msg.wparam == HK_RETURN || msg.wparam == HK_TAB )
			{
				onEndEdit();

				if ( msg.wparam == HK_TAB )	// find next edit box in the heriarchy, and set it active
				{
					NodeWindow * pParent = parentWindow();

					for(int i=0;i<pParent->childCount();i++)
						if ( WidgetCast<WindowEdit>( pParent->child(i) ) )
						{
							((WindowEdit *)pParent->child(i))->onBeginEdit();
							break;
						}
				}

				return true;
			}
			else if ( msg.wparam == HK_ESCAPE )
			{
				onAbortEdit();
				return true;
			}
			else if ( msg.wparam == HK_BACK )
			{
				if ( m_FirstChar )
				{
					m_Text = "";
					m_FirstChar = false;
					m_Cursor = 0;
				}
				else if ( m_Cursor > 0 && m_Text.length() > 0 )
				{
					m_Text.remove( m_Cursor - 1 );
					m_Cursor--;
				}

				return true;
			}
			else if ( isprint( msg.wparam ) )
			{
				m_FirstChar = false;

				// insert new character @ m_Cursor
				m_Text.insert( msg.wparam, m_Cursor );
				m_Cursor++;

				return true;
			}

			return true;
		}
		
		if ( msg.message == HM_KEYDOWN )
		{
			if ( msg.wparam == HK_LEFT )
			{
				m_FirstChar = false;
				if ( m_Cursor > 0 )
					m_Cursor--;
			}
			else if ( msg.wparam == HK_RIGHT )
			{
				m_FirstChar = false;
				if ( m_Cursor < m_Text.length() )
					m_Cursor++;
			}
			else if ( msg.wparam == HK_DELETE )
			{
				if ( m_FirstChar )
				{
					m_Text = "";
					m_FirstChar = false;
					m_Cursor = 0;
				}
				else if ( m_Cursor < m_Text.length() )
					m_Text.remove( m_Cursor );
			}
			else if ( msg.wparam == HK_HOME )
			{
				m_FirstChar = false;
				if ( m_Cursor > 0 )
					m_Cursor = 0;
			}
			else if ( msg.wparam == HK_END )
			{
				m_FirstChar = false;
				if ( m_Cursor < m_Text.length() )
					m_Cursor = m_Text.length();
			}
			return true;
		}
		
		if ( msg.message == HM_KEYUP )
			return true;
	}

	// call base class
	return( NodeWindow::onMessage( msg ) );
}

//----------------------------------------------------------------------------

const int		CURSOR_WIDTH = 2;
const float		CURSOR_BLINK_RATE = 1.0f;
const int		ACTIVE_ALPHA = 255;
const int		INACTIVE_ALPHA = 190;

void WindowEdit::onRender( RenderContext & context, const RectInt & window )
{
	if (! windowStyle() )
		return;

	DisplayDevice * pDisplay = context.display();
	ASSERT( pDisplay );
	Font * pFont = windowStyle()->font();
	ASSERT( pFont );

	// calculate the total text size
	SizeInt textSize( pFont->size( WideString( m_Text ) ) );
	// get the text color
	Color textColor( windowStyle()->color() );
	textColor.m_A = m_Editing ? ACTIVE_ALPHA : INACTIVE_ALPHA;

	SizeInt windowSize( window.size() );

	// check the size of the window compared to the text size
	m_EditBegin = 0;
	m_EditEnd = m_Text.length() - 1;
	while ( textSize.m_Width > windowSize.m_Width )
	{
		if ( m_EditBegin < m_Cursor )
		{
			textSize.m_Width -= pFont->characterWidth( m_Text[ m_EditBegin ] );
			m_EditBegin++;
		}
		else if ( m_EditEnd > m_Cursor )
		{
			textSize.m_Width -= pFont->characterWidth( m_Text[ m_EditEnd ] );
			m_EditEnd--;
		}
		else	// not enough room, the font is probably too large so just return
			return;
	}

	// extract the displayable part of the text
	WideString display( m_Text );
	display.mid( m_EditBegin, (m_EditEnd - m_EditBegin) + 1 );

	// draw text left justified and centered vertically
	PointInt ptText( window.m_Left, window.m_Top + ((windowSize.m_Height / 2) - (textSize.m_Height / 2)) );
	// draw the text, construct another PointInt on the stack because Font::push will modify the point
	PointInt ptText2 = ptText;
	Font::push( pDisplay, pFont, ptText2, display, textColor );

	// display cursor if editing
	if ( m_Editing )
	{
		// draw cursor
		for(int i=m_EditBegin;i<m_Cursor;i++)
			ptText.m_X += pFont->characterWidth( m_Text[i] );

		Color cursorColor( textColor );
		cursorColor.m_A = (u8)(fmod( m_ActiveTime, CURSOR_BLINK_RATE ) * 255);

		RectInt cursorRect( ptText, SizeInt( CURSOR_WIDTH, textSize.height ) );

		PrimitiveMaterial::push( pDisplay, PrimitiveMaterial::ADDITIVE );
		PrimitiveWindow::push( pDisplay, cursorRect , RectFloat(0,0,0,0), cursorColor );
	}
}

bool WindowEdit::onCursorMove( const RectInt & window, const PointInt & position, const PointInt & delta )
{
	if ( window.inRect( position ) || m_Editing )
	{
		setCursorState( EDIT );
		return true;
	}

	return false;
}

bool WindowEdit::onLeftDown( const RectInt & window, const PointInt & position )
{
	if ( window.inRect( position ) )
	{
		if ( m_Editing )
		{
			Font * pFont = windowStyle()->font();
			ASSERT( pFont );

			// get pixel offset from left side of window
			int offset = position.m_X - window.m_Left;
			// move cursor to position in text
			int i = m_EditBegin;
			for(;i<=m_EditEnd && offset > 0;i++)
				offset -= pFont->characterWidth( m_Text[i] );
			// set the cursor position
			m_Cursor = i;

			return true;
		}

		// begin editing
		onBeginEdit();
		return true;
	}
	else
	{
		if ( m_Editing )
		{
			onEndEdit();
			return true;
		}
	}

	return false;
}

bool WindowEdit::onLeftUp( const RectInt & window, const PointInt & position )
{
	if ( window.inRect( position ) )
	{
		return true;
	}
	else
	{
		if ( m_Editing )
		{
			onEndEdit();
			return true;
		}
	}

	return false;
}

//----------------------------------------------------------------------------

void WindowEdit::onBeginEdit()
{
	m_Editing = true;
	m_FirstChar = true;
	m_SavedText = m_Text;
	m_Cursor = m_Text.length();
	m_EditBegin = 0;
	m_EditEnd = m_Cursor - 1;

	// set the focus to this edit window
	if ( m_Style & CAN_FOCUS )
		context()->setFocus( this );
	context()->lockFocus( this );

	postMessage( WE_BEGINEDIT, (dword)this, 0 );
}

void WindowEdit::onEndEdit()
{
	m_Editing = false;

	// post message that edit is complete
	postMessage( WE_EDITDONE, (dword)this, 0 );
	// unlock the focus
	context()->unlockFocus( this );
}

void WindowEdit::onAbortEdit()
{
	m_Editing = false;
	m_Text = m_SavedText;

	context()->unlockFocus( this );
}

//-------------------------------------------------------------------------------
// EOF
