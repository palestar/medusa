/*
	WindowSlider.h
	(c)2005 Palestar, Richard Lyle
*/

#ifndef WINDOW_SLIDER_H
#define WINDOW_SLIDER_H

#include "Gui3d/WindowBar.h"
#include "Gui3d/GUI3DDll.h"

//----------------------------------------------------------------------------

class DLL WindowSlider : public WindowBar
{
public:
	DECLARE_WIDGET_CLASS();
	DECLARE_PROPERTY_LIST();

	// Types
	typedef Reference<WindowSlider>		Ref;

	// Construction
	WindowSlider();

	// NodeWindow Interface
	bool				onCursorMove( const RectInt & window, const PointInt & position, const PointInt & delta );
	bool				onLeftDown( const RectInt & window, const PointInt & position );
	bool				onLeftUp( const RectInt & window, const PointInt & position );

	// Accessors
	bool				sliding() const;

protected:
	// Data
	bool				m_Sliding;
};

//----------------------------------------------------------------------------

inline bool WindowSlider::sliding() const
{
	return m_Sliding;
}

//-------------------------------------------------------------------------------

#endif

//----------------------------------------------------------------------------
// EOF
