/*
	NodeScene.cpp
	(c)2005 Palestar, Richard Lyle
*/

#define RENDER3D_DLL
#include "Debug/Assert.h"
#include "Render3D/NodeScene.h"
#include "Render3D/Scene.h"

//----------------------------------------------------------------------------

IMPLEMENT_FACTORY( NodeScene, NodeTransform );
REGISTER_FACTORY_KEY(  NodeScene, 3924477385498890010LL );

BEGIN_PROPERTY_LIST( NodeScene, NodeTransform )
	ADD_PROPERTY( m_pScene );
	ADD_PROPERTY( m_bNormalizeFrame );
END_PROPERTY_LIST();

//----------------------------------------------------------------------------

NodeScene::NodeScene() 
{
	m_bNormalizeFrame = true;
}

NodeScene::NodeScene( Scene * pScene, bool bNormalize /*= true*/ )
{
	m_pScene = pScene;
	m_bNormalizeFrame = bNormalize;
}

NodeScene::~NodeScene()
{
	setScene( NULL );
}

//-------------------------------------------------------------------------------

BoxHull NodeScene::hull() const
{
	if ( m_pScene.valid() )
		return m_pScene->calculateHull();

	return( BoxHull( Vector3(1,1,1), Vector3(-1,-1,-1) ) );
}

void NodeScene::render(  RenderContext &context, const Matrix33 & frame, const Vector3 & position )
{
	NodeTransform::render( context, frame, position );

	if ( m_pScene.valid( true, false ) )
	{
		// render the scene
		if ( m_bNormalizeFrame )
		{
			Matrix33 normalizedFrame( frame );
			normalizedFrame.orthoNormalizeXY();

			m_pScene->render( context, normalizedFrame, position );
		}
		else
			m_pScene->render( context, frame, position );
	}
}

//----------------------------------------------------------------------------

Scene * NodeScene::scene() const
{
	return m_pScene;
}

//-------------------------------------------------------------------------------

void NodeScene::setScene( Scene * pScene, bool bNormalize /*= true*/ )
{
	m_pScene = pScene;
	m_bNormalizeFrame = bNormalize;
}

//----------------------------------------------------------------------------
// EOF

