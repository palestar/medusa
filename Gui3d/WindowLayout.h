/*
	WindowLayout.h

	This is the base class for all layout classes, a layout class is used to organize child windows
	(c)2005 Palestar Inc, Richard Lyle
*/

#ifndef WINDOWLAYOUT_H
#define WINDOWLAYOUT_H

#include "Gui3d/NodeWindow.h"
#include "Gui3d/GUI3DDll.h"

//----------------------------------------------------------------------------

class DLL WindowLayout : public NodeWindow
{
public:
	DECLARE_WIDGET_CLASS();
	DECLARE_PROPERTY_LIST();

	// Types
	enum Constants
	{
		H_SPACING		= 2,
		V_SPACING		= 2,
	};
	enum Style
	{
		H_LEFT			= 0,
		H_CENTER		= NodeWindow::USER1,
		H_RIGHT			= NodeWindow::USER2,

		V_TOP			= 0,
		V_BOTTOM		= NodeWindow::USER3,

		H_SPACING_X2	= NodeWindow::USER4,				// double the horz spacing between child windows
		V_SPACING_X2	= NodeWindow::USER5,				// double the vert spacing between child windows
		SORT			= NodeWindow::USER6,				// sort child nodes by name

		LAYOUT_VERT		= NodeWindow::USER7,				// layout vertically instead of horz
	};

	// Construction
	WindowLayout();

	// NodeWindow Interface
	void				onRender( RenderContext & context,
							const RectInt & window );		// client render, window is in screen coordinates

	// Mutators
	virtual void		doLayout();

protected:
	// Data
};

//----------------------------------------------------------------------------



#endif

//----------------------------------------------------------------------------
//EOF
