/*
	NodeZone.cpp
	(c)2000 Palestar Inc, Richard Lyle
*/

#define MEDUSA_TRACE_ON

#include "Debug/Assert.h"
#include "Debug/Profile.h"
#include "Standard/Time.h"
#include "WorldContext.h"
#include "NodeZone.h"

#include "CollisionHashBSP.h"
#include "CollisionHash1D.h"

#include "Standard/StringBuffer.h"
#include "Standard/STLHelper.h"

#include <set>

//----------------------------------------------------------------------------

//! Define this to the class to use for our collision detection...
#define COLLISION_HASH_CLASS		CollisionHashBSP
//#define COLLISION_HASH_CLASS		CollisionHash1D
//! Define to non-zero to turn on profiling of all noun updates
#define PROFILE_NOUNS				0

// object limits
Constant		MAX_ZONE_PLATFORMS( "MAX_ZONE_PLATFORMS", 128 );
// Any objects moving faster than this are ignored...
Constant		COLLISION_SANITY_CHECK( "COLLISION_SANITY_CHECK", 2000.0f );
//! Any collision radius larger than this should be checked into..
Constant		COLLISION_LARGE_CHECK( "COLLISION_LARGE_CHECK", 200.0f );

//----------------------------------------------------------------------------

IMPLEMENT_FACTORY( NodeZone, NodeTransform );
REGISTER_FACTORY_KEY( NodeZone, 4444418230630222717LL );

BEGIN_PROPERTY_LIST( NodeZone, NodeTransform )
	ADD_PROPERTY_FLAGS( m_Position, PF_TRANSMIT );
	ADD_PROPERTY_FLAGS( m_Name, PF_NOSAVE );
	ADD_PROPERTY_FLAGS( m_Children, PF_TRANSMIT );
	ADD_TRANSMIT_PROPERTY( m_Scenery );
	ADD_TRANSMIT_PROPERTY( m_Hull );
END_PROPERTY_LIST();

NodeZone::NodeZone() : 
	m_Tick( 0 ), 
	m_pContext( NULL ), 
	m_nProfileTime( 0 ), 
	m_nNounCount( 0 ), 
	m_bLocked( false ), 
	m_pCollisionHash( new COLLISION_HASH_CLASS )
{}

NodeZone::~NodeZone()
{
	releaseZone();
	delete m_pCollisionHash;
}

//---------------------------------------------------------------------------------------------------

bool NodeZone::read( const InStream & input )
{
	if (! NodeTransform::read( input ) )
		return false;

	setZone( this, this );
	return true;
}

bool NodeZone::write( const OutStream & output )
{
	// if this is a client serialization, then strip out nouns that shouldn't be transmitted, etc..
	if ( (output.filter() & WorldContext::FF_CLIENT) != 0 )
	{
		// this array is for child objects that we don't serialized down to the client..
		Children saved;
		// remove any nouns that shouldn't go down to the client..
		size_t i =0;
		while( i < m_Children.size() )
		{
			Noun * pNoun = WidgetCast<Noun>( m_Children[i] );
			if ( pNoun != NULL 
				&& pNoun->nounType() != TYPE_OBJECT
				&& pNoun->nounType() != TYPE_LANDMARK )
			{
				saved.push_back( pNoun );
				removeSwapIndex( m_Children, i );
			}
			else 
				++i;
		}

		// now call the base class which will write m_Children to the output..
		bool bResult = NodeTransform::write( output );

		// restore the child objects...
		for(size_t i=0;i<saved.size();++i)
			m_Children.push_back( saved[i] );

		return bResult;
	}

	return NodeTransform::write( output );
}

//----------------------------------------------------------------------------

void NodeZone::onChildDetached()
{
	NodeTransform::onChildDetached();

	if ( childCount() == 0 
		&& context() != NULL 
		&& context()->user() != NULL
		&& context()->user()->isServer() && locked() )
	{
		TRACE("NodeZone::onChildDetached() - Last object has left zone, unlocking zone!" );
		context()->unlockZone( this );
	}
}

BoxHull NodeZone::hull() const
{
	return m_Hull;
}

void NodeZone::preRender( RenderContext &context, const Matrix33 & frame, const Vector3 & position )
{
	PROFILE_START("NodeZone::preRender()");

	context.setInstanceKey( context.instanceKey() + key() );

	Matrix33 vWorldFrame( frame * m_Frame );
	Vector3 vWorldPosition( (frame * m_Position) + position );
	Matrix33 vViewFrame( context.worldToView( vWorldFrame ) );
	Vector3 vViewPosition( context.worldToView( vWorldPosition ) );

	// if this zone isn't visible, then we render ambient children only..
	bool bAmbientOnly = !context.boxVisible( m_Hull, vViewFrame, vViewPosition );
	
	// render all children
	for(int i=0;i<childCount();i++)
	{
		BaseNode * pChild = child(i);
		if ( !bAmbientOnly || (pChild->nodeFlags() & BaseNode::NF_AMBIENT) != 0 )
			pChild->preRender( context, vWorldFrame, vWorldPosition );
	}

	context.setInstanceKey( context.instanceKey() - key() );
	PROFILE_END();
}

