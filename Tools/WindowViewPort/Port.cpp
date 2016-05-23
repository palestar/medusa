/*
	Port.cpp
	(c)2005 Palestar, Richard Lyle
*/

#define WINDOWVIEWPORT_DLL
#include "stdafx.h"

#include "Port.h"
#include "PortView.h"

#include "Tools/ScenePort/ChildFrame.h"
#include "Tools/ScenePort/Port.h"
#include "Gui3d/WindowView.h"

//----------------------------------------------------------------------------

IMPLEMENT_FACTORY( WindowViewPort, NodeWindowPort );
REGISTER_FACTORY_KEY(  WindowViewPort, 4095943837046388794 );

BEGIN_PROPERTY_LIST( WindowViewPort, NodeWindowPort );
	ADD_CLASSKEY_PROPERTY( m_ViewClass, WindowView::View );
END_PROPERTY_LIST();

WindowViewPort::WindowViewPort() : NodeWindowPort()
{
	m_Class = m_Type = CLASS_KEY(WindowView);
}

//-------------------------------------------------------------------------------

const int VERSION_032100 = 32100;

bool WindowViewPort::read( const InStream & input )
{
	if (! NodeWindowPort::read( input ) )
	{
		int version;
		input >> version;

		switch( version )
		{
		case VERSION_032100:
			input >> m_ViewClass;
			break;
		default:
			return false;
		}
	}
	return true;
}


//-------------------------------------------------------------------------------

void WindowViewPort::dependencies( DependentArray & dep )
{
	NodeWindowPort::dependencies( dep );

	// TODO: push all ports this node is dependent on, for example:
	// dep.push("L:\\Projects\\Seeker\\Image1.Prt");
}

CFrameWnd * WindowViewPort::createView()
{
	NodeWindowPort::createView();

	static CMultiDocTemplate docTemplate( IDR_PORT, NULL, 
		RUNTIME_CLASS(CChildFrame), RUNTIME_CLASS(CPortView) );

    CChildFrame *pFrame = dynamic_cast<CChildFrame *>( docTemplate.CreateNewFrame( this, NULL ) );
	ASSERT( pFrame );

	docTemplate.InitialUpdateFrame(pFrame, this);

	return( pFrame );
}

BaseNode * WindowViewPort::createNode()
{
	return( NodeWindowPort::createNode() );
}

void WindowViewPort::initializeNode( BaseNode * pNode )
{
	NodeWindowPort::initializeNode( pNode );

	WindowView * pView = dynamic_cast<WindowView *>( pNode );
	ASSERT( pView );

	pView->setViewClass( m_ViewClass );

	// check for naming conflicts with the child nodes, send a report if any are found
	Tree< CharString, int > names;
	checkNames( this, names );
}

void WindowViewPort::checkNames( BaseNodePort * pNode, Tree< CharString, int > & names )
{
	if ( dynamic_cast<WindowViewPort *>( pNode ) && pNode != this )
		return;		// do not recurse into other view nodes

	if ( names.find( pNode->name() ).valid() )
	{
		CharString sNewName;

		int i = 1;
		do {
			sNewName.format( "%s%d", pNode->name(), i++ );
		} while( names.find( sNewName ).valid() );

		report( CharString().format("Node '%s' name conflict, renamed to '%s'.", pNode->name(), sNewName.cstr() ) );
		pNode->setName( sNewName );

		// flag our scene so it will be re saved..
		((ScenePort *)m_Parent)->m_bKeyConflicts = true;
	}

	names[ pNode->name() ] = 1;

	// recurse into child nodes
	for(int i=0;i<pNode->childCount();i++)
		checkNames( pNode->child(i), names );
}

//-------------------------------------------------------------------------------
// EOF

