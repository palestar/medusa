/*
	WindowBar.h
	(c)2005 Palestar, Richard Lyle
*/

#ifndef WINDOW_BAR_H
#define WINDOW_BAR_H

#include "Gui3d/NodeWindow.h"
#include "Gui3d/GUI3DDll.h"

//----------------------------------------------------------------------------

class DLL WindowBar : public NodeWindow
{
public:
	DECLARE_WIDGET_CLASS();
	DECLARE_PROPERTY_LIST();

	// Types
	typedef Reference<WindowBar>		Ref;

	// Construction
	WindowBar();

	// NodeWindow interface
	void				onRender( RenderContext & context,
							const RectInt & window );		// client render, window is in screen coordinates
	// Accessors
	float				position() const;					// position will range between 0.0f - 1.0f
	Color				baseColor() const;
	Color				barColor() const;
	// Mutators
	void				setPosition( float position );
	void				setBaseColor( Color color );
	void				setBarColor( Color color );

protected:
	// Data
	float				m_Position;
	Color				m_BaseColor;
	Color				m_BarColor;
};

//----------------------------------------------------------------------------

inline float WindowBar::position() const
{
	return m_Position;
}

inline Color WindowBar::baseColor() const
{
	return m_BaseColor;
}

inline Color WindowBar::barColor() const
{
	return m_BarColor;
}

//----------------------------------------------------------------------------

inline void WindowBar::setPosition( float position )
{
	m_Position = position;
}

inline void WindowBar::setBaseColor( Color color )
{
	m_BaseColor = color;
}

inline void WindowBar::setBarColor( Color color )
{
	m_BarColor = color;
}

//-------------------------------------------------------------------------------




#endif

//----------------------------------------------------------------------------
// EOF
