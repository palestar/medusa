/*
	ButtonSetScene.cpp
	(c)2005 Palestar, Richard Lyle
*/

#define GUI3D_DLL
#include "Gui3d/ButtonSetScene.h"

//----------------------------------------------------------------------------

IMPLEMENT_FACTORY( ButtonSetScene, WindowButton );
REGISTER_FACTORY_KEY(  ButtonSetScene, 4060633371130507027 );

ButtonSetScene::ButtonSetScene()
{}

//----------------------------------------------------------------------------

void ButtonSetScene::onButtonUp()
{
	// call the base class
	WindowButton::onButtonUp();
	// send message to the interface to set it's scene
	postMessage( IC_SET_SCENE, StringHash( name() ), 0 );
}

//----------------------------------------------------------------------------
// EOF
