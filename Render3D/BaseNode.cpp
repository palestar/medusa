/*
	BaseNode.cpp
	(c)2005 Palestar, Richard Lyle
*/

#pragma warning( disable: 4786 )	// identifier was truncated to '255' characters in the browser information

#define RENDER3D_DLL
#include "Render3D/BaseNode.h"
#include "Debug/Assert.h"
#include "Debug/Trace.h"
#include "Standard/Exception.h"
#include "Standard/STLHelper.h"

#include <algorithm>

//----------------------------------------------------------------------------

IMPLEMENT_RESOURCE_FACTORY( BaseNode, Resource );

BEGIN_PROPERTY_LIST( BaseNode, Resource )
	ADD_PROPERTY( m_Name );
	ADD_BITS_PROPERTY( m_nNodeFlags );
		ADD_BIT_OPTION( m_nNodeFlags, NF_NOBOUNDS );
		ADD_BIT_OPTION( m_nNodeFlags, NF_NOSHADOW );
	ADD_OBJECT_PROPERTY( m_Children );
END_PROPERTY_LIST();

bool		BaseNode::s_ShowSelected = false;			// draw box around selected node
BaseNode *	BaseNode::s_pSelectedNode = NULL;

//-------------------------------------------------------------------------------

BaseNode::BaseNode() : m_pParent( NULL ), m_nNodeFlags( 0 )
{}

BaseNode::BaseNode( const BaseNode &copy ) : 
	m_Name( copy.m_Name ),
	m_nNodeFlags( copy.m_nNodeFlags ),
	m_Children( copy.m_Children ),
	m_pParent( copy.m_pParent )
{}

BaseNode::BaseNode( BaseNode * pParent ) : m_pParent( NULL ), m_nNodeFlags( 0 )
{
	pParent->attachNode( this );
}

BaseNode::~BaseNode()
{
	// set the parent pointers to NULL
	for(size_t i=0;i<m_Children.size();i++)
	{
		if ( m_Children[i].valid() )
			m_Children[i]->m_pParent = NULL;
	}
}

//----------------------------------------------------------------------------

bool BaseNode::read( const InStream & input )
{
	if (! Resource::read( input ) )
		return false;

	// set the parent for all the children nodes and remove any NULL children..
	for(size_t i=0;i<m_Children.size();++i)
	{
		BaseNode * pChild = m_Children[i];
		if (! pChild )
			continue;

		// This can happen when the port is copied using the file system rather than Resourcer, which can
		// result in duplicate unique ID's
		if ( pChild->m_pParent != NULL && pChild->m_pParent != this )
		{
			LOG_ERROR( "BaseNode", "Child %u has wrong parent pointer in node %s", i, m_Name.cstr() );
			return false;
		}

		pChild->m_pParent = this;
	}

	return true;
}

//----------------------------------------------------------------------------

BoxHull BaseNode::hull() const
{
	return( BoxHull( Vector3(0,0,0), Vector3(0,0,0) ) );
}

float BaseNode::radius() const
{
	// use the hull, so nobody has to implement both functions
	return( hull().radius() );	
}

//----------------------------------------------------------------------------

int BaseNode::attachNode( BaseNode * pChild )
{
	if ( this == NULL || pChild == NULL )
		return -1;
	if ( pChild->isChild( this ) )		// assert that this isn't a circular attachment
		return -1;

	// this checks if this node is already attached to this node, if so just pretend we are attaching..
	if ( this == pChild->m_pParent )
	{
		for(size_t i=0;i<m_Children.size();++i)
			if ( m_Children[i] == pChild )
			{
				// notify ourselves
				onAttach( pChild );
				// notify the child object
				pChild->onAttached();

				return (int)i;
			}
		return -1;
	}

	// grab a reference to the child, otherwise when we detach from the old parent the child
	// may get deleted.
	Ref rChild( pChild );
	if ( pChild->m_pParent != NULL )
		pChild->m_pParent->detachNode( pChild );

	// update the internal version number of the parent object, this is important for 
	// serialization reasons.
	updateVersion();

	pChild->m_pParent = this;
	m_Children.push_back( pChild );

	// notify ourselves
	onAttach( pChild );
	// notify the child object
	pChild->onAttached();

	return( m_Children.size() - 1 );
}

int	BaseNode::insertNode( int n, BaseNode * pChild )
{
	if (! pChild )
		return -1;
	if ( this == NULL || pChild == NULL )
		return -1;
	if ( pChild->isChild( this ) )		// assert that this isn't a circular attachment
		return -1;

	// grab a reference to the child, otherwise when we detach from the old parent the child
	// may get deleted.
	Ref rChild( pChild );
	if ( pChild->m_pParent != NULL )
		pChild->m_pParent->detachNode( pChild );

	updateVersion();

	pChild->m_pParent = this;
	m_Children.insert( m_Children.begin() + n, pChild );

	// notify ourselves
	onAttach( pChild );
	// notify the child object
	pChild->onAttached();

	return n;
}

