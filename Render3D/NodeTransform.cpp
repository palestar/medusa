/*
	NodeTransform.cpp
	(c)2005 Palestar, Richard Lyle
*/

#define RENDER3D_DLL

#pragma warning( disable: 4786 )	// identifier was truncated to '255' characters in the browser information

#include "Math/Quat.h"
#include "Render3D/NodeTransform.h"
#include "Render3D/Material.h"
#include "Display/PrimitiveLineList.h"
#include "Display/PrimitiveSetTransform.h"

//----------------------------------------------------------------------------

bool NodeTransform::s_bDrawNodeAxis = false;			// draw node axis
bool NodeTransform::s_ShowHull = false;				// show rotated hull around nodes
bool NodeTransform::s_ShowAlignedHull = false;		// show axis-aligned hull around nodes

//----------------------------------------------------------------------------

IMPLEMENT_FACTORY( NodeTransform, BaseNode );
REGISTER_FACTORY_KEY(  NodeTransform, 3924294499673922376LL );
REGISTER_FACTORY_ALIAS( NodeTransform, Node );		// register our old name so data continues to load..

BEGIN_PROPERTY_LIST( NodeTransform, BaseNode )
	ADD_PROPERTY( m_Position );
	ADD_PROPERTY( m_Frame );
END_PROPERTY_LIST();

//-------------------------------------------------------------------------------

NodeTransform::NodeTransform() : m_Position( true ), m_Frame( true )
{}

NodeTransform::NodeTransform( const NodeTransform &copy ) : BaseNode( copy )
{
	m_Position = copy.m_Position;
	m_Frame = copy.m_Frame;
}

NodeTransform::NodeTransform( const Matrix33 &frame, const Vector3 &position )
{
	m_Frame = frame;
	m_Position = position;
}

NodeTransform::NodeTransform( NodeTransform * pParent, const Matrix33 &frame, const Vector3 &position ) : BaseNode( pParent )
{
	m_Frame = frame;
	m_Position = position;

	pParent->attachNode( this );
}

//----------------------------------------------------------------------------

void NodeTransform::render( RenderContext &context, const Matrix33 & frame, const Vector3 & position )
{
	if ( (s_ShowSelected && s_pSelectedNode == this) || s_bDrawNodeAxis )
	{
		const Line NODE_AXIS_LINES[] = 
		{
			Line( Vertex( Vector3(0,0,0), Vector3(1,0,0), 0, 0 ), 
				Vertex( Vector3(1,0,0), Vector3(1,0,0), 0, 0) ),	// X
			Line( Vertex( Vector3(0,0,0), Vector3(1,0,0), 0, 0 ), 
				Vertex( Vector3(0,1,0), Vector3(1,0,0), 0, 0) ),	// Y
			Line( Vertex( Vector3(0,0,0), Vector3(1,0,0), 0, 0 ), 
				Vertex( Vector3(0,0,1), Vector3(1,0,0), 0, 0) )	// Z
		};

		// push a bright material
		context.push( Material( WHITE ).material( context ) );
		context.pushTransform( frame, position );
		context.push( PrimitiveLineList::create( context.device(), 3, NODE_AXIS_LINES ) );
	}

	if ( (s_ShowSelected && s_pSelectedNode == this) || s_ShowHull )
	{
		// render the object-orientated box hull
		BoxHull oob( hull() );
		context.pushTransform( frame, position );
		context.renderBox( oob, Color(255,0,0,255) );
	}

	if ( s_ShowAlignedHull )
	{
		// get the hull for this object
		BoxHull	nodeHull( hull() );
		// render the axis-aligned box hull
		BoxHull	alignedHull( nodeHull, frame, position );
		
		context.pushIdentity();
		context.renderBox( alignedHull, Color(255,255,255,255) );
	}
}

void NodeTransform::preRender( RenderContext &context, const Matrix33 & frame, const Vector3 & position )
{
	BaseNode::preRender( context, frame % m_Frame, (frame % m_Position) + position );
}

//----------------------------------------------------------------------------

Vector3 NodeTransform::worldPosition( BaseNode * pBaseNode )
{
	Vector3 worldPosition( true );

	while( pBaseNode != NULL )
	{
		NodeTransform * pNode = WidgetCast<NodeTransform>( pBaseNode );
		if ( pNode != NULL )
			worldPosition = (pNode->frame() % worldPosition) + pNode->position();

		pBaseNode = pBaseNode->parent();
	}

	return worldPosition;
}

Matrix33 NodeTransform::worldFrame( BaseNode * pBaseNode )
{
	Matrix33 worldFrame( true );

	while( pBaseNode != NULL )
	{
		NodeTransform * pNode = WidgetCast<NodeTransform>( pBaseNode );
		if ( pNode != NULL )
			worldFrame = pNode->frame() % worldFrame;

		pBaseNode = pBaseNode->parent();
	}

	return worldFrame;
}

BoxHull NodeTransform::worldHull( BaseNode * pBaseNode )
{
	Matrix33 frame( true );
	Vector3  position( true );
	BoxHull	 hull( pBaseNode->hull() );

	while( pBaseNode != NULL )
	{
		NodeTransform * pNode = WidgetCast<NodeTransform>( pBaseNode );
		if ( pNode != NULL )
		{
			position = (pNode->frame() % position) + pNode->position();
			frame = (pNode->frame() % frame);
		}

		pBaseNode = pBaseNode->parent();
	}

	// return AA box hull in world space
	return( BoxHull( hull, frame, position ) );
}

//----------------------------------------------------------------------------
// EOF
