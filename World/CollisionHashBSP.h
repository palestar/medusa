/*
	CollisionHashBSP.h

	Concrete implementation of the CollisionHashBSP object using BSP trees
	(c)2004 Palestar Inc, Richard Lyle
*/

#ifndef COLLISIONHASHBSP_H
#define COLLISIONHASHBSP_H

#include "CollisionHashAbstract.h"
#include "Math/Plane.h"

//----------------------------------------------------------------------------

class CollisionHashBSP : public CollisionHashAbstract
{
public:
	// Construction
	CollisionHashBSP();
	~CollisionHashBSP();

	//! CollisionHashAbstract interface
	virtual bool		isInitialized() const;
	virtual bool		query( const Vector3 & vPosition, float fRadius, 
							Array< NounCollision > & nodes, const ClassKey & nType ) const;

	virtual void		initialize( const BoxHull & hull );						// initialize the hash with the specified hull
	virtual void		update();												// updates the hash, queries all objects for their current position and radius
	virtual void		release();												// release the hash

	virtual bool		insert( Noun * pNoun );									// insert a collision node into the hash
	virtual bool		remove( Noun * pNoun );									// remove a collision node from the hash
	virtual void		flush();												// flush the hash, removes inserted collision nodes

private:
	// Types
	struct Leaf
	{
		Leaf( Noun * _pNoun, Leaf * _pNext );
		~Leaf();

		Noun::WeakRef	pNoun;			// worldContext object
		Leaf *			pNext;			// next object in this list
	};

	struct Branch
	{
		Branch( Branch * _pParent, const BoxHull & _hull, int splits );
		~Branch();

		void		flush();

		BoxHull		hull;
		Plane		plane;			// splitting plane
		Branch *	pParent;		// our parent branch
		Branch *	pFront;			// objects on the front side of this plane
		Branch *	pBack;			// objects on the back side of this plane
		bool		bLastBranch;	// true if this is the last branch

		Leaf *		pLeaves;		// objects on this plane
		int			nLeafCount;		// number of leaves down this branch
	};

	// Data
	Branch *		m_pBSP;			// our BSP tree
};

//----------------------------------------------------------------------------

inline CollisionHashBSP::Leaf::Leaf( Noun * _pNoun, Leaf * _pNext ) : pNoun( _pNoun ), pNext( _pNext )
{}

inline CollisionHashBSP::Leaf::~Leaf()
{
	if ( pNoun != NULL )
		pNoun->setCollisionContext( NULL );
	if ( pNext != NULL )
		delete pNext;
}

//----------------------------------------------------------------------------

inline CollisionHashBSP::Branch::Branch( Branch * _pParent, const BoxHull & _hull, int splits ) 
	: hull( _hull ), pParent( _pParent ), pFront( NULL ), pBack( NULL ), pLeaves( NULL ), nLeafCount( 0 ), bLastBranch( splits == 0 )
{
	Vector3 center( hull.center() );
	
	if ( hull.lengthX() > hull.lengthZ() )
	{
		// split the X axis
		plane.set( Vector3( 1, 0, 0 ), center );
		if ( splits > 0 )
		{
			BoxHull frontHull( hull );
			frontHull.min.x = center.x;
			BoxHull backHull( hull );
			backHull.max.x = center.x;

			pFront = new Branch( this, frontHull, splits - 1 );
			pBack = new Branch( this, backHull, splits - 1 );
		}	
	}
	else
	{
		// split the Z axis
		plane.set( Vector3( 0, 0, 1 ), center );
		if ( splits > 0 )
		{
			BoxHull frontHull( hull );
			frontHull.min.z = center.z;
			BoxHull backHull( hull );
			backHull.max.z = center.z;

			pFront = new Branch( this, frontHull, splits - 1 );
			pBack = new Branch( this, backHull, splits - 1 );
		}								
	}
}

inline CollisionHashBSP::Branch::~Branch()
{
	if ( pFront != NULL )
		delete pFront;
	if ( pBack != NULL )
		delete pBack;

	delete pLeaves;
	pLeaves = NULL;
}

inline void CollisionHashBSP::Branch::flush()
{
	if ( pFront != NULL && pFront->nLeafCount > 0 )
		pFront->flush();
	if ( pBack != NULL && pBack->nLeafCount > 0 )
		pBack->flush();

	delete pLeaves;
	pLeaves = NULL;
	nLeafCount = 0;
}

//----------------------------------------------------------------------------



#endif

//----------------------------------------------------------------------------
//EOF
