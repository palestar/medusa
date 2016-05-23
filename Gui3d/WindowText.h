/*
	WindowText.h
	(c)2005 Palestar, Richard Lyle
*/

#ifndef WINDOW_TEXT_H
#define WINDOW_TEXT_H

#include "Gui3d/NodeWindow.h"
#include "Gui3d/GUI3DDll.h"

//----------------------------------------------------------------------------

class DLL WindowText : public NodeWindow
{
public:
	DECLARE_WIDGET_CLASS();
	DECLARE_PROPERTY_LIST();

	// Types
	typedef Reference<WindowText>			Ref;

	// Construction
	WindowText();
	WindowText( NodeWindow * pParent, const RectInt & window );

	// NodeInterfaceClient interface
	void				onActivate();									
	void				onDeactivate();

	// NodeWindow interface
	void				onRender( RenderContext & context,
							const RectInt & window );		// client render, window is in screen coordinates
	// Accessors
	const char *		text() const;						// window text
	// Mutators
	void				setText( const char * pText );	// set the window text 
	void				invalidate();						// force cached data to be recalculated

protected:
	// Types
	struct TextLine
	{
		PointInt		position;
		Font::Ref		font;
		WideString		line;
		Color			color;

		TextLine & operator=( const TextLine & copy )
		{
			position = copy.position;
			font = copy.font;
			line = copy.line;
			color = copy.color;
			return *this;
		}
	};
	struct TextImage
	{
		Material::Ref	material;
		RectInt			rect;
		Color			color;

		TextImage & operator=( const TextImage & copy )
		{
			material = copy.material;
			rect = copy.rect;
			color = copy.color;
			return *this;
		}
	};
	struct Connect
	{
		PointInt		src;
		PointInt		dst;
		Color			color;
	};

	// Data
	CharString			m_Text;								// window text

	// non-serialize
	RectInt				m_CacheWindow;						// window state when cache was generated
	Queue< TextLine >	m_LineCache;
	Queue< TextImage >	m_ImageCache;
	Queue< Connect >	m_ConnectCache;

	// Mutators
	void				updateCache( const RectInt & window );
};

//-------------------------------------------------------------------------------

inline const char * WindowText::text() const
{
	return m_Text;
}

//----------------------------------------------------------------------------

inline void WindowText::setText( const char * pText )
{
	invalidate();
	m_Text = pText;
}

inline void WindowText::invalidate()
{
	m_CacheWindow.setInvalid();
}

//----------------------------------------------------------------------------



#endif

//----------------------------------------------------------------------------
// EOF

