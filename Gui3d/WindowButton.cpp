/*
	WindowButton.cpp
	(c)2005 Palestar, Richard Lyle
*/

#define GUI3D_DLL
#include "Debug/Assert.h"
#include "Draw/Draw.h"
#include "Standard/StringHash.h"
#include "Standard/Constant.h"
#include "System/CommandTarget.h"
#include "System/Keyboard.h"

#include "TypeHotKey.h"
#include "WindowTip.h"
#include "ButtonPopup.h"
#include "WindowButton.h"
#include "InterfaceContext.h"

//----------------------------------------------------------------------------

Constant		BUTTON_FADE_IN_TIME( "BUTTON_FADE_IN_TIME", 0.5f );			// how long to fade in windows with the EFFECT_FADEIN style
Constant		BUTTON_GREYED_SCALE( "BUTTON_GREYED_SCALE", 0.5f );

//-------------------------------------------------------------------------------

IMPLEMENT_FACTORY( WindowButton, NodeWindow );
REGISTER_FACTORY_KEY(  WindowButton, 3978639485261632642 );

BEGIN_PROPERTY_LIST( WindowButton, NodeWindow );
	ADD_PROPERTY( m_Toggle );
	ADD_PROPERTY( m_ButtonDown );
	ADD_PROPERTY( m_Label );
	ADD_PROPERTY( m_Button );
	ADD_PROPERTY( m_Tip );
	ADD_HOTKEY_PROPERTY( m_HotKey );
END_PROPERTY_LIST();

WindowButton::WindowButton()
{
	m_Toggle = false;
	m_ButtonDown = false;
	m_CursorOver = false;
	m_HotKey = 0;
	m_Color = Color(255,255,255,255);
	m_HotKeyDown = false;
	m_Time = 0.0f;
	m_bSavedCursorOver = false;
	m_bGreyed = false;
}

//-------------------------------------------------------------------------------

bool WindowButton::read( const InStream & input )
{
	if (! NodeWindow::read( input ) )
		return false;

	m_Color = m_Button.valid() ? m_Button->emissive() : Color(255,255,255,255);
	//m_Label = LocalizedString( m_Label );
	//m_Tip = LocalizedString( m_Tip );
	return true;
}

//----------------------------------------------------------------------------

void WindowButton::onActivate()
{
	NodeWindow::onActivate();

	m_CursorOver = false;
	if ( m_ButtonDown )
		m_Flags |= DEPRESSED;
	else
		m_Flags &= ~DEPRESSED;
}

void WindowButton::onDeactivate()
{}

bool WindowButton::onMessage( const Message & msg )
{
	if ( enabled() )
	{
		switch( msg.message )
		{
		case HM_SYSKEYDOWN:
		case HM_KEYDOWN:
			if ( (msg.message == HM_KEYDOWN && msg.wparam == m_HotKey) || 
				(msg.message == HM_SYSKEYDOWN && (msg.wparam | HMK_ALT_MASK) == m_HotKey) )
			{
				if ( m_ButtonDown && m_Toggle )
				{
					onButtonUp();
					return trapKey();
				}

				m_HotKeyDown = true;
				onButtonDown();

				return trapKey();
			}
			break;
		case HM_SYSKEYUP:
		case HM_KEYUP:
			if ( m_HotKeyDown && (msg.wparam & 0xff) == (m_HotKey & 0xff) )
			{
				if ( !m_Toggle && m_ButtonDown )
					onButtonUp();
				m_HotKeyDown = false;

				return trapKey();
			}
			break;
		}
	}

	return NodeWindow::onMessage( msg );
}

//----------------------------------------------------------------------------

void WindowButton::onHide()
{
	NodeWindow::onHide();

	// if button was held down, release the button now..
	if ( m_ButtonDown && !m_Toggle )
		onButtonUp();

	// save the cursor over state, in some cases a button will be hidden, then unhidden by some button logic, if
	// we don't do this then it makes it impossible to click on those buttons.
	m_bSavedCursorOver = m_CursorOver;
	// clear some booleans
	m_CursorOver = m_HotKeyDown = false;

}

void WindowButton::onShow()
{
	NodeWindow::onShow();
	// restore the previous cursor over state
	m_CursorOver = m_bSavedCursorOver;
}


const int DISABLED_ALPHA = -64;
const int DOWN_ALPHA = 64;
const int OVER_ALPHA = 32;