void BaseNode::detachNode( int child )
{
	ASSERT( this != NULL );

	if ( child >= 0 && child < (int)m_Children.size() )
	{
		updateVersion();

		BaseNode * pChild = m_Children[ child ];
		if ( pChild != NULL )
		{
			// inform the child it is being detached
			pChild->onDetaching();
			// inform ourselves
			onDetach( pChild );
			// null the parent pointer and remove from the child list
			pChild->m_pParent = NULL;
		}

		m_Children.erase( m_Children.begin() + child );
		// notify this object that a child object has been detached
		onChildDetached();
	}
}

void BaseNode::detachNode( BaseNode * pChild )
{
	if ( pChild != NULL )
	{
		for(size_t i=0;i<m_Children.size();i++)
			if ( m_Children[i].pointer() == pChild )
			{
				detachNode( i );
				break;
			}
	}
}

void BaseNode::detachNodeSwap( int nChild )
{
	if ( nChild >= 0 && nChild < (int)m_Children.size() )
	{
		updateVersion();

		BaseNode * pChild = m_Children[ nChild ];
		if ( pChild != NULL )
		{
			// inform the child it is being detached
			pChild->onDetaching();
			// inform ourselves
			onDetach( pChild );
			// null the parent pointer and remove from the child list
			pChild->m_pParent = NULL;
		}

		removeSwapIndex( m_Children, nChild );
		// notify this object that a child object has been detached
		onChildDetached();
	}
}

void BaseNode::detachNodeSwap( BaseNode * pChild )
{
	for(size_t i=0;i<m_Children.size();i++)
		if ( m_Children[i].pointer() == pChild )
		{
			detachNodeSwap( i );
			break;
		}
}

void BaseNode::detachAllNodes()
{
	updateVersion();
	for(size_t i=0;i<m_Children.size();i++)
	{
		BaseNode * pChild = m_Children[ i ];
		if (! pChild )
			continue;

		// inform the child it is being detached
		pChild->onDetaching();
		// inform ourselves
		onDetach( pChild );
		// set the parent pointer to NULL
		pChild->m_pParent = NULL;
	}

	m_Children.clear();

	// notify this object that a child object has been detached
	onChildDetached();
}

void BaseNode::detachSelf()
{
	if ( m_pParent.valid() )
		m_pParent->detachNode( this );
}

void BaseNode::onAttach( BaseNode * pChild )
{}

void BaseNode::onAttached()
{}

void BaseNode::onDetach( BaseNode * pChild )
{}

void BaseNode::onChildDetached()
{}

void BaseNode::onDetaching()
{}

void BaseNode::render( RenderContext &context, const Matrix33 & frame, const Vector3 & position )
{}

void BaseNode::preRender( RenderContext &context, const Matrix33 & frame, const Vector3 & position )
{
	context.setInstanceKey( context.instanceKey() + key() );

	if (! context.isShadowPass() || (m_nNodeFlags & NF_NOSHADOW) == 0 )
		render( context, frame, position );

	// recurse into the child nodes
	for(size_t i=0;i<m_Children.size();i++)
		m_Children[i]->preRender( context, frame, position );

	context.setInstanceKey( context.instanceKey() - key() );
}

bool BaseNode::intersect( const Matrix33 &frame, const Vector3 & position,
	IntersectContext & context )
{
	// rotate and transform the bounding box, it is still axis-aligned
	BoxHull tHull( hull(), frame, position );
	// test for intersection with the bounding box
	return ( tHull.intersect( context.origin, context.ray, context.intersect ) );
}

void BaseNode::sortNodes()
{
	// sort the child nodes by name
	updateVersion();

	std::sort( m_Children.begin(), m_Children.end(), sorter );
}

//-------------------------------------------------------------------------------

bool BaseNode::isChild( const BaseNode * pNode )
{
	if ( pNode == this )
		return( true );

	for(size_t i=0;i<m_Children.size();i++)
		if ( m_Children[i]->isChild( pNode ) )
			return true;

	return false;
}

BaseNode * BaseNode::findNode( const char * name )
{
	if ( strcmp<char>( m_Name, name) == 0 )
		return( this );

	BaseNode * found = NULL;
	for(size_t i=0;i<m_Children.size();i++)
		if ( (found = m_Children[i]->findNode( name )) != NULL )
			return( found );

	return( NULL );
}

BaseNode * BaseNode::findNode( const WidgetKey & key )
{
	if ( m_Key == key )
		return( this );

	BaseNode * found = NULL;
	for(size_t i=0;i<m_Children.size();i++)
		if ( (found = m_Children[i]->findNode( key )) != NULL )
			return( found );

	return( NULL );
}

//----------------------------------------------------------------------------

bool BaseNode::sorter( Ref p1, Ref p2 )
{
	return p1->m_Name.compareNoCase( p2->m_Name ) > 0;
}

//----------------------------------------------------------------------------
// EOF
