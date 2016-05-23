/*
	ButtonExit.h
	(c)2005 Palestar, Richard Lyle
*/

#ifndef BUTTON_EXIT_H
#define BUTTON_EXIT_H

#include "Gui3d/WindowButton.h"
#include "Gui3d/GUI3DDll.h"

//----------------------------------------------------------------------------

class DLL ButtonExit : public WindowButton
{
public:
	DECLARE_WIDGET_CLASS();

	// Types
	typedef Reference<ButtonExit>		Ref;

	// Construction
	ButtonExit();
	// WindowButton Interface
	void				onButtonUp();
};

//-------------------------------------------------------------------------------


#endif

//----------------------------------------------------------------------------
// EOF