void WindowButton::onRender( RenderContext & context, const RectInt & window )
{
	int baseAlpha = m_Alpha * 255;
	int alpha = Clamp( baseAlpha + (enabled() ? (m_ButtonDown ? DOWN_ALPHA : m_CursorOver ? OVER_ALPHA : 0) : DISABLED_ALPHA) , 0, 255 );

	// update the image frame
	if ( m_Button.valid() )
	{
		static RectFloat buttonUV( 0, 0, 1, 1 );

		// get the button color
		//Color buttonColor( m_ButtonDown ? windowStyle()->highColor() : windowStyle()->color() );
		Color buttonColor( m_ButtonDown ? m_Color * 2.0f : m_Color );
		buttonColor.m_A = alpha;

		if ( m_bGreyed )
			buttonColor.greyscale(BUTTON_GREYED_SCALE);

		// if the button has multiple frames, set the time to display the correct frame
		if ( m_Button->frames() > 1 )
		{
			ASSERT( m_Button->fps() > 0 );

			int frame = 0;
			if ( m_Button->frames() == 2 )
				frame = m_ButtonDown ? 1 : 0;
			else
				frame = m_ButtonDown ? 1 : m_CursorOver ? 2 : 0;

			m_Time = (1.0f / m_Button->fps()) * frame;
		}

		RectInt buttonRect( window );
		if ( (m_Style & LOCK_ICON_SIZE) != 0 )
		{
			int nDiffuse = m_Button->findTexture( PrimitiveSurface::DIFFUSE );
			if ( nDiffuse >= 0 )
			{
				SizeInt buttonSize( m_Button->texture( nDiffuse ).m_pImage->size() );
				// keep button original size
				buttonRect.setWidth( buttonSize.width );
				buttonRect.setHeight( buttonSize.height );
			}
		}

		// flush the button material, so we can have different frames for each button
		m_Button->flushMaterial();
		// save then change the context time
		float fContextTime = context.time();
		context.setTime( m_Time );
		// push the button material
		Material::push( context, m_Button );
		// draw the button
		PrimitiveWindow::push( context.display(), buttonRect, buttonUV, buttonColor );
		// restore the context time
		context.setTime( fContextTime );
	}

	// display the label
	if ( m_Label.length() > 0 )
	{
		String sLabel = m_Label;

		Font * pFont = windowStyle()->font();
		ASSERT( pFont );

		Color cLabel = labelColor();
		if ( m_Style & EFFECT_FADEIN && visibleTime() < BUTTON_FADE_IN_TIME )
			cLabel.a = (visibleTime() / BUTTON_FADE_IN_TIME) * 255;
		else
			cLabel.a = 255;

		SizeInt labelSize( pFont->size( sLabel ) );
		PointInt labelPos( window.m_Left + ((window.width() / 2) - (labelSize.width / 2)),
			window.m_Top + ((window.height() / 2) - (labelSize.height / 2)) );

		Font::push( context.display(), pFont, labelPos, sLabel, cLabel );
	}

	// display hotkey in lower right corner
	if ( (m_Style & SHOW_HOTKEY) != 0 && m_HotKey != 0 )
	{
		Font * pFont = windowStyle()->font();
		ASSERT( pFont );

		Color color( ((m_CursorOver || m_ButtonDown) && enabled()) ? 
			windowStyle()->highColor() : windowStyle()->color() );

		if ( m_Style & EFFECT_FADEIN && visibleTime() < BUTTON_FADE_IN_TIME )
			color.a = (visibleTime() / BUTTON_FADE_IN_TIME) * 255;
		else
			color.a = 255;

		if ( m_bGreyed )
			color.greyscale(BUTTON_GREYED_SCALE);

		String sKey( Keyboard::keyShortText( Keyboard::unmap( m_HotKey ) ) );
		SizeInt textSize( pFont->size( sKey ) );
		PointInt textPos( window.m_Right - textSize.width, 
			window.m_Bottom - textSize.height );
		
		Font::push( context.display(), pFont, textPos, sKey, color );
	}
}

bool WindowButton::onCursorMove( const RectInt & window, const PointInt & position, const PointInt & delta )
{
	bool bInRect = false;
	if ( !m_HotKeyDown )
	{
		if ( visible() && window.inRect( position ) )
			bInRect = true;
	}

	if ( bInRect )
	{
		if ( !m_bGreyed )
		{
			// display the press button
			context()->setCursorState( SELECT );

			if ( m_HotKey != 0 )
			{
				updateTip( CharString().format("%s  '%s'", m_Tip, 
					keyText( Keyboard::unmap( m_HotKey ) ) ) );
			}
			else
				updateTip( m_Tip );
		}

		if ( !m_CursorOver )
		{
			if ( !m_bGreyed )
			{
				postMessage( WB_OVERBUTTON, (dword)this, 0 );
				hoverSound();
			}

			m_CursorOver = true;
			return false;			// return false on the first time a cursor is over a button, so that the other button can set m_CursorOver to false
		}

		return true;
	}
	else if ( m_CursorOver )
		m_CursorOver = false;

	return NodeWindow::onCursorMove( window, position, delta );
}

