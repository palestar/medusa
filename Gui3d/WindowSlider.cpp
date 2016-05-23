/*
	WindowSlider.cpp
	(c)2005 Palestar, Richard Lyle
*/

#define GUI3D_DLL
#include "Gui3d/WindowSlider.h"

//----------------------------------------------------------------------------

IMPLEMENT_FACTORY( WindowSlider, WindowBar );
REGISTER_FACTORY_KEY(  WindowSlider, 4097041083671617915 );

BEGIN_PROPERTY_LIST( WindowSlider, WindowBar );
	ADD_PROPERTY( m_Sliding );
END_PROPERTY_LIST();

WindowSlider::WindowSlider() : m_Sliding( false )
{}

//----------------------------------------------------------------------------

bool WindowSlider::onCursorMove( const RectInt & window, const PointInt & position, const PointInt & delta )
{
	if ( m_Sliding )
	{
		if ( window.width() > window.height() )
		{
			// snap position to current mouse position
			int dx = position.x - window.left;
			m_Position = Clamp( (1.0f / window.size().width) * dx, 0.0f, 1.0f );
		}
		else
		{
			int dy = position.y - window.top;
			m_Position = Clamp( (1.0f / window.size().height) * dy, 0.0f, 1.0f );
		}
		return true;
	}

	if ( visible() && enabled() && window.inRect( position ) )
	{
		if ( window.width() > window.height() )
			setCursorState( HARROW );
		else
			setCursorState( VARROW );

		return true;
	}
	return false;
}

bool WindowSlider::onLeftDown( const RectInt & window, const PointInt & position )
{
	if ( visible() && enabled() && window.inRect( position ) )
	{
		if ( window.width() > window.height() )
		{
			// snap position to current mouse position
			int dx = position.x - window.left;
			m_Position = Clamp( (1.0f / window.size().width) * dx, 0.0f, 1.0f );
		}
		else
		{
			int dy = position.y - window.top;
			m_Position = Clamp( (1.0f / window.size().height) * dy, 0.0f, 1.0f );
		}

		postMessage( WS_BEGIN_SLIDE, *((dword *)&m_Position), 0 );
		m_Sliding = true;
		return true;
	}
	return false;
}

bool WindowSlider::onLeftUp( const RectInt & window, const PointInt & position )
{
	if ( m_Sliding )
	{
		postMessage( WS_END_SLIDE, *((dword *)&m_Position), 0 );
		m_Sliding = false;
		return true;
	}
	return false;
}

//----------------------------------------------------------------------------
// EOF
