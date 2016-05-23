/*
	WindowView.cpp

	This class is used to handle a heirarchy of controls and windows, displaying information
	from the document class. This is modeled after MFC's document/view design.

	(c)2005 Palestar, Richard Lyle

	$ Log: $
*/

#define GUI3D_DLL
#include "Debug/Assert.h"
#include "Gui3d/WindowView.h"

//----------------------------------------------------------------------------

IMPLEMENT_FACTORY( WindowView, NodeWindow );
REGISTER_FACTORY_KEY(  WindowView, 4095927739917546200 );

BEGIN_PROPERTY_LIST( WindowView, NodeWindow );
	ADD_PROPERTY( m_ViewClass );
END_PROPERTY_LIST();

IMPLEMENT_NAMED_ABSTRACT_FACTORY( FACTORY_WindowViewView, WindowView::View, Widget, true );

WindowView::WindowView()
{}

//----------------------------------------------------------------------------

void WindowView::onActivate()
{
	NodeWindow::onActivate();

	createViewClass();
	if ( m_View.valid() )
		m_View->onActivate();
}

void WindowView::onDeactivate()
{
	NodeWindow::onDeactivate();
	if ( m_View.valid() )
		m_View->onDeactivate();
}

void WindowView::onUpdate( float t )
{
	NodeWindow::onUpdate( t );
	if ( m_View.valid() )
		m_View->onUpdate( t );
}

bool WindowView::onMessage( const Message & msg )
{
	if ( m_View.valid() )
		if ( m_View->onMessage( msg ) )
			return true;

	return( NodeWindow::onMessage( msg ) );
}

//----------------------------------------------------------------------------

void WindowView::onShow()
{
	if ( m_View.valid() )
		m_View->onShow();
}

void WindowView::onHide()
{
	if ( m_View.valid() )
		m_View->onHide();
}

void WindowView::onRender( RenderContext & context, const RectInt & window )
{
	NodeWindow::onRender( context, window );

	if ( m_View.valid() )
		m_View->onRender( context, window );
}

bool WindowView::onCursorMove( const RectInt & window, 
							const PointInt & position,		// window and position are local coordinates
							const PointInt & delta )
{
	if ( m_View.valid() )
		if ( m_View->onCursorMove( window, position, delta ) )
			return true;

	return NodeWindow::onCursorMove( window, position, delta );
}

bool WindowView::onLeftDown( const RectInt & window, 
							const PointInt & position )
{
	if ( m_View.valid() )
		if ( m_View->onLeftDown( window, position ) )
			return true;

	return NodeWindow::onLeftDown( window, position );
}

bool WindowView::onLeftUp( const RectInt & window, 
							const PointInt & position )
{
	if ( m_View.valid() )
		if ( m_View->onLeftUp( window, position ) )
			return true;

	return NodeWindow::onLeftUp( window, position );
}

bool WindowView::onRightDown( const RectInt & window,
							const PointInt & position )
{
	if ( m_View.valid() )
		if ( m_View->onRightDown( window, position ) )
			return true;

	return NodeWindow::onRightDown( window, position );
}

bool WindowView::onRightUp( const RectInt & window,
							const PointInt & position )
{
	if ( m_View.valid() )
		if ( m_View->onRightUp( window, position ) )
			return true;

	return NodeWindow::onRightUp( window, position );
}

//----------------------------------------------------------------------------

void WindowView::createViewClass()
{
	Factory * pFactory = Factory::findFactory( m_ViewClass );
	if ( pFactory != NULL )
	{
		// create the view class!
		m_View = WidgetCast<View>( pFactory->createWidget() );
		ASSERT( m_View.valid() );

		m_View->setWindow( this );
		TRACE( CharString().format("View class %s created!", pFactory->className() ) );
	}
	else
	{
		TRACE( CharString().format("Failed to find factory for view class %s!", m_ViewClass.string() ) );
	}
}

//----------------------------------------------------------------------------
// EOF