bool WindowButton::onLeftDown( const RectInt & window, const PointInt & position )
{
	if ( m_CursorOver )
	{
		if ( !m_bGreyed )
		{
			if ( m_ButtonDown && m_Toggle )
			{
				onButtonUp();
				return true;
			}

			onButtonDown();
		}
		return true;
	}

	return NodeWindow::onLeftDown( window, position );
}

bool WindowButton::onLeftUp( const RectInt & window, const PointInt & position )
{
	if ( m_CursorOver )
	{
		if ( !m_Toggle && !m_bGreyed )
		{
			if ( !m_ButtonDown )
				onButtonDown();
			onButtonUp();
		}
		return true;
	}
	else if (! m_Toggle )
		onButtonAbort();

	return NodeWindow::onLeftUp( window, position );
}

bool WindowButton::onRightDown( const RectInt & window, const PointInt & position )
{
	if ( m_CursorOver )
	{
		if ( !m_bGreyed )
			onMenuDown();
		return true;
	}

	return NodeWindow::onRightDown( window, position );
}

bool WindowButton::onRightUp( const RectInt & window, const PointInt & position )
{
	if ( m_CursorOver )
	{
		if ( !m_bGreyed )
			onMenuUp();
		return true;
	}
	else if (! m_Toggle )		// don't clear toggle buttons just because the mouse right button went up
		onButtonAbort();

	return NodeWindow::onRightUp( window, position );
}

//----------------------------------------------------------------------------

void WindowButton::setTip( const char * pTip )
{
	m_Tip = pTip;
	if ( m_CursorOver )
		updateTip();
}

void WindowButton::updateButton( bool bVisible, bool bEnabled /*= true*/, const char * pTip /*= NULL*/ )
{
	setVisible( bVisible );
	setEnable( bVisible );
	setGreyed( ! bEnabled );

	if ( pTip != NULL )
	{
		if ( bEnabled )
			setTip( pTip );
		else
			setTip( "" );
	}
}

//----------------------------------------------------------------------------

void WindowButton::onButtonDown()
{
	m_ButtonDown = true;
	m_Flags |= DEPRESSED;

	// send message to listeners
	postMessage( WB_BUTTONDOWN, StringHash( name() ), 0 );
}

void WindowButton::onButtonUp()
{
	m_ButtonDown = false;
	m_Flags &= ~DEPRESSED;

	// play select sound
	selectSound();
	// post message to listeners
	postMessage( WB_BUTTONUP, StringHash( name() ), 0 );

	// abort any ButtonPopup parents
	BaseNode * pParent = parent();
	while( pParent != NULL )
	{
		ButtonPopup * pPopup = WidgetCast<ButtonPopup>( pParent );
		if ( pPopup != NULL )
		{
			pPopup->onButtonAbort();
			break;
		}
		// next parent object
		pParent = pParent->parent();
	}
}

void WindowButton::onMenuDown()
{
	m_Flags |= DEPRESSED;

	postMessage( WB_MENUDOWN, StringHash( name() ), 0 );
}

void WindowButton::onMenuUp()
{
	m_Flags &= ~DEPRESSED;

	// post message to listeners
	postMessage( WB_MENUUP, StringHash( name() ), 0 );
}

void WindowButton::onButtonAbort()
{
	m_ButtonDown = false;
	m_Flags &= ~DEPRESSED;
}

bool WindowButton::trapKey() const
{
	// by default do trap the key
	return true;
}

Color WindowButton::labelColor() const
{
	Color c = ((m_CursorOver || m_ButtonDown) && enabled()) ? windowStyle()->highColor() : windowStyle()->color();
	if ( m_bGreyed )
		c.greyscale(BUTTON_GREYED_SCALE);
	return c;
}

//----------------------------------------------------------------------------

CharString WindowButton::keyText( dword key )
{
	return Keyboard::keyText( key );
}

//----------------------------------------------------------------------------

void WindowButton::updateTip()
{
	if ( m_HotKey != 0 )
		updateTip( CharString().format("%s  '%s'", m_Tip, keyText( Keyboard::unmap( m_HotKey ) ) ) );
	else
		updateTip( m_Tip );
}

void WindowButton::updateTip( const char * pTip )
{
	if ( strlen( pTip ) > 0 && rootWindow() != NULL )
	{
		WindowTip * pWindow = WindowTip::getWindowTip( this );
		if ( pWindow != NULL )
			pWindow->setText( pTip );
		else
			new WindowTip( this, pTip );

		// clear any cursor tip..
		setCursorTip( "" );
	}

}

//-------------------------------------------------------------------------------
// EOF