//----------------------------------------------------------------------------

void NodeZone::initializeZone( WorldContext * pContext )
{
	// set the context on this zone and all nouns
	setContext( pContext );
	// set the zone pointer for all nouns
	setZone( this, this );
	
	bool bServer = pContext->isServer();
	if ( childCount() > 0 ) 
	{
		m_pCollisionHash->initialize( m_Hull );
		// attach all child objects to the update chain
		for(int i=0;i<childCount();++i)
		{
			Noun * pNoun = WidgetCast<Noun>( child(i) );
			if (! pNoun )
				continue;

			hookNouns( pNoun );

			// if on the server, then set all nouns as syncronized
			if ( bServer )
				pNoun->setSyncronized( true );
		}
	}

	setTick( this, pContext->tick() );
}

void NodeZone::releaseZone()
{
	// unhook all nouns
	for(int i=0;i<childCount();++i)
		unhookNouns( child(i) );
	// delete the hash 
	m_pCollisionHash->release();
}

void NodeZone::setLocked( bool bLocked )
{
	m_bLocked = bLocked;
}

void NodeZone::setScenery( Scene * pScenery )
{
	m_Scenery = pScenery;
}

void NodeZone::setHull( const BoxHull & hull )
{
	// set the hull
	m_Hull = hull;
	setName( (CharString)m_Hull );
}

void NodeZone::setContext( WorldContext * pContext )
{
	m_pContext = pContext;
	// update the context on all child objects
	setContext( this, pContext );
}

void NodeZone::setTick( dword tick )
{
	m_Tick = tick;
	// set the tick on all child objects
	setTick( this, tick );
}
	
bool NodeZone::attachNoun( Noun * pNoun, BaseNode * pParent /*= NULL*/ )
{
	ASSERT( pNoun != NULL );

	// if parent is null, then attach directly to this zone
	if ( pParent == NULL )
		pParent = this;

	// set the nouns zone now
	pNoun->setZone( this );
	// attach the node to it's parent
	pParent->attachNode( pNoun );
	
	return true;
}

bool NodeZone::replaceNoun( Noun * pReplace, Noun * pWith )
{
	ASSERT( pReplace && pWith );

	BaseNode * pRoot = pReplace->parent();
	ASSERT( pRoot );

	for(int i=0;i<pRoot->childCount();i++)
		if ( pRoot->child(i) == pReplace )
		{
			// update the scene
			pRoot->detachNodeSwap( i );
			// attach the noun to it's parent
			pRoot->insertNode( i, pWith );

			return true;
		}

	return false;
}

void NodeZone::detachNoun( Noun * pDetach )
{
	pDetach->detachSelf();
}

void NodeZone::leaveZone( Noun * pNoun )
{
	ERROR_ON( context() == NULL );
	ERROR_ON( pNoun->parent() != this );

	// find new zone for this noun, or detach the noun if it's no longer in the world
	NodeZone * pInZone = NULL;
	for(int i=0;i<context()->zoneCount() && pInZone == NULL;i++)
	{
		NodeZone * pZone = context()->zone( i );
		ASSERT( pZone );

		if ( pZone == this )
			continue;
		if ( pZone->hull().inBox( pNoun->position() + (position() - pZone->position()) ) )
			pInZone = pZone;
	}

	if ( pInZone != NULL )
	{
		transferNoun( pNoun, pInZone, true );	// transfer noun to it's new zone
	}
	else
	{
		// clamp the nouns position to stay within the zone...
		Vector3 vPosition( pNoun->position() );
		hull().clamp( vPosition );
		pNoun->setPosition( vPosition );

		//TRACE( CharString().format("NodeZone::leaveZone() - Noun %s not in zone at %s", 
		//	pNoun->name(), ConvertType<CharString>( pNoun->position() ) ) );
		//detachNoun( pNoun );					// remove the noun if outside the worldContext
	}
}

