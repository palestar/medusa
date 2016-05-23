/*
	ButtonSetScene.h
	(c)2005 Palestar, Richard Lyle
*/

#ifndef BUTTON_SET_SCENE_H
#define BUTTON_SET_SCENE_H

#include "Gui3d/WindowButton.h"
#include "Gui3d/GUI3DDll.h"

//----------------------------------------------------------------------------

class DLL ButtonSetScene : public WindowButton
{
public:
	DECLARE_WIDGET_CLASS();

	// Types
	typedef Reference<ButtonSetScene>		Ref;

	// Construction
	ButtonSetScene();
	// WindowButton Interface
	void				onButtonUp();
};

//-------------------------------------------------------------------------------

#endif

//----------------------------------------------------------------------------
// EOF
