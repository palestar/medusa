/*
	EditPassword.cpp
	(c)2005 Palestar, Richard Lyle
*/

#define GUI3D_DLL
#include "Debug/Assert.h"
#include "Draw/Draw.h"
#include "System/Messages.h"

#include "Gui3d/EditPassword.h"

#include <ctype.h>

//-------------------------------------------------------------------------------

IMPLEMENT_FACTORY( EditPassword, WindowEdit );
REGISTER_FACTORY_KEY(  EditPassword, 4097044393062231240 );

EditPassword::EditPassword()
{}

//----------------------------------------------------------------------------

void EditPassword::onRender( RenderContext & context, const RectInt & window )
{
	// save the text
	CharString password( m_Text );
	// replace the text with '*'
	m_Text.allocate( '*', m_Text.length() );
	// render with the base class
	WindowEdit::onRender( context, window );
	// restore the text
	m_Text = password;
}


//-------------------------------------------------------------------------------
// EOF