bool NodeZone::transferNoun( Noun * pNoun, NodeZone * pNewZone, bool updatePosition )
{
	ASSERT( pNoun );
	ASSERT( pNewZone );

	if ( pNewZone == this )
		return false;		// no transfer

	BaseNode * pParent = pNoun->parent();
	if ( pParent == NULL )
		return false;		// noun must be attached
	if ( pParent != this )
		return false;		// we can only transfer root nouns, no children allowed

	// prevent the noun from being deleted while transferring
	if (! pNoun->grabReference() )
		return false;		// noun is already destroyed

	// turn off the syncronized flag, so that the detach is not propagated 
	// to other servers/clients causing the object to flash in-out of existence
	pNoun->setSyncronized( false );
	pNoun->setTransfer( true );
	// detach the noun from this zone
	detachNodeSwap( pNoun );

	// optionally, transform the position of the noun
	if ( updatePosition )
	{
		// calculate the new position
		Vector3 offset( position() - pNewZone->position() );
		Vector3 newPosition( pNoun->position() + offset );

		// make sure the position is inside the bounding box of the new zone, don't clamp the last position however
		pNewZone->hull().clamp( newPosition );
		// set the new positions now
		pNoun->setPosition( newPosition );
		pNoun->setLastPosition( newPosition );
	}

	// turn on the synchronized flag, so the noun is not sent to other servers/clients
	pNoun->setSyncronized( true );
	// attach the noun to it's new zone
	pNewZone->attachNode( pNoun );
	pNoun->setTransfer( false );
	pNoun->setZone( pNewZone );
	// release our reference
	pNoun->releaseReference();
	pNoun->invalidateWorld();

	if ( updatePosition )
	{
		// notify the noun of the zone change...
		pNoun->onTransferZones( pNewZone, this );
		// inform the user we have transfered a noun
		if ( context() != NULL )
			context()->onTransferNoun( pNoun, pNewZone, this );
	}

	return true;
}

void NodeZone::onAttachNoun( Noun * pNoun )
{
	// hook the noun into the update chain
	hookNouns( pNoun );
	// inform the context user of the new noun
	if ( context() != NULL && context()->user() != NULL )
	{
		// don't inform the user if we are just moving the object to a new zone...
		if ( !pNoun->transfer() )
			context()->onAttachNoun( pNoun );
		if ( context()->user()->isServer() && !locked() )
			context()->lockZone( this );
	}
}

void NodeZone::onDetachNoun( Noun * pNoun )
{
	// inform the context user that this noun is being detached
	if ( context() != NULL && context()->user() != NULL )
	{
		// don't inform the user if we are just moving the object to a new zone...
		if ( !pNoun->transfer() )
			context()->onDetachNoun( pNoun );
	}

	// detach the noun from the update chain
	unhookNouns( pNoun );
}

void NodeZone::onWorldTransform( const Vector3 & vAdd )
{
	// update our current position
	setPosition( position() + vAdd );

	// inform all child nodes of the transform, so they can update any world positions
	for(int i=0;i<childCount();i++)
		((Noun *)child(i))->onWorldTransform( vAdd );
}

const int MAX_TICKS = TICKS_PER_SECOND * 30;

void NodeZone::simulate( dword nTick )
{
	if (! context() )
		return;

	qword nZoneStart = Time::CPU();

	PROFILE_START( "NodeZone::simulate()" );

	// get current time from the context
	m_Tick = context()->tick();

	// update all nouns
	for(int i=0;i<childCount();)
	{
		Noun * pChild = (Noun *)child(i);
		if ( pChild->detach() )
		{
			detachNodeSwap( i );		// detach the child, replace it with the child from the end of the array..
			continue;
		}

#if PROFILE_NOUNS
		PROFILE_START( pChild->className() );
		pChild->simulate( m_Tick );
		PROFILE_END();
#else
		pChild->simulate( m_Tick );
#endif
		++i;
	}

	// perform collisions between objects
	detectCollisions();

	// perform post-simulation update on all nouns
	for(int i=0;i<childCount();++i)
		((Noun *)child(i))->update();

	PROFILE_END();

	// add CPU used time to the zone counters
	setProfileTime( Time::CPU() - nZoneStart );
}

void NodeZone::close()
{
	Queue< Noun::Ref > nouns;
	for(int i=0;i<childCount();i++)
		nouns.push( (Noun *)child(i) );

	while( nouns.valid() )
	{
		leaveZone( *nouns );
		nouns.pop();
	}
}

// use the collision hash to find nouns in the area
bool NodeZone::proximityCheck( const Vector3 & vPosition, float fRadius, Array< NounCollision > & nouns )
{
	//PROFILE_FUNCTION();
	if ( childCount() == 0 )
		return false;

	return m_pCollisionHash->query( vPosition - position(), fRadius, nouns );
}

