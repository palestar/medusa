/*
	LayoutRight.cpp
	(c)2005 Palestar Inc, Richard Lyle
*/

#define GUI3D_DLL
#include "Gui3d/LayoutRight.h"

//----------------------------------------------------------------------------

IMPLEMENT_FACTORY( LayoutRight, WindowLayout );
REGISTER_FACTORY_KEY(  LayoutRight, 4186561334851698608 );

LayoutRight::LayoutRight()
{
	m_Style |= H_RIGHT;
}

//----------------------------------------------------------------------------
//EOF
