/*
	Scene.cpp
	(c)2005 Palestar, Richard Lyle
*/

#define RENDER3D_DLL
#include "Debug/Assert.h"
#include "Debug/Trace.h"
#include "Render3D/Scene.h"

#include "Standard/StringHash.h"

//----------------------------------------------------------------------------

IMPLEMENT_RESOURCE_FACTORY( Scene, Resource );

BEGIN_PROPERTY_LIST( Scene, Resource );
	ADD_PROPERTY( m_Node );
	ADD_PROPERTY( m_Fps );
	ADD_PROPERTY( m_Segments );
	ADD_PROPERTY( m_DefaultCameraPosition );
	ADD_PROPERTY( m_DefaultCameraFrame );
END_PROPERTY_LIST();

Scene * Scene::s_pCurrentScene = NULL;

//-------------------------------------------------------------------------------

Scene::Scene()
{
	m_Node = new NodeTransform;
	m_Fps = 15.0f;
}

Scene::Scene( BaseNode * pNode )
{
	m_Node = pNode;
	m_Fps = 15.0f;
}

//----------------------------------------------------------------------------

BaseNode * Scene::node() const
{
	return( m_Node );
}

float Scene::fps() const
{
	return( m_Fps );
}

int Scene::findSegment( const char * pName ) const
{
	StringHash hashId( pName );

	// TODO: use a hash table for quicker lookup
	for(int i=0;i<m_Segments.size();i++)
		if ( m_Segments[i].hashId == hashId )
			return i;

	return -1;
}

int	Scene::segmentCount() const
{
	return( m_Segments.size() );
}

const Scene::Segment & Scene::segment( int n ) const
{
	return( m_Segments[ n ] );
}

Matrix33 Scene::defaultCameraFrame() const
{
	return( m_DefaultCameraFrame );
}

Vector3 Scene::defaultCameraPosition() const
{
	return( m_DefaultCameraPosition );
}

BoxHull Scene::calculateHull() const
{
	// enumerate through the nodes in the scene, building a bounding box
	BoxHull hull( false );
	enumBoundingBox( Matrix33( true ), Vector3( 0.0f ), m_Node, hull );

	return( hull );
}

//----------------------------------------------------------------------------

void Scene::setNode( BaseNode * pNode )
{
	m_Node = pNode;
}

void Scene::setFps( float fps )
{
	m_Fps = fps;
}

int Scene::addSegment( const Segment &segment )
{
	m_Segments.push( segment );
	return( segmentCount() - 1 );
}

void Scene::removeSegment( int n )
{
	m_Segments.remove( n );
}

void Scene::setDefaultCamera( const Matrix33 & frame, const Vector3 & position )
{
	m_DefaultCameraFrame = frame;
	m_DefaultCameraPosition = position;
}

bool Scene::intersect( const Matrix33 &frame, const Vector3 &position, IntersectContext &context )
{
	context.intersectNode = NULL;
	context.intersectDistance2 = 32768.0f;

	enumIntersect( m_Node, frame, position, context );

	return( context.intersectNode != NULL );
}

//-------------------------------------------------------------------------------

void Scene::render( RenderContext &context, const Matrix33 & frame, const Vector3 & position )
{
	// set this are active scene
	Scene * previousScene = s_pCurrentScene;
	s_pCurrentScene = this;

	m_Node->preRender( context, frame, position );

	// restore last scene
	s_pCurrentScene = previousScene;
}

//-------------------------------------------------------------------------------

void Scene::enumBoundingBox( const Matrix33 &rotation, 
				const Vector3 &translation, 
				BaseNode *pBaseNode, 
				BoxHull & hull ) const
{
	Vector3 tPosition( translation );
	Matrix33 tFrame( rotation );

	BoxHull nodeHull( pBaseNode->hull() );
	//pBaseNode->boxHull( nodeHull );

	NodeTransform * pNode = WidgetCast<NodeTransform>( pBaseNode );
	if ( pNode != NULL )
	{
		Matrix33 tRotation( tFrame );
		tRotation.transpose();

		tPosition = ( tRotation * pNode->position() ) + tPosition;
		tFrame = tRotation * pNode->frame();
	}

	if ( (pBaseNode->nodeFlags() & BaseNode::NF_NOBOUNDS) == 0 )
		hull.addBox( BoxHull( nodeHull, tFrame, tPosition ) );

	for(int i=0;i<pBaseNode->childCount();i++)
		enumBoundingBox( tFrame, tPosition, pBaseNode->child(i), hull ); 
}

void Scene::enumIntersect( BaseNode *pBaseNode, const Matrix33 &rotation, const Vector3 &translation,
				IntersectContext & context ) const
{
	Matrix33 tFrame( rotation );
	Vector3 tPosition( translation );

	NodeTransform * pNode = WidgetCast<NodeTransform>( pBaseNode );
	if ( pNode != NULL )
	{
		Matrix33 tRotation( ~tFrame );
		tPosition = ( tRotation * pNode->position() ) + tPosition;
		tFrame = tRotation * pNode->frame();
	}

	BaseNode::IntersectContext nodeContext;
	nodeContext.origin = context.origin;
	nodeContext.ray = context.ray;

	if ( pBaseNode->intersect( tFrame, tPosition, nodeContext ) )
	{
		float distance2 = (context.origin - nodeContext.intersect).magnitude();
		if ( distance2 < context.intersectDistance2 )
		{
			context.intersect = nodeContext.intersect;
			context.intersectDistance2 = distance2;
			context.intersectNode = pBaseNode;
		}
	}

	for(int i=0;i<pBaseNode->childCount();i++)
		enumIntersect( pBaseNode->child(i), tFrame, tPosition, context ); 
}

//----------------------------------------------------------------------------
// EOF
