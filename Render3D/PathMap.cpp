/*
	PathMap.cpp

	This object is used to store a pathfinding map for a 3D scene of any type
	(c)2005 Palestar Inc, Richard Lyle
*/

#define RENDER3D_DLL
#include "Debug/Assert.h"
#include "Display/PrimitiveSetTransform.h"
#include "Display/PrimitiveLineList.h"
#include "Display/PrimitiveMaterial.h"
#include "Render3D/PathMap.h"

#include <limits.h>

//----------------------------------------------------------------------------

IMPLEMENT_FACTORY( PathMap, BaseNode );
REGISTER_FACTORY_KEY(  PathMap, 4553043809753496552LL );

BEGIN_PROPERTY_LIST( PathMap, BaseNode );
	ADD_PROPERTY( m_Nodes );
END_PROPERTY_LIST();

PathMap::PathMap()
{}

//----------------------------------------------------------------------------

void PathMap::render( RenderContext & context, const Matrix33 &frame, const Vector3 & position )
{
	BaseNode::render( context, frame, position );

	// render the nodes if this is the selected node
	if ( s_pSelectedNode == this )
	{
		DisplayDevice * pDisplay = context.display();
		ASSERT( pDisplay );

		// set a material
		PrimitiveMaterial::push( pDisplay, WHITE, PrimitiveMaterial::ALPHA );
		// set the transform
		PrimitiveSetTransform::push( pDisplay, Matrix33( frame ).normalize(), position );
		
		PrimitiveLineList::Ref lines = PrimitiveLineList::create( context.display(), m_Nodes.size() );

		Line * pLines = lines->lock();
		for(int i=0;i<m_Nodes.size();i++)
		{
			Node & node = m_Nodes[ i ];

			Vector3 n( node.position );
			n.normalize();

			pLines[i].v[0].normal = n;
			pLines[i].v[0].position = node.position;
			pLines[i].v[0].u = pLines[i].v[0].v = 0.0f;

			pLines[i].v[1].normal = n;
			pLines[i].v[1].position = node.position + n;
			pLines[i].v[1].u = pLines[i].v[1].v = 0.0f;
		}
		lines->unlock();

		pDisplay->push( lines );
	}
}

//----------------------------------------------------------------------------

int PathMap::addNode( const Node & add )
{
	flushPathCache();
	m_Nodes.push( add );
	return m_Nodes.size() - 1;	
}

void PathMap::setNode( const Node & node )
{
	flushPathCache();
	if ( m_Nodes.isValid( node.id ) )
		m_Nodes[ node.id ] = node;
}

void PathMap::deleteNode( int n ) 
{
	flushPathCache();
	if ( m_Nodes.isValid( n ) )
		m_Nodes.remove( n );
}

void PathMap::release()
{
	flushPathCache();
	m_Nodes.release();
}

void PathMap::flushPathCache()
{
	m_PathCache.release();
}

PathMap::Path * PathMap::findPath( int nStart, int nEnd )
{
	// make a 64-bit key from the starting node and ending node
	qword nPathKey = (((qword)nStart) << 32) | nEnd;
	PathCacheIt iPath = m_PathCache.find( nPathKey );
	if ( iPath.valid() )
		return *iPath;

	// make a new path
	Path * pPath = new Path( this, nStart, nEnd );
	m_PathCache[ nPathKey ] = pPath;

	return pPath;
}

//----------------------------------------------------------------------------

PathMap * PathMap::findMap( BaseNode * pNode )
{
	if ( WidgetCast<PathMap>( pNode ) )
		return (PathMap *)pNode;

	for(int i=0;i<pNode->childCount();i++)
	{
		PathMap * pMap = findMap( pNode->child(i) );
		if ( pMap != NULL )
			return pMap;
	}

	return NULL;
}

//----------------------------------------------------------------------------

PathMap::Path::Path()
{}

PathMap::Path::Path( PathMap * pMap, int nStart, int nEnd ) 
{
	find( pMap, nStart, nEnd );
}

PathMap::Path::Path( const Path & copy ) : Referenced(), m_Path( copy.m_Path )
{}

// implementation of A* for pathfinding
void PathMap::Path::find( PathMap * pMap, int nStart, int nEnd )
{
	ASSERT( pMap != NULL );
	ASSERT( nStart >= 0 && nStart < pMap->nodeCount() );
	ASSERT( nEnd >= 0 && nEnd < pMap->nodeCount() );

	// release any existing path
	m_Path.release();

	// initialize the depth table
	Array< int > depths( pMap->nodeCount() );
	for(int i=0;i<depths.size();i++)
		depths[i] = -1;

	Queue< int > queue;

	// mark the end of the path
	depths[ nEnd ] = 0;
	// push the first point to check 
	queue.push( nEnd );

	// start check from the destination until we give up or reach the source poiint
	while( queue.valid() && depths[ nStart ] < 0 )
	{
		// get the head off the queue
		int nNode = *queue;
		queue.pop();
		// get the depth of the nodes around this node
		int nDepth = depths[ nNode ] + 1;

		// check around this hex
		const PathMap::Node & node = pMap->node( nNode );
		for(int i=0;i<node.links.size();++i)
		{
			int nLink = node.links[ i ];
			if ( nLink >= 0 && depths[ nLink ] < 0 )
			{
				// update the hex depth
				depths[ nLink ] = nDepth;
				// push the hex into the queue to be checked
				queue.push( nLink );
			}
		}
	}

	// return if no path found from start to end...
	if ( depths[ nStart ] < 0 )
		return;

	// now construct the actual path from the depth table...
	int nNext = nStart;
	while( depths.isValid( nNext ) && depths[ nNext ] > 0 )
	{
		int nBestDepth = INT_MAX;
		int nBestNode = -1;

		const PathMap::Node & node = pMap->node( nNext );
		for(int i=0;i<node.links.size();++i)
		{
			int nLink = node.links[i];
			if ( nLink < 0 )
				continue;

			int nDepth = depths[ nLink ];
			if ( nDepth >= 0 && nDepth < nBestDepth )
			{
				nBestDepth = nDepth;
				nBestNode = node.links[i];
			}
		}

		m_Path.push( nBestNode );
		nNext = nBestNode;
	}
}

//----------------------------------------------------------------------------
//EOF
