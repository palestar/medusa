/*
	WindowEdit.h

	This GUI object allows the user to edit the text in the NodeWindow
	(c)2005 Palestar, Richard Lyle
*/

#ifndef WINDOW_EDIT_H
#define WINDOW_EDIT_H

#include "Gui3d/NodeWindow.h"
#include "Gui3d/GUI3DDll.h"

//----------------------------------------------------------------------------

class DLL WindowEdit : public NodeWindow
{
public:
	DECLARE_WIDGET_CLASS();
	DECLARE_PROPERTY_LIST();

	// Types
	typedef Reference<WindowEdit>		Ref;

	// Construction
	WindowEdit();

	// NodeInterfaceClient interface
	void				onActivate();
	void				onDeactivate();
	bool				onMessage( const Message & msg );
	// NodeWindow interface
	void				onRender( RenderContext & context,
							const RectInt & window );		// client render, window is in screen coordinates
	bool				onCursorMove( const RectInt & window, const PointInt & position, const PointInt & delta );
	bool				onLeftDown( const RectInt & window, const PointInt & position );
	bool				onLeftUp( const RectInt & window, const PointInt & position );

	// Accessors
	const char *		text() const;
	bool				editing() const;
	// Mutators
	void				setText( const char * pText );

	virtual void		onBeginEdit();
	virtual void		onEndEdit();
	virtual void		onAbortEdit();

protected:
	// Data
	CharString			m_Text;				// editing

	bool				m_Editing;			// true when window has focus
	bool				m_FirstChar;		// true before first character is entered

	int					m_Cursor;			// cursor position in m_Text
	int					m_EditBegin;		// the first visible character
	int					m_EditEnd;			// the last visible character

	CharString			m_SavedText;		// original text, in case the user hits ESC
};

//----------------------------------------------------------------------------

inline const char * WindowEdit::text() const
{
	return m_Text;
}

inline bool WindowEdit::editing() const
{
	return m_Editing;
}

inline void WindowEdit::setText( const char * pText )
{
	m_Text = pText;
}

//-------------------------------------------------------------------------------



#endif

//----------------------------------------------------------------------------
// EOF
