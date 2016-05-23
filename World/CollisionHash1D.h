/*
	CollisionHash1D.h

	Concrete implementation of the CollisionHashAbstract object using a 1D grid
	(c)2004 Palestar Inc, Richard Lyle
*/

#ifndef COLLISIONHASH1D_H
#define COLLISIONHASH1D_H

#include "CollisionHashAbstract.h"
#include "Math/Plane.h"

#include <vector>

//----------------------------------------------------------------------------

class CollisionHash1D : public CollisionHashAbstract
{
public:
	// Construction
	CollisionHash1D();
	~CollisionHash1D();

	//! CollisionHashAbstract interface
	virtual bool		query( const Vector3 & vPosition, float fRadius, 
							Array< NounCollision > & nodes, const ClassKey & nType ) const;

	virtual void		initialize( const BoxHull & hull );						// initialize the hash with the specified hull
	virtual void		update();												// updates the hash, queries all objects for their current position and radius
	virtual void		release();												// release the hash

	virtual bool		insert( Noun * pNoun );									// insert a collision node into the hash
	virtual bool		remove( Noun * pNoun );									// remove a collision node from the hash
	virtual void		flush();												// flush the hash, removes inserted collision nodes

	void				setCellCount( int a_nCount );							// call this before initialize to change the cell count

private:
	// Types
	struct Node : public Referenced
	{
		Node() : m_pNoun( NULL ), m_nTestID( 0 )
		{}

		typedef Reference< Node >		Ref;

		Ref				m_pNext;
		Noun *			m_pNoun;
		unsigned int	m_nTestID;
	};

	struct Cell : public Referenced
	{
		typedef Reference< Cell >		Ref;

		Ref				m_pNext;
		Node::Ref		m_pNode;
	};
	typedef std::vector< Cell::Ref >		CellVector;

	// Data
	int					m_nCellCount;
	float				m_fCenter;
	float				m_fCellSize;
	float				m_fCellSizeInv;			// = 1.0f / m_fCellSize
	CellVector			m_Cells;
	unsigned int		m_nTestID;				// current test ID
	Node::Ref			m_pNodes;				// linked list of all nodes currently in the hash
};

//---------------------------------------------------------------------------------------------------

inline void CollisionHash1D::setCellCount( int a_nCount )
{
	m_nCellCount = a_nCount;
}

//---------------------------------------------------------------------------------------------------

#endif

//----------------------------------------------------------------------------
//EOF
