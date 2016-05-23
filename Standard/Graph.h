/*
	Graph.h

	A Directed Graph template class
	(c)2005 Palestar Inc, Richard Lyle
*/

#ifndef GRAPH_H
#define GRAPH_H

#include "Array.h"
#include "Queue.h"
#include "MedusaDll.h"

//-------------------------------------------------------------------------------

template<typename T>
class Graph
{
public:
	// Types
	struct Node
	{
		Node() : nCost( 1 )
		{}
		Node( const T & object );
		Node( const Node & copy );

		T					Object;					// the contained object
		int					nCost;					// cost for this node

		Array<int>			Links;					// links to other nodes
		Array<int>			InLinks;				// links into this node
	};

	// Accessors
	int				nodeCount() const;
	const Node &	node(int n) const;
	
	// A* path find from starting node to ending node with the least cost, returns the cost of the path, -1 if no path was found
	int				findPath( int nStart, int nEnd, Array< int > & path ) const;

	// Mutators
	void			validate( int n );				// reallocs to make node n valid
	void			realloc( int n );				// realloc the nodes, keeps existing contents
	void			release();

	int				addNode( const T & copy );
	void			setNode( int n, const T & copy );
	void			setCost( int n, int nCost );
	void			removeNode( int n );

	void			addLink( int n, int k );		// add link from node n to node k
	void			removeLink( int n, int k );		// remove link from node n to node k

private:
	// Data
	Array<Node>		m_Nodes;					// array of nodes
};

//----------------------------------------------------------------------------

template<typename T>
Graph<T>::Node::Node( const T &object ) : Object( object ), nCost( 1 )
{}

template<typename T>
Graph<T>::Node::Node( const Node &copy ) 
	: Object( copy.Object ), nCost( copy.nCost ), Links( copy.Links ), InLinks( copy.InLinks )
{}

//-------------------------------------------------------------------------------

template<typename T>
inline int Graph<T>::nodeCount() const
{
	return m_Nodes.size();
}

template<typename T>
inline const typename Graph<T>::Node & Graph<T>::node(int n) const
{
	return m_Nodes[ n ];
}

template<typename T>
inline int Graph<T>::findPath( int nStart, int nEnd, Array< int > & path ) const
{
	if ( !m_Nodes.isValid( nStart ) || !m_Nodes.isValid( nEnd ) )
		return -1;			// invalid start or ending node
	if ( nStart == nEnd )
		return 0;			// already at the end

	// initialize the depth table
	Array< int > cost( nodeCount() );
	for(int i=0;i<cost.size();i++)
		cost[i] = -1;

	Queue< int > queue;
	// mark the end of the path
	cost[ nEnd ] = 0;
	// push the first point to check 
	queue.push( nEnd );

	// start check from the destination until we give up or reach the source poiint
	while( queue.valid() && cost[ nStart ] < 0 )
	{
		// get the head off the queue
		int nNode = *queue;
		queue.pop();

		// get the cost to this node
		int nCost = cost[ nNode ];

		const Node & node = m_Nodes[ nNode ];
		for(int i=0;i<node.InLinks.size();i++)
		{
			int nLink = node.InLinks[ i ];
			if ( nLink >= 0 && cost[ nLink ] < 0 )
			{
				// update the node cost
				cost[ nLink ] = nCost + m_Nodes[ nLink ].nCost;
				// push the link into the queue to be traversed
				queue.push( nLink );
			}
		}
	}

	// return if no path found from start to end...
	if ( cost[ nStart ] < 0 )
		return -1;

	// use the costs generated above, now actually find a path through the nodes from the start to the end, following
	// the least cost
	int nNext = nStart;
	while( nNext != nEnd )
	{
		int nBestCost = 32768;
		int nBestNode = -1;

		const Node & node = m_Nodes[ nNext ];
		for(int i=0;i<node.Links.size();i++)
		{
			int nLink = node.Links[i];
			if ( nLink < 0 )
				continue;

			int nCost = cost[ nLink ];
			if ( nCost >= 0 && nCost < nBestCost )
			{
				nBestCost = nCost;
				nBestNode = nLink;
			}
		}

		if ( nBestNode < 0 )
			return -1;		// failed to find a next node...

		// push the best node into the 
		path.push( nBestNode );
		nNext = nBestNode;
	}

	// return the costs of the path
	return path.size();
}

//----------------------------------------------------------------------------

template<typename T>
inline void Graph<T>::validate( int n )
{
	int nSize = n + 1;
	if ( m_Nodes.size() < nSize )
		m_Nodes.realloc( nSize );
}

template<typename T>
inline void Graph<T>::realloc( int n )
{
	m_Nodes.realloc( n );
}

template<typename T>
inline void Graph<T>::release()
{
	m_Nodes.release();
}

template<typename T>
inline int Graph<T>::addNode( const T & copy )
{
	m_Nodes.push( Node(copy) );
	return m_Nodes.size() - 1;
}

template<typename T>
inline void Graph<T>::setNode( int n, const T & copy )
{
	validate( n );
	m_Nodes[ n ].Object = copy;
}

template<typename T>
inline void Graph<T>::setCost( int n, int nCost )
{
	validate( n );
	m_Nodes[ n ].nCost = nCost;
}

template<typename T>
inline void Graph<T>::removeNode( int n )
{
	if ( m_Nodes.isValid( n ) )
	{	
		Node & node = m_Nodes[ n ];

		// remove all links from / into this node
		while( node.Links.size() > 0 )
			removeLink( n, node.Links[ 0 ] );
		while( node.InLinks.size() > 0 )
			removeLink( node.InLinks[ 0 ], n );
		m_Nodes.remove( n );
		
		// correct all other node links
		for(int k=0;k<m_Nodes.size();k++)
		{
			Node & node = m_Nodes[k];
			for(int j=0;j<node.Links.size();j++)
			{
				if ( node.Links[j] > n )
					node.Links[j]--;				// reduce the node index by 1
				else if ( node.Links[j] == n )
					node.Links[j] = -1;			// invalidate the link, node is gone..

				if ( node.InLinks[j] > n )
					node.InLinks[j]--;				// reduce the node index by 1
				else if ( node.InLinks[j] == n )
					node.InLinks[j] = -1;			// invalidate the link, node is gone..
			}
		}
	}
}

template<typename T>
inline void Graph<T>::addLink( int n, int k )
{
	if ( n != k )
	{
		validate( n );
		m_Nodes[ n ].Links.push( k );
		validate( k );
		m_Nodes[ k ].InLinks.push( n );
	}
}

template<typename T>
inline void Graph<T>::removeLink( int n, int k )
{
	if ( m_Nodes.isValid( n ) )
		m_Nodes[ n ].Links.removeSearch( k );
	if ( m_Nodes.isValid( k ) )
		m_Nodes[ k ].InLinks.removeSearch( n );
}

//-------------------------------------------------------------------------------



#endif

//-------------------------------------------------------------------------------
// EOF
