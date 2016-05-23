/*
	WindowButton.h

	Displays a image and/or text in the window, reacts by sending a message when clicked on
	(c)2005 Palestar, Richard Lyle
*/

#ifndef WINDOW_BUTTON_H
#define WINDOW_BUTTON_H

#include "Draw/Font.h"
#include "Gui3d/NodeWindow.h"
#include "Gui3d/GUI3DDll.h"

//----------------------------------------------------------------------------

class DLL WindowButton : public NodeWindow
{
public:
	DECLARE_WIDGET_CLASS();
	DECLARE_PROPERTY_LIST();
	
	// Types
	typedef Reference<WindowButton>		Ref;

	enum ButtonStyle
	{
		SHOW_HOTKEY		= NodeWindow::USER1,			// display the button hotkey in the lower right
		LOCK_ICON_SIZE	= NodeWindow::USER2,			// do not stretch the icon over the button
	};

	// Construction
	WindowButton();
	// Widget interface
	bool				read( const InStream & input );
	// NodeInterfaceClient interface
	void				onActivate();	
	void				onDeactivate();
	bool				onMessage( const Message & msg );

	// NodeWindow Interface
	void				onHide();
	void				onShow();
	void				onRender( RenderContext & context,
							const RectInt & window );		// client render, window is in screen coordinates
	bool				onCursorMove( const RectInt & window, const PointInt & position, const PointInt & delta );
	bool				onLeftDown( const RectInt & window, const PointInt & position );
	bool				onLeftUp( const RectInt & window, const PointInt & position );
	bool				onRightDown( const RectInt & window, const PointInt & position );
	bool				onRightUp( const RectInt & window, const PointInt & position );

	// Accessors
	const char *		label() const;
	Material *			icon() const;
	Color				iconColor() const;
	bool				toggle() const;
	const char *		tip() const;
	dword				hotKey() const;

	bool				cursorOver() const;
	bool				buttonDown() const;
	bool				hotkeyDown() const;
	float				time() const;

	Material *			button() const;
	bool				greyed() const;						// is button greyed out

	// Mutators
	void				setLabel( const char * label );		// set the button label
	void				setIcon( Material * pIcon );		// set the button icon
	void				setIconColor( Color color );		// set the icon color
	void				setToggle( bool toggle );			// set as a toggle button true, false for normal button
	void				setTip( const char * pTip );
	void				setHotKey( dword key );

	void				setButtonDown( bool down );
	void				setTime( float t );

	void				setButton( Material * pButton );	// set the button material (depreciated, use setIcon() )
	void				setColor( Color color );			// override the material color (depreciated, see setIconColor())
	void				setGreyed( bool bGreyed );			// set button greyed out
	
	void				updateButton( bool bVisible, bool bEnabled = true, const char * pTip = NULL );

	// client interface
	virtual void		onButtonDown();						// called on button depressed
	virtual void		onButtonUp();						// called on button release, action
	virtual void		onMenuDown();						// called when right mouse button is clicked on this button
	virtual void		onMenuUp();							// called when right mouse button is released
	virtual void		onButtonAbort();					// button released, but outside the button rectangle

	virtual bool		trapKey() const;					// should this button trap the key
	virtual Color		labelColor() const;					// allow override of the label color

	// static
	static CharString	keyText( dword key );				// translate a keycode to a text string

protected:
	// Data
	bool				m_Toggle;							// is this a toggle button
	bool				m_ButtonDown;						// is button depressed
	CharString			m_Label;							
	Material::Link		m_Button;							// button material
	CharString			m_Tip;								
	dword				m_HotKey;							// button hotkey

	// non-serialized
	Color				m_Color;							// button color
	bool				m_CursorOver;						// is the cursor over this button
	bool				m_HotKeyDown;						// is hotkey down
	float				m_Time;								// current time for button
	bool				m_bSavedCursorOver;					// saved cursor over state
	bool				m_bGreyed;							// button is greyed out

	// Helpers
	void				updateTip();						// update our tool tip
	void				updateTip( const char * pTip );	// update the tool tip
};

//----------------------------------------------------------------------------

inline const char * WindowButton::label() const
{
	return m_Label;
}

inline Material * WindowButton::icon() const
{
	return m_Button;
}

inline Color WindowButton::iconColor() const
{
	return m_Color;
}

inline Material * WindowButton::button() const
{
	return m_Button;
}

inline bool WindowButton::toggle() const
{
	return m_Toggle;
}

inline const char * WindowButton::tip() const
{
	return m_Tip;
}

inline dword WindowButton::hotKey() const
{
	return m_HotKey;
}

inline bool WindowButton::cursorOver() const
{
	return m_CursorOver;
}

inline bool WindowButton::buttonDown() const
{
	return m_ButtonDown;
}

inline bool WindowButton::hotkeyDown() const
{
	return m_HotKeyDown;
}

inline float WindowButton::time() const
{
	return m_Time;
}

inline bool WindowButton::greyed() const
{
	return m_bGreyed;
}

//----------------------------------------------------------------------------

inline void WindowButton::setLabel( const char * label )
{
	m_Label = label;
}

inline void WindowButton::setIcon( Material * pIcon )
{
	m_Button = pIcon;
	m_Color = m_Button.valid() ? m_Button->emissive() : Color(255,255,255,255);
}

inline void WindowButton::setIconColor( Color color )
{
	m_Color = color;
}

inline void WindowButton::setToggle( bool toggle )
{
	m_Toggle = toggle;
}

inline void WindowButton::setHotKey( dword key )
{
	m_HotKey = key;
}

inline void WindowButton::setButtonDown( bool down )
{
	if (! m_bGreyed )
	{
		if ( down && !m_ButtonDown )
			onButtonDown();
		else if ( !down && m_ButtonDown )
			onButtonUp();
	}
	else
		m_ButtonDown = down;
}

inline void WindowButton::setTime( float t )
{
	m_Time = t;
}

//----------------------------------------------------------------------------

inline void	WindowButton::setButton( Material * pButton )
{
	setIcon( pButton );
}

inline void WindowButton::setColor( Color color )
{
	m_Color = color;
}

inline void WindowButton::setGreyed( bool bGreyed )
{
	m_bGreyed = bGreyed;
}

//-------------------------------------------------------------------------------



#endif

//----------------------------------------------------------------------------
// EOF

