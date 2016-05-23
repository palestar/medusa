/*
	CollisionHashBSP.cpp
	(c)2004 Palestar Inc, Richard Lyle
*/


#include "Standard/Constant.h"

#include "Debug/Profile.h"
#include "CollisionHashBSP.h"
#include "WorldContext.h"

//----------------------------------------------------------------------------

// when making the BSP tree, what is our target size for each hull
Constant		BSP_TARGET_HULL_SIZE( "BSP_TARGET_HULL_SIZE", 1000.0f );
// when building the BSP, this limits the number of split levels
Constant		BSP_MAX_SPLITS( "BSP_MAX_SPLITS", 12 );

//----------------------------------------------------------------------------

CollisionHashBSP::CollisionHashBSP() : m_pBSP( NULL )
{}

CollisionHashBSP::~CollisionHashBSP()
{
	release();
}

//----------------------------------------------------------------------------

bool CollisionHashBSP::isInitialized() const
{
	return m_pBSP != NULL;
}

bool CollisionHashBSP::query( const Vector3 & vPosition, float fRadius, Array< NounCollision > & nodes, const ClassKey & nType ) const
{
	if ( m_pBSP == NULL )
		return false;		// hash not initialized
	if (! m_pBSP->hull.intersect( vPosition, fRadius ) )
		return false;		// make sure it intersects with our main hull

	//PROFILE_START( "CollisionHashBSP::query()" );

	float fRadius2 = fRadius * fRadius;

	Array< Branch * > branches;
	branches.push( m_pBSP );

	int nBranchCount = 0;
	while( nBranchCount < branches.size() )
	{
		Branch * pBranch = branches[ nBranchCount++ ];

		// check the nodes in this branch
		Leaf * pLeaf = pBranch->pLeaves;
		while( pLeaf != NULL )
		{
			Noun * pCheck = pLeaf->pNoun;
			pLeaf = pLeaf->pNext;

			if ( pCheck == NULL )
				continue;
			if ( nType != ClassKey((qword)0) && !pCheck->factory()->isType( nType ) )
				continue;

			float fDistance2 = (pCheck->lastPosition() - vPosition).magnitude2();
			if ( fDistance2 <= (fRadius2 + pCheck->radius2()) )
			{
				NounCollision & node = nodes.push();
				node.pNoun = pCheck;
				node.fDistance = sqrtf( fDistance2 );
			}
		}

		if (! pBranch->bLastBranch )
		{
			Plane::Side nSide = pBranch->plane.compare( vPosition, fRadius );
			if ( (nSide & Plane::BACK) && pBranch->pBack->nLeafCount > 0)
				branches.push( pBranch->pBack );
			if ( (nSide & Plane::FRONT) && pBranch->pFront->nLeafCount > 0 )
				branches.push( pBranch->pFront );
		}
	}
	//PROFILE_END();

	return nodes.size() > 0;
}

//----------------------------------------------------------------------------

void CollisionHashBSP::initialize( const BoxHull & hull )
{
	// release the previous tree if any
	release();

	// determine the number of splits based on the size of the HULL... try for an optimal size 
	// specified by UNIVERSE_HULL_SIZE
	float xAxis = hull.lengthX();
	float zAxis = hull.lengthZ();

	int splits = 0;
	while( xAxis > BSP_TARGET_HULL_SIZE || zAxis > BSP_TARGET_HULL_SIZE )
	{
		if ( xAxis > zAxis )
			xAxis *= 0.5f;
		else
			zAxis *= 0.5f;
		splits++;

		if ( splits >= BSP_MAX_SPLITS )
			break;
	}

	// build the splitting planes
	m_pBSP = new Branch( NULL, hull, splits );
}

