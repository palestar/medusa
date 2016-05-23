/*
	TestView.cpp
	(c)2005 Palestar, Richard Lyle
*/

#include "stdafx.h"
#include "TestView.h"

//----------------------------------------------------------------------------

IMPLEMENT_FACTORY( TestView, WindowView::View );
REGISTER_FACTORY_KEY(  TestView, 4096048957556060091 );

BEGIN_PROPERTY_LIST( TestView, WindowView::View );
END_PROPERTY_LIST();

TestView::TestView()
{
	// Construct your view class
}

//----------------------------------------------------------------------------

bool TestView::read( const InStream & input )
{
	View::read( input );
}

bool TestView::write( const OutStream & output)
{
	View::write( output );
	// implement serialization
}

//----------------------------------------------------------------------------

void TestView::onActivate()
{
	//{{BEGIN_DATA_INIT
	m_pList1 = dynamic_cast<WindowList *>( window()->findNode( "List1" ) );
	m_pWindow2 = dynamic_cast<NodeWindow *>( window()->findNode( "Window2" ) );
	m_pWindow1 = dynamic_cast<NodeWindow *>( window()->findNode( "Window1" ) );
	m_pTextWindow = dynamic_cast<WindowText *>( window()->findNode( "TextWindow" ) );
	//END_DATA_INIT}}

	// called when this view is created
}

void TestView::onDeactivate()
{
	// called before this view is destroyed
}

void TestView::onUpdate( float t )
{
	// implement view updating
}

bool TestView::onMessage( const Message & msg )
{
	//{{BEGIN_MSG_MAP
	MESSAGE_MAP_HM( HM_CHAR, onChar);
	MESSAGE_MAP( WB_OVERBUTTON, 3982933029, onButton1Over);
	MESSAGE_MAP( WB_BUTTONUP, 3982933061, onButton2);
	MESSAGE_MAP( WB_BUTTONUP, 3982933029, onButton1);
	//END_MSG_MAP}}

	return false;
}

void TestView::onDocumentUpdate()
{
	// document data has changed, update this view if needed
}

void TestView::onRender( RenderContext & context, const RectInt & window )
{
	// render this view, window is in screen space
}

//----------------------------------------------------------------------------

bool TestView::onButton1(const Message & msg)
{
	return true;
}

bool TestView::onButton2(const Message & msg)
{
	return true;
}

bool TestView::onButton1Over(const Message & msg)
{
	return true;
}

bool TestView::onChar(const Message & msg)
{
	return true;
}

