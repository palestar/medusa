/*
	WindowHelp.cpp
	(c)2005 Palestar Inc, Richard Lyle
*/

#define GUI3D_DLL
#include "Display/PrimitiveMaterial.h"
#include "Display/PrimitiveLineStrip.h"

#include "WindowHelp.h"
#include "InterfaceContext.h"

//----------------------------------------------------------------------------

IMPLEMENT_FACTORY( WindowHelp, WindowText );
REGISTER_FACTORY_KEY(  WindowHelp, 4415956196550367095 );

WindowHelp::WindowHelp()
{}

//----------------------------------------------------------------------------

void WindowHelp::onActivate()
{}

void WindowHelp::onDeactivate()
{
	m_TargetWindow = NULL;
}

void WindowHelp::onUpdate( float t )
{
	if ( visible() )
	{
		NodeWindow * pHelpWindow = context()->helpWindow();
		if ( pHelpWindow != NULL )
		{
			if ( m_TargetWindow != pHelpWindow )
			{
				m_TargetWindow = pHelpWindow;
				setText( pHelpWindow->help() );
			}
		}
		else if ( m_TargetWindow.valid() )
		{
			m_TargetWindow = NULL;
			setText( "" );
		}
	}
}


//----------------------------------------------------------------------------
//EOF