void CollisionHashBSP::update()
{
	if (! m_pBSP )
		return;

	Array< Branch * > branches;
	branches.push( m_pBSP );

	Leaf * pLeaves = NULL;

	int nBranchCount = 0;
	while( nBranchCount < branches.size() )
	{
		Branch * pBranch = branches[ nBranchCount++ ];

		if ( pBranch->pFront && pBranch->pFront->nLeafCount > 0 )
			branches.push( pBranch->pFront );
		if ( pBranch->pBack && pBranch->pBack->nLeafCount > 0 )
			branches.push( pBranch->pBack );

		Leaf * pLeaf = pBranch->pLeaves;
		while( pLeaf != NULL )
		{
			Leaf * pNext = pLeaf->pNext;
			Noun * pNoun = pLeaf->pNoun;
			if ( pNoun != NULL )
			{
				pLeaf->pNext = pLeaves;
				pLeaves = pLeaf;
			}
			else
			{
				pLeaf->pNext = NULL;		// set to NULL so it doesn't delete the remaining leaves
				delete pLeaf;
			}

			pLeaf = pNext;
		}

		pBranch->pLeaves = NULL;
		pBranch->nLeafCount = 0;
	}

	Leaf * pLeaf = pLeaves;
	while( pLeaf != NULL )
	{
		Leaf * pNext = pLeaf->pNext;

		Vector3 vPosition = pLeaf->pNoun->lastPosition();
		float	fRadius = pLeaf->pNoun->radius();

		Branch * pBranch = m_pBSP;
		while ( pBranch != NULL )
		{
			pBranch->nLeafCount++;

			if (! pBranch->bLastBranch )
			{
				Plane::Side nSide = pBranch->plane.compare( vPosition, fRadius );
				if ( nSide == Plane::BACK )
				{
					pBranch = pBranch->pBack;
					continue;
				}
				else if ( nSide == Plane::FRONT )
				{
					pBranch = pBranch->pFront;
					continue;
				}
			}

			pLeaf->pNoun->setCollisionContext( pBranch );
			pLeaf->pNext = pBranch->pLeaves;
			pBranch->pLeaves = pLeaf;

			break;
		}

		pLeaf = pNext;
	}
}

void CollisionHashBSP::release()
{
	if ( m_pBSP != NULL )
	{
		delete m_pBSP;
		m_pBSP = NULL;
	}
}

bool CollisionHashBSP::insert( Noun * pNoun )
{
	if ( m_pBSP == NULL )
		return false;			// not initialized
	if ( pNoun->collisionContext() != NULL )
		return false;			// node already attached

	const Vector3 & vPosition = pNoun->lastPosition();
	float	fRadius = pNoun->radius();

	Branch * pBranch = m_pBSP;
	while ( pBranch != NULL )
	{
		pBranch->nLeafCount++;

		if (! pBranch->bLastBranch )
		{
			Plane::Side nSide = pBranch->plane.compare( vPosition, fRadius );
			if ( nSide == Plane::BACK )
			{
				pBranch = pBranch->pBack;
				continue;
			}
			else if ( nSide == Plane::FRONT )
			{
				pBranch = pBranch->pFront;
				continue;
			}
		}

		pNoun->setCollisionContext( pBranch );
		// noun is on the planet, or no more branches
		pBranch->pLeaves = new Leaf( pNoun, pBranch->pLeaves );
		return true;
	}

	return false;
}

bool CollisionHashBSP::remove( Noun * pNoun )
{
	if ( m_pBSP == NULL )
		return false;			// not initialized

	// get the branch the branch was attached onto...
	Branch * pBranch = (Branch *)pNoun->collisionContext();
	if ( pBranch == NULL )
		return false;

	Leaf * pLeaf = pBranch->pLeaves;
	Leaf * pLastLeaf = NULL;
	while( pLeaf != NULL )
	{
		if ( pLeaf->pNoun.pointer() == pNoun )
		{
			// clear the collision context, so if we attempt to remove a 2nd time it will fail..
			pNoun->setCollisionContext( NULL );

			// remove from the linked list
			if ( pLastLeaf != NULL )
				pLastLeaf->pNext = pLeaf->pNext;
			else
				pBranch->pLeaves = pLeaf->pNext;

			// set the next pointer to null, or the destructor will destroy linked leaves
			pLeaf->pNext = NULL;
			// delete the leaf, make sure pNext is null first
			delete pLeaf;

			// update the leaf counts
			while( pBranch != NULL )
			{
				pBranch->nLeafCount--;
				pBranch = pBranch->pParent;
			}

			// node removed from the tree
			return true;
		}

		pLastLeaf = pLeaf;
		pLeaf = pLeaf->pNext;
	}

	// not found!
	return false;
}

void CollisionHashBSP::flush()
{
	if ( m_pBSP != NULL )
		m_pBSP->flush();
}

//----------------------------------------------------------------------------
//EOF
