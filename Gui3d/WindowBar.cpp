/*
	WindowBar.cpp
	(c)2005 Palestar, Richard Lyle
*/

#define GUI3D_DLL
#include "Display/PrimitiveMaterial.h"
#include "Display/PrimitiveWindow.h"
#include "Gui3d/WindowBar.h"

//----------------------------------------------------------------------------

IMPLEMENT_FACTORY( WindowBar, NodeWindow );
REGISTER_FACTORY_KEY(  WindowBar, 4097005835816605346 );

BEGIN_PROPERTY_LIST( WindowBar, NodeWindow );
	ADD_PROPERTY( m_Position );
	ADD_PROPERTY( m_BaseColor );
	ADD_PROPERTY( m_BarColor );
END_PROPERTY_LIST();

WindowBar::WindowBar()
{
	m_Position = 0.5f;
	m_BaseColor = Color(64,64,64,128 );
	m_BarColor = Color(255,0,0,128 );
}

//----------------------------------------------------------------------------

void WindowBar::onRender( RenderContext & context, const RectInt & window )
{
	const RectFloat windowUV(0,0,1,1);

	PrimitiveMaterial::push( context.display(), PrimitiveMaterial::ALPHA );
	PrimitiveWindow::push( context.display(), window, windowUV, m_BaseColor );

	if ( window.width() > window.height() )
	{
		// horizontal bar
		const RectInt barWindow( window.m_Left, window.m_Top, (int)(window.m_Left + (m_Position * window.width())), window.m_Bottom );
		PrimitiveWindow::push( context.display(), barWindow, windowUV, m_BarColor );
	}
	else
	{
		// vertical bar
		const RectInt barWindow( window.m_Left, (int)(window.m_Bottom - (m_Position * window.height())), window.m_Right, window.m_Bottom );
		PrimitiveWindow::push( context.display(), barWindow, windowUV, m_BarColor );
	}
}

//----------------------------------------------------------------------------
// EOF
