/*
	BaseNode.h

	3d BaseNode
	(c)2005 Palestar, Richard Lyle
*/

#ifndef BASE_NODE_H
#define BASE_NODE_H

#include "Standard/Array.h"
#include "Standard/String.h"
#include "Standard/WeakReference.h"
#include "Math/Vector3.h"
#include "Math/Matrix33.h"
#include "Math/BoxHull.h"
#include "Resource/Resource.h"

#include "Material.h"
#include "RenderContext.h"

#include <vector>

#include "Render3dDll.h"

//-------------------------------------------------------------------------------

class DLL BaseNode : public Resource
{
public:
	DECLARE_WIDGET_CLASS();
	DECLARE_PROPERTY_LIST();

	// Types
	typedef Reference<BaseNode>			Ref;
	typedef WidgetReference<BaseNode>	wRef;
	typedef WeakReference<BaseNode>		WeakRef;

	typedef std::vector< Ref >			Children;

	struct IntersectContext 
	{
		Vector3			origin;					// ray origin
		Vector3			ray;					// ray direction
		Vector3			intersect;				// intersection point
	};

	enum Flags {
		NF_AMBIENT				= 0x1,			// if true, node is always rendered desipite bounds
		NF_NOBOUNDS				= 0x2,			// if true, no not includes this node in bounds calculations
		NF_NOSHADOW				= 0x4,			// if true, this node doesn't cast shadows
	};
	
	// Construction
	BaseNode();
	BaseNode( const BaseNode &copy );
	BaseNode( BaseNode * pParent );
	virtual	~BaseNode();

	// Widget Interface
	bool					read( const InStream & input );

	// Accessors
	const char *			name() const;							// node name
	dword					nodeFlags() const;						// get the node flags
	BaseNode *				parent() const;
	int						childCount() const;
	BaseNode *				child( int n ) const;

	virtual BoxHull			hull() const;							// BoxHull for this object
	virtual float			radius() const;							// radius for this object

	// Mutators
	void					setName( const char * name );
	void					setNodeFlags( dword a_nFlags );
	void					addNodeFlags( dword a_nFlags );
	void					clearNodeFlags( dword a_nFlags );

	void					setParent( BaseNode * pParent );		// set the nodes parent, normally set by attachNode()

	int						attachNode( BaseNode * pChild );		// returns the index of the inserted child node
	int						insertNode( int n, BaseNode * pChild );	// insert node before child n, returns the child index
	void					detachNode( int child );
	void					detachNode( BaseNode * pChild );
	void					detachNodeSwap( int nChild );		// detach the child node, swap it's position with the last node
	void					detachNodeSwap( BaseNode * pChild );
	void					detachAllNodes();
	virtual void			detachSelf();

	virtual void			onAttach( BaseNode * pChild );			// called when a child object is attached to this node
	virtual void			onAttached();							// called when this object is attached as a child
	virtual void			onDetach( BaseNode * pChild );			// called before the child object is detached
	virtual void			onChildDetached();						// called after a child object has been detached from this object
	virtual void			onDetaching();							// called before this object is detached from it's parent


	//! interface for rendering a object, frame & position are the world space position of the object.
	virtual void			render( RenderContext &context, 
								const Matrix33 & frame, const Vector3 & position );
	//! this function calls render(), this allows a node to handle the rendering of child objects.
	virtual void			preRender( RenderContext &context, 
								const Matrix33 & frame, const Vector3 & position );
	virtual bool			intersect( const Matrix33 &frame, const Vector3 & position,
								IntersectContext & context );

	void					sortNodes();							// sort child nodes by name

	// Helpers
	bool					isChild( const BaseNode * pNode );
	BaseNode *				findNode( const char * name );
	BaseNode *				findNode( const WidgetKey & key );

	// Static
	static bool				s_ShowSelected;			// draw box around selected node
	static BaseNode *		s_pSelectedNode;

protected:
	// Data
	CharString				m_Name;					// node name
	dword					m_nNodeFlags;			// node flags
	Children				m_Children;				// children node
	// non-serialized
	WeakRef					m_pParent;				// parent node

	// Static
	static bool				sorter( Ref p1, Ref p2 );
};

//----------------------------------------------------------------------------

inline const char * BaseNode::name() const
{
	return( m_Name );
}

inline dword BaseNode::nodeFlags() const
{
	return m_nNodeFlags;
}

inline BaseNode * BaseNode::parent() const
{
	return( m_pParent );
}

inline int BaseNode::childCount() const
{
	return (int)m_Children.size();
}

inline BaseNode * BaseNode::child( int n ) const
{
	return( m_Children[ n ] );
}

//-------------------------------------------------------------------------------

inline void BaseNode::setName( const char * name )
{
	m_Name = name;
}

inline void BaseNode::setNodeFlags( dword a_nFlags )
{
	m_nNodeFlags = a_nFlags;
}

inline void BaseNode::addNodeFlags( dword a_nFlags )
{
	m_nNodeFlags |= a_nFlags;
}

inline void BaseNode::clearNodeFlags( dword a_nFlags )
{
	m_nNodeFlags &= ~a_nFlags;
}

inline void BaseNode::setParent( BaseNode * pParent )
{
	m_pParent = pParent;
}

//----------------------------------------------------------------------------

//! Helper function to find a single node of a given type and return it..
template<class T>
inline T * FindNodeClass( BaseNode * pNode)
{
	if ( WidgetCast<T>( pNode ) )
		return (T *)pNode;

	for(int i=0;i<pNode->childCount();i++)
	{
		T * pFound = FindNodeClass<T>( pNode->child(i) );
		if ( pFound )
			return pFound;
	}

	return NULL;
}


//! This searched an entire heirarch of nodes, returning all found nodes in the given list..
template<class T>
inline void FindNodeClasses( BaseNode * pNode, std::list< T * > & a_Nodes )
{
	if ( WidgetCast<T>( pNode ) )
		a_Nodes.push_back( (T *)pNode );

	for(int i=0;i<pNode->childCount();i++)
		FindNodeClasses<T>( pNode->child(i), a_Nodes );
}

template<class T>
inline void FindNodeClasses( BaseNode * pNode, std::vector< T * > & a_Nodes )
{
	if ( WidgetCast<T>( pNode ) )
		a_Nodes.push_back( (T *)pNode );

	for(int i=0;i<pNode->childCount();i++)
		FindNodeClasses<T>( pNode->child(i), a_Nodes );
}

//-------------------------------------------------------------------------------



#endif

//-------------------------------------------------------------------------------
// EOF