bool NodeZone::proximityCheck( const Vector3 & vPosition, float fRadius, Array< NounCollision > & nouns, const ClassKey & nType )
{
	//PROFILE_FUNCTION();
	if ( childCount() == 0 )
		return false;

	return m_pCollisionHash->query( vPosition - position(), fRadius, nouns, nType );
}

//----------------------------------------------------------------------------

bool NodeZone::isActiveZone() const
{
	return context() != NULL ? context()->activeZone() == this : false;
}

bool NodeZone::inZone( Noun * pNoun ) const
{
	return pNoun != NULL ? pNoun->zone() == this : false;
}

//----------------------------------------------------------------------------

void NodeZone::setContext( BaseNode * pNode, WorldContext * pContext )
{
	if ( WidgetCast<Noun>( pNode ) )
		((Noun *)pNode)->setContext( pContext );
	for(int i=0;i<pNode->childCount();i++)
		setContext( pNode->child(i), pContext );
}

void NodeZone::setZone( BaseNode * pNode, NodeZone * pZone )
{
	if ( WidgetCast<Noun>( pNode ) )
		((Noun *)pNode)->setZone( pZone );
	for(int i=0;i<pNode->childCount();i++)
		setZone( pNode->child(i), pZone );
}

void NodeZone::setTick( BaseNode * pNode, dword tick )
{
	if ( WidgetCast<Noun>( pNode ) )
		((Noun *)pNode)->setTick( tick );
	for(int i=0;i<pNode->childCount();i++)
		setTick( pNode->child(i), tick );
}

void NodeZone::updateLastPosition( BaseNode * pNode )
{
	if ( WidgetCast<Noun>( pNode ) )
		((Noun *)pNode)->setLastPosition( ((Noun *)pNode)->zonePosition() );
	for(int i=0;i<pNode->childCount();i++)
		updateLastPosition( pNode->child(i));
}

//---------------------------------------------------------------------------------------------------

typedef std::pair< qword, qword >	CollisionKey;
typedef std::set< CollisionKey >	CollisionSet;

static bool IsCollisionChecked( CollisionSet & a_collisionSet, const qword & a_Key1, const qword & a_Key2 )
{
	// note, we have to make sure the higher key is the first in the pair always, so the CollisionKey
	// will be identical in the set no matter what order the keys are passed into this function.
	CollisionKey k( a_Key1 > a_Key2 ? CollisionKey( a_Key1, a_Key2 ) : CollisionKey( a_Key2, a_Key1 ) );
	if ( a_collisionSet.find( k ) != a_collisionSet.end() )
		return true;
	a_collisionSet.insert( k );
	return false;
}

//----------------------------------------------------------------------------

typedef std::list< Noun::Ref >		NounList;

