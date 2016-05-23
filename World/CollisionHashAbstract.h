/*
	CollisionHashAbstract.h

	Abstract interface for a CollisionHash
	(c)2004 Palestar Inc, Richard Lyle
*/

#ifndef COLLISIONHASHABSTRACT_H
#define COLLISIONHASHABSTRACT_H

#include "Math/Vector3.h"
#include "Math/BoxHull.h"
#include "Noun.h"

//----------------------------------------------------------------------------

class CollisionHashAbstract
{
public:
	// Types
	struct NounCollision		// result structure, so that distance doesn't need to be recalculated again
	{
		Noun *			pNoun;
		float			fDistance;
	};

	// Accessors
	virtual bool		isInitialized() const = 0;
	virtual bool		query( const Vector3 & vPosition, float fRadius, 
							Array< NounCollision > & nodes, const ClassKey & nType ) const = 0;

	// Mutators
	virtual void		initialize( const BoxHull & hull ) = 0;					// initialize the hash with the specified hull
	virtual void		update() = 0;											// updates the hash, queries all objects for their current position and radius
	virtual void		release() = 0;											// release the hash

	virtual bool		insert( Noun * pNoun ) = 0;								// insert a collision node into the hash
	virtual bool		remove( Noun * pNoun ) = 0;								// remove a collision node from the hash
	virtual void		flush() = 0;											// flush the hash, removes inserted collision nodes

	// Helpers
	bool				query( const Vector3 & vPosition, float fRadius, 
							Array< NounCollision > & nodes ) const;				// get a list of all objects within a certain range and position
};

//---------------------------------------------------------------------------------------------------

inline bool CollisionHashAbstract::query( const Vector3 & vPosition, float fRadius, Array< NounCollision > & nodes ) const
{
	return query( vPosition, fRadius, nodes, (qword)0 );
}

//---------------------------------------------------------------------------------------------------

#endif

//----------------------------------------------------------------------------
//EOF
