/*
	ButtonExit.cpp
	(c)2005 Palestar, Richard Lyle
*/

#define GUI3D_DLL
#include "Gui3d/ButtonExit.h"

//----------------------------------------------------------------------------

IMPLEMENT_FACTORY( ButtonExit, WindowButton );
REGISTER_FACTORY_KEY(  ButtonExit, 4060808707227961458 );

ButtonExit::ButtonExit()
{}

//----------------------------------------------------------------------------

void ButtonExit::onButtonUp()
{
	// call the base class
	WindowButton::onButtonUp();
	// send message to the interface to exit
	postMessage( IC_EXIT, 0, 0 );
}

//----------------------------------------------------------------------------
// EOF