void NodeZone::detectCollisions()
{
	PROFILE_START( "NodeZone::detectCollisions()" );

	// use a std::set to track which objects have already been informed of their collisions...
	CollisionSet collisionSet;
	// this array is used to track any objects outside of this zone
	NounList outsideZone;
	// detect collisions for all nouns in this zone
	for(int j=0;j<childCount();++j)
	{
		Noun::Ref pNoun = (Noun *)child(j);

		Vector3 vStart( pNoun->lastPosition() );
		Vector3 vEnd( pNoun->zonePosition() );
		
		if ( !m_Hull.inBox( vEnd ) )
		{
			outsideZone.push_back( pNoun );
			continue;		// don't even bother, the noun is outside our zone
		}

		// check for root objects that canCollide()
		if ( pNoun->canCollide() )
		{
			// get the change in position since the hash was last updated
			Vector3 vDelta( vEnd - vStart );

			// get the collision radius and add the radius of our change in position
			float fNounRadius = pNoun->radius();
			float fDeltaRadius = vDelta.magnitude();
			float fRadius = fNounRadius + fDeltaRadius + 1.0f;		// add +1 so we don't miss anything due to rounding errors

			if ( fRadius > COLLISION_SANITY_CHECK )
			{
				LOG_DEBUG_MED( "NodeZone", "Skipping collision for %s, %.1f > %.1f", pNoun->name(), fRadius, (float)COLLISION_SANITY_CHECK );
				pNoun->setLastPosition( vEnd );
				continue;			// skip this, collision check... the range is insane!
			}

			if ( fRadius > COLLISION_LARGE_CHECK )
				LOG_DEBUG_LOW( "NodeZone", "Large collision for %s, %.1f > %.1f", pNoun->name(), fRadius, (float)COLLISION_LARGE_CHECK );

			// find all nouns within the possible collision radius
			Array< NounCollision > nodes;
			m_pCollisionHash->query( vStart, fRadius, nodes );

			// process the possible collisions
			for(int i=0;i<nodes.size();i++)
			{
				NounCollision & check = nodes[ i ];

				Noun * pCheck = check.pNoun;
				if ( pCheck == pNoun )
					continue;		// skip collision test against self
				if ( pCheck->parent() == NULL )
					continue;		// verify this noun is still attached to this zone

				// check if we already did a collision check on both objects already..
				if ( IsCollisionChecked( collisionSet, pNoun->key(), pCheck->key() ) )
					continue;		// skip any checks, objects already have collided

				Vector3 vCheckStart( pCheck->lastPosition() );
				Vector3 vCheckEnd( pCheck->zonePosition() );
				Vector3 vCheckDelta( vCheckEnd - vCheckStart );
				float fCheckRadius = pCheck->radius();

				// do a ray to sphere intersection both ways.. 
				Vector3 vIntersect[2];
				bool bIntersectA = Noun::intersect( vCheckEnd, fCheckRadius + fNounRadius, vStart, vDelta, vIntersect[0] );
				bool bIntersectB = Noun::intersect( vEnd, fCheckRadius + fNounRadius, vCheckStart, vCheckDelta, vIntersect[1] );

				if ( bIntersectA || bIntersectB )
				{
					if (! bIntersectB )
						vIntersect[1] = vIntersect[0];
					else if (! bIntersectA )
						vIntersect[0] = vIntersect[1];
					//TRACE( CharString().format("Sphere/Ray collision between %s and %s, distance %.1f gu", 
					//	pNoun->name(), pCheck->name(), check.distance) );

					// yes objects collided
					pNoun->collision( pCheck, vIntersect[0] + position() );
					pCheck->collision( pNoun, vIntersect[1] + position() );
				}
			}

		} // if ( pNoun->canCollide() )
	}

	// process all nouns that are outside this zone
	for ( NounList::iterator iNoun = outsideZone.begin(); iNoun != outsideZone.end(); ++iNoun )
		leaveZone( *iNoun );
	// update the last position vector on all collidable objects in this zone..
	updateLastPosition( this );
	// update the collision hash, this will allow us to find objects by their last position..
	m_pCollisionHash->update();

	PROFILE_END();
}

const dword MAX_NOUN_TICK = TICKS_PER_SECOND * 10;

bool NodeZone::hookNouns( BaseNode::Ref pNode )
{
	Noun * pNoun = WidgetCast<Noun>( pNode );
	if ( pNoun == NULL )
	{
		LOG_ERROR( "NodeZone", "hookNouns called for non-noun node. (%s).", pNode != NULL ? pNode->name() : "?" );
		return false;
	}

	// update the object counts
	++m_nNounCount;

	pNoun->setLastPosition( pNoun->zonePosition() );
	pNoun->setContext( context() );
	pNoun->setZone( this );

	// check the nouns current tick, set the tick if not set
	if ( pNoun->tick() == 0 )
		pNoun->setTick( m_Tick );

	// don't call initialize() if this noun is just getting transfered from one zone to another..
	if ( !pNoun->transfer() )
		pNoun->initialize();

	// if collidable then insert into the collision hash
	if ( pNoun->collidable() )
	{
		if ( !m_pCollisionHash->isInitialized() )
			m_pCollisionHash->initialize( m_Hull );
		m_pCollisionHash->insert( pNoun );
	}

	// recurse into children
	for(int i=0;i<pNode->childCount();i++)
	{
		if (! hookNouns( pNode->child(i) ) )
		{
			LOG_ERROR( "NodeZone", "Failed to hook child node %d on parent node %s", i, pNode->name() );
			pNode->detachNodeSwap( i-- );		// remove the invalid child node..
		}
	}

	return true;
}

void NodeZone::unhookNouns( BaseNode::Ref pNode )
{
	for(int i=0;i<pNode->childCount();i++)
		unhookNouns( pNode->child(i) );

	Noun * pNoun = WidgetCast<Noun>( pNode );
	if ( pNoun != NULL )
	{
		if ( pNoun->collisionContext() != NULL )
			m_pCollisionHash->remove( pNoun );

		// release the noun, we don't release() this object if it's just getting transfered
		// from this zone to another... since that may cause internal pointers to get reset.
		if ( !pNoun->transfer() )
			pNoun->release();

		// update the object counts
		--m_nNounCount;

		pNoun->setTick( tick(), false );			// set the current tick, so if the noun is being simulated, it will stop...
		pNoun->setZone( NULL );
		//pNoun->setContext( NULL );	-- Commented out, was causing kamikaze points to be rewarded instead of friendly fire
	}
}

//----------------------------------------------------------------------------
//EOF

