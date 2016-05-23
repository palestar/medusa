/*
	NodeDecal.cpp
	(c)2005 Palestar Inc, Richard Lyle
*/

#define RENDER3D_DLL
#include "Render3D/NodeDecal.h"

//----------------------------------------------------------------------------

IMPLEMENT_FACTORY( NodeDecal, NodeTransform );
REGISTER_FACTORY_KEY(  NodeDecal, 4419192510756757150LL );

BEGIN_PROPERTY_LIST( NodeDecal, NodeTransform )
	ADD_PROPERTY( m_Material )
END_PROPERTY_LIST()

//----------------------------------------------------------------------------

NodeDecal::NodeDecal()
{
	m_Scale = 1.0f;
}

//----------------------------------------------------------------------------

BoxHull NodeDecal::hull() const
{
	if ( m_Decal.valid() )
		return m_Decal->hull();

	return BoxHull( false );
}

void NodeDecal::preRender( RenderContext &context, const Matrix33 & f, const Vector3 & p )
{
	if (! m_Decal.valid() )
	{
		// build the decal
		m_Decal = new NodeComplexMesh2;

		// find our parent mesh
		BaseNode * pParent = parent();
		while( pParent != NULL )
		{
			if ( WidgetCast<NodeComplexMesh2>( pParent ) )
			{
				NodeComplexMesh2 * pMesh = (NodeComplexMesh2 *)pParent;

				Vector3 n( -frame().k );
				n.normalize();

				Vector3 t( frame().i );
				t.normalize();

				float width = frame().i.magnitude();
				float height = frame().j.magnitude();
				float depth = frame().k.magnitude();

				m_Decal->buildDecal( pMesh, position(), n, t, width, height, depth, m_Material );
				break;
			}

			pParent = pParent->parent();
		}
	}

	m_Decal->render( context, f, p );

	// call the base class
	NodeTransform::preRender( context, f, p );
}


void NodeDecal::setPosition( const Vector3 &position )
{
	NodeTransform::setPosition( position );
	invalidate();
}

void NodeDecal::setFrame( const Matrix33 &frame )
{
	NodeTransform::setFrame( frame );
	invalidate();
}

//----------------------------------------------------------------------------

Material * NodeDecal::material() const
{
	return m_Material;
}

//----------------------------------------------------------------------------

void NodeDecal::setMaterial( Material * pMaterial )
{
	m_Material = pMaterial;
}

void NodeDecal::invalidate()
{
	m_Decal = NULL;
}

//----------------------------------------------------------------------------
//EOF
