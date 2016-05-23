/*
	NodeConnector.cpp
	(c)2005 Palestar Inc, Richard Lyle
*/

#define GUI3D_DLL
#include "Display/PrimitiveLineStrip.h"
#include "Gui3d/NodeConnector.h"

//----------------------------------------------------------------------------

IMPLEMENT_FACTORY( NodeConnector, BaseNode );
REGISTER_FACTORY_KEY(  NodeConnector, 4543279781739306362 );

BEGIN_PROPERTY_LIST( NodeConnector, BaseNode );
	ADD_PROPERTY( m_Origin );
	ADD_PROPERTY( m_Connect );
	ADD_COLOR_PROPERTY( m_Color );
END_PROPERTY_LIST();

NodeConnector::NodeConnector()
{}

NodeConnector::NodeConnector( NodeWindow * pOrigin, NodeWindow * pConnect, Color color ) 
	: m_Origin( pOrigin ), m_Connect( pConnect ), m_Color( color )
{}

//----------------------------------------------------------------------------

void NodeConnector::render( RenderContext & context, const Matrix33 & frame, const Vector3 & position )
{
	BaseNode::render( context, frame, position );

	if ( m_Origin.valid() && m_Connect.valid() )
	{
		// get the rect for the origin
		RectInt originRect( m_Origin->screenWindow() );
		PointInt originCenter( originRect.center() );
		// get the rect for the destination
		RectInt connectRect( m_Connect->screenWindow() );
		PointInt connectCenter( connectRect.center() );

		PrimitiveMaterial::push( context.display(), PrimitiveMaterial::ALPHA, false );

		if ( originCenter.x < connectCenter.x )
		{
			// origin is to the left of the destination
			if ( originCenter.y < connectCenter.y )
				PrimitiveLineStripDTL::pushConnector( context.display(), originRect.lowerRight(), connectRect.upperLeft(), m_Color );
			else
				PrimitiveLineStripDTL::pushConnector( context.display(), originRect.upperRight(), connectRect.lowerLeft(), m_Color );
		}
		else	// originCenter.x >= connectCenter.x
		{
			// origin is to the right of the destination
			if ( originCenter.y < connectCenter.y )
				PrimitiveLineStripDTL::pushConnector( context.display(), originRect.lowerLeft(), connectRect.upperRight(), m_Color );
			else
				PrimitiveLineStripDTL::pushConnector( context.display(), originRect.upperLeft(), connectRect.lowerRight(), m_Color );
		}
	}

}

//----------------------------------------------------------------------------

NodeWindow * NodeConnector::origin() const
{
	return m_Origin;
}

NodeWindow * NodeConnector::connect() const
{
	return m_Connect;
}

Color NodeConnector::color() const
{
	return m_Color;
}

//----------------------------------------------------------------------------

void NodeConnector::setOrigin( NodeWindow * pOrigin )
{
	m_Origin = pOrigin;
}

void NodeConnector::setConnect( NodeWindow * pConnect )
{
	m_Connect = pConnect;
}

void NodeConnector::setColor( Color color )
{
	m_Color = color;
}

//----------------------------------------------------------------------------
//EOF
