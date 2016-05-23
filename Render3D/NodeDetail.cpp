/*
	NodeDetail.cpp

	This node renders only one child based on the distance from the viewer,
	the distance from the viewer is divided by the nodes 'distance', the 
	integer of that result is the child that is rendered.

	(c)2005 Palestar, Richard Lyle
*/

#define RENDER3D_DLL
#include "Debug/Assert.h"
#include "Standard/Limits.h"
#include "Render3D/NodeDetail.h"

//-------------------------------------------------------------------------------

IMPLEMENT_FACTORY( NodeDetail, BaseNode );
REGISTER_FACTORY_KEY(  NodeDetail, 4038759653253627122LL );

BEGIN_PROPERTY_LIST( NodeDetail, BaseNode )
	ADD_PROPERTY( m_Distance )
END_PROPERTY_LIST()

//----------------------------------------------------------------------------

NodeDetail::NodeDetail()
{
	m_Distance = 500.0f;
}

//----------------------------------------------------------------------------

void NodeDetail::preRender( RenderContext &context, 
	const Matrix33 & frame, const Vector3 & position )
{
	if ( m_Children.size() > 0 )
	{
		Vector3 positionVS( context.worldToView( position ) );
		float fDistance = positionVS.magnitude() * (1.0f - context.detail());

		int nDetailLevel = Clamp<int>( fDistance / m_Distance, 0, m_Children.size() - 1 );
		m_Children[ nDetailLevel ]->preRender( context, frame, position );
	}
}

//----------------------------------------------------------------------------

void NodeDetail::setDistance( float distance )
{
	m_Distance = distance;
}

//-------------------------------------------------------------------------------
// EOF
