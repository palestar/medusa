/*
	PathMap.h

	This object is used to hold path finding information for a 3D scene
	(c)2005 Palestar Inc, Richard Lyle
*/

#ifndef PATHMAP_H
#define PATHMAP_H

#include "Math/Vector3.h"
#include "Render3D/BaseNode.h"
#include "Render3D/Render3dDll.h"

//----------------------------------------------------------------------------

class DLL PathMap : public BaseNode
{
public:
	DECLARE_WIDGET_CLASS();
	DECLARE_PROPERTY_LIST();

	// Types
	typedef ResourceLink<PathMap>		Link;
	typedef Reference<PathMap>			Ref;
	typedef WidgetReference<PathMap>	wRef;

	struct Node
	{
		int			id;				// node id
		Vector3		position;		// position in object space
		Array< int >	
					links;			// index of linked nodes

		Node & operator=( const Node & copy );
	};

	class DLL Path : public Referenced		// path finding class
	{
	public:
		// Types
		typedef Reference< Path >		Ref;

		// Construction
		Path();
		Path( const Path & copy );
		Path( PathMap * pMap, int startNode, int endNode );

		// Accessors
		int					pathCount() const;											// number of nodes in the path
		int					path( int n ) const;										// get node index n

		// Mutators
		void				find( PathMap * pPlanet, int startNode, int endNode );		// do the path find

		// depreciated
		int					next();														// returns the next node along the path
		int					distance();													// returns the distance to the ending node

	private:
		// Data
		Array< int >		m_Path;
	};

	// Construction
	PathMap();

	// BaseNode interface
	void				render( RenderContext &context, 
							const Matrix33 & frame, const Vector3 & position );

	// Accessors
	int					nodeCount() const;
	const Node &		node( int n ) const;
	bool				isNodeValid( int n ) const;

	// Mutators
	int					addNode( const Node & add );	// add a new node
	void				setNode( const Node & node );	// modify an existing node
	void				deleteNode( int n );			// delete a node
	void				release();		
	
	void				flushPathCache();						// release all cached paths
	Path *				findPath( int nStart, int nEnd );		// find a path object from the start to the beginning

	// Helpers
	static PathMap *	findMap( BaseNode * pNode );	// search nodes for a PathMap object

protected:
	// Types
	typedef Hash< qword, Path::Ref >	PathCache;
	typedef PathCache::Iterator			PathCacheIt;

	// Data
	Array< Node >		m_Nodes;
	// non-serialized
	PathCache			m_PathCache;					
};

//----------------------------------------------------------------------------

inline int PathMap::nodeCount() const
{
	return m_Nodes.size();
}

inline const PathMap::Node & PathMap::node( int n ) const
{
	return m_Nodes[ n ];		// throws exception if n is out of range
}

inline bool PathMap::isNodeValid( int n ) const
{
	return m_Nodes.isValid( n );
}

//----------------------------------------------------------------------------

inline PathMap::Node & PathMap::Node::operator =( const Node & copy )
{
	id = copy.id;
	position = copy.position;
	links = copy.links;
	return *this;
}

inline const InStream & operator>>( const InStream & input, PathMap::Node & a_Node )
{
	input >> a_Node.id;
	input >> a_Node.position;
	input >> a_Node.links;
	return input;
}

inline const OutStream & operator<<( const OutStream & output, const PathMap::Node & a_Node )
{
	output << a_Node.id;
	output << a_Node.position;
	output << a_Node.links;
	return output;
}

//----------------------------------------------------------------------------

inline int PathMap::Path::pathCount() const
{
	return m_Path.size();
}

inline int PathMap::Path::path( int n ) const
{
	return m_Path[ n ];
}

inline int PathMap::Path::next()
{
	if ( m_Path.size() > 0 )
		return m_Path[ 0 ];

	return -1;		// no path to the destination
}

inline int PathMap::Path::distance()
{
	return m_Path.size();
}

//----------------------------------------------------------------------------




#endif

//----------------------------------------------------------------------------
//EOF
