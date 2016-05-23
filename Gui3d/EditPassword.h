/*
	EditPassword.h
	(c)2005 Palestar, Richard Lyle
*/

#ifndef EDIT_PASSWORD_H
#define EDIT_PASSWORD_H

#include "Gui3d/WindowEdit.h"
#include "Gui3d/GUI3DDll.h"

//----------------------------------------------------------------------------

class DLL EditPassword : public WindowEdit
{
public:
	DECLARE_WIDGET_CLASS();

	// Types
	typedef Reference<EditPassword>		Ref;

	// Construction
	EditPassword();
	// NodeWindow interface
	void				onRender( RenderContext & context,
							const RectInt & window );		// client render, window is in screen coordinates
};

//-------------------------------------------------------------------------------

#endif

//----------------------------------------------------------------------------
// EOF
