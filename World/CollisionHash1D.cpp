/*
	CollisionHash1D.cpp
	(c)2004 Palestar Inc, Richard Lyle
*/


#include "Standard/Constant.h"

#include "Debug/Profile.h"
#include "CollisionHash1D.h"
#include "WorldContext.h"

//! space is sub-divided into this number of cells
const dword GRID_1D_CELL_COUNT	= 256;

//----------------------------------------------------------------------------

CollisionHash1D::CollisionHash1D() : 
	m_nCellCount( GRID_1D_CELL_COUNT ),
	m_fCenter( 0.0f ),
	m_fCellSize( 0.0f ),
	m_fCellSizeInv( 0.0f ),
	m_nTestID( 1 )
{}

CollisionHash1D::~CollisionHash1D()
{
	release();
}

//----------------------------------------------------------------------------

bool CollisionHash1D::query( const Vector3 & vPosition, float fRadius, Array< NounCollision > & nodes, const ClassKey & nType ) const
{
	if ( m_Cells.size() != m_nCellCount )
		return false;		// hash not initialized

	int nMinCell = (int)floorf( (vPosition.x - fRadius + m_fCenter) * m_fCellSizeInv );
	if ( nMinCell < 0 )
		nMinCell = 0;
	int nMaxCell = (int)ceilf( (vPosition.x + fRadius + m_fCenter) * m_fCellSizeInv );
	if ( nMaxCell > (m_nCellCount - 1) )
		nMaxCell = m_nCellCount - 1;

	const_cast<CollisionHash1D *>( this )->m_nTestID += 1;

	float fRadius2 = fRadius * fRadius;
	for(int i=nMinCell;i<=nMaxCell;++i)
	{
		CollisionHash1D::Cell * pCell = m_Cells[ i ];
		while( pCell != NULL )
		{
			CollisionHash1D::Node * pNode = pCell->m_pNode;
			pCell = pCell->m_pNext;

			if ( pNode == NULL || pNode->m_nTestID == m_nTestID )
				continue;
			Noun * pCheck = pNode->m_pNoun;
			if (! pCheck )
				continue;
			if ( nType != ClassKey((qword)0) && !pCheck->factory()->isType( nType ) )
				continue;

			pNode->m_nTestID = m_nTestID;

			float fDistance2 = (pCheck->lastPosition() - vPosition).magnitude2();
			if ( fDistance2 < (fRadius2 + pCheck->radius2()) )
			{
				NounCollision & node = nodes.push();
				node.pNoun = pCheck;
				node.fDistance = sqrtf( fDistance2 );
			}
		}
	}

	return nodes.size() > 0;
}

//----------------------------------------------------------------------------

void CollisionHash1D::initialize( const BoxHull & hull )
{
	// release the previous tree if any
	release();

	float fX = hull.lengthX();
	m_fCenter = fX / 2.0f;
	m_fCellSize = fX / m_nCellCount;
	m_fCellSizeInv = 1.0f / m_fCellSize;

	m_Cells.resize( m_nCellCount );
}

void CollisionHash1D::update()
{
	if ( m_Cells.size() != m_nCellCount )
		return;					// hash not initialized

	m_Cells.clear();
	m_Cells.resize( m_nCellCount );

	CollisionHash1D::Node * pNode = m_pNodes;
	while( pNode != NULL )
	{
		Noun * pNoun = pNode->m_pNoun;
		
		float fRadius = pNoun->radius();
		const Vector3 & vPosition = pNoun->lastPosition();

		int nMinCell = (int)floorf( (vPosition.x - fRadius + m_fCenter) * m_fCellSizeInv );
		if ( nMinCell < 0 )
			nMinCell = 0;
		int nMaxCell = (int)ceilf( (vPosition.x + fRadius + m_fCenter) * m_fCellSizeInv );
		if ( nMaxCell > (m_nCellCount - 1) )
			nMaxCell = m_nCellCount - 1;

		for(int i=nMinCell;i<=nMaxCell;++i)
		{
			Cell * pNewCell = new Cell();
			pNewCell->m_pNode = pNode;
			pNewCell->m_pNext = m_Cells[i];
			m_Cells[i] = pNewCell;
		}

		pNode = pNode->m_pNext;
	}
}

void CollisionHash1D::release()
{
	m_Cells.clear();
	m_pNodes = NULL;
}

bool CollisionHash1D::insert( Noun * pNoun )
{
	if ( m_Cells.size() != m_nCellCount )
		return false;		// hash not initialized
	if ( pNoun->collisionContext() != NULL )
		return false;			// node already attached

	CollisionHash1D::Node * pNode = new CollisionHash1D::Node();
	pNode->m_pNoun = pNoun;
	pNode->m_pNext = m_pNodes;
	m_pNodes = pNode;

	pNoun->setCollisionContext( pNode );

	float fRadius = pNoun->radius();
	const Vector3 & vPosition = pNoun->lastPosition();

	int nMinCell = (int)floorf( (vPosition.x - fRadius + m_fCenter) * m_fCellSizeInv );
	if ( nMinCell < 0 )
		nMinCell = 0;
	int nMaxCell = (int)ceilf( (vPosition.x + fRadius + m_fCenter) * m_fCellSizeInv );
	if ( nMaxCell > (m_nCellCount - 1) )
		nMaxCell = m_nCellCount - 1;

	for(int i=nMinCell;i<=nMaxCell;++i)
	{
		Cell * pCell = new Cell();
		pCell->m_pNode = pNode;
		pCell->m_pNext = m_Cells[i];
		m_Cells[i] = pCell;
	}

	return true;
}

bool CollisionHash1D::remove( Noun * pNoun )
{
	if ( m_Cells.size() != m_nCellCount )
		return false;		// hash not initialized
	// get the branch the branch was attached onto...
	CollisionHash1D::Node * pRemoveNode = (CollisionHash1D::Node *)pNoun->collisionContext();
	if (! pRemoveNode )
		return false;

	CollisionHash1D::Node * pPrevNode = NULL;
	CollisionHash1D::Node * pNode = m_pNodes;
	while( pNode != NULL )
	{
		if ( pNode == pRemoveNode )
		{
			// clear the collision context, so if we attempt to remove a 2nd time it will fail..
			pNoun->setCollisionContext( NULL );
			pNode->m_pNoun = NULL;

			CollisionHash1D::Node::Ref pNextNode = pNode->m_pNext;
			if ( pPrevNode != NULL )
				pPrevNode->m_pNext = pNextNode;
			else
				m_pNodes = pNextNode;		// next node becomes new head node..

			// node is removed, during the next call to update() it will not be added to a cell..
			return true;
		}

		pPrevNode = pNode;
		pNode = pNode->m_pNext;
	}

	// not found!
	return false;
}

void CollisionHash1D::flush()
{
	m_Cells.clear();
	m_pNodes = NULL;

	m_Cells.resize( m_nCellCount );
}

//----------------------------------------------------------------------------
//EOF
