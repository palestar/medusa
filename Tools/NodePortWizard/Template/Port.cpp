/*
	Port.cpp
	(c)2005 Palestar, Richard Lyle
*/

#define $$ROOT$$_DLL
#include "stdafx.h"

#include "Port.h"
#include "PortView.h"
#include "Tools/ScenePort/ChildFrame.h"

//----------------------------------------------------------------------------

IMPLEMENT_FACTORY( $$root$$, BaseNodePort );

BEGIN_PROPERTY_LIST( $$root$$, BaseNodePort );
END_PROPERTY_LIST();

$$root$$::$$root$$() : BaseNodePort()
{}

//-------------------------------------------------------------------------------

void $$root$$::dependencies( DependentArray & dep )
{
	// TODO: push all ports this node is dependent on, for example:
	// dep.push("L:\\Projects\\Seeker\\Image1.Prt");
}

CFrameWnd * $$root$$::createView()
{
	BaseNodePort::createView();

	static CMultiDocTemplate docTemplate( IDR_PORT, NULL, 
		RUNTIME_CLASS(CChildFrame), RUNTIME_CLASS(CPortView) );

    CChildFrame *pFrame = dynamic_cast<CChildFrame *>( docTemplate.CreateNewFrame( this, NULL ) );
	ASSERT( pFrame );

	docTemplate.InitialUpdateFrame(pFrame, this);

	return( pFrame );
}

BaseNode * $$root$$::createNode()
{
	// TODO: Create your node type here
	BaseNode * pNode = new BaseNode;	
	initializeNode( pNode );

	return( pNode );
}

void $$root$$::initializeNode( BaseNode * pNode )
{
	// TODO: initialize your node type
	BaseNodePort::initializeNode( pNode );
}


//-------------------------------------------------------------------------------
// EOF

