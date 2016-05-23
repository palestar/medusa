/*
	LayoutCenter.cpp
	(c)2005 Palestar Inc, Richard Lyle
*/

#define GUI3D_DLL
#include "Gui3d/LayoutCenter.h"

//----------------------------------------------------------------------------

IMPLEMENT_FACTORY( LayoutCenter, WindowLayout );
REGISTER_FACTORY_KEY(  LayoutCenter, 4186566976394593668 );

LayoutCenter::LayoutCenter()
{
	m_Style |= H_CENTER;
}

//----------------------------------------------------------------------------
//EOF
