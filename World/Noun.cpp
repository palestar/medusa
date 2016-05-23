/*
	Noun.cpp

	This base class is used by the worldContext to represent a person, place, or thing
	(c)1999 Palestar Development, Richard Lyle
*/


#include "Debug/Assert.h"
#include "Debug/Profile.h"
#include "Resource/Text.h"
#include "Resource/Resource.h"
#include "Render3D/NodeSound.h"
#include "Render3D/NodeLight.h"

#include "Noun.h"
#include "NounTarget.h"
#include "WorldContext.h"
#include "VerbChat.h"
#include "TraitMovement.h"
#include "TraitReplication.h"

#include <algorithm>

static bool SortTraits( Trait::Ref p1, Trait::Ref p2 )
{
	return strcmp( p1->factory()->className(), p2->factory()->className() ) < 0;
}

//! define to non-zero to enable visibility checks on nouns, if zero then render() is called on all noun types
#define ENABLE_VISIBILITY_CHECK		0
//! define to non-zero to enable sphere visible check instead of box 
#define USE_SPHERE_VISIBLE			0

//----------------------------------------------------------------------------

NounContext			Noun::DEFAULT_CONTEXT( true );				

//-------------------------------------------------------------------------------

IMPLEMENT_FACTORY( Noun, NodeTransform );
REGISTER_FACTORY_KEY( Noun, 4033603948217583626LL );

BEGIN_PROPERTY_LIST( Noun, NodeTransform )
	ADD_TRANSMIT_UPDATE_PROPERTY( m_Position );
	ADD_TRANSMIT_UPDATE_PROPERTY( m_vOrientation );
	ADD_TRANSMIT_UPDATE_PROPERTY( m_Flags );
		//ADD_PROPERTY_FLAGS( m_Flags, PF_REPLICATE );
	ADD_TRANSMIT_UPDATE_PROPERTY( m_Tick );
	ADD_TRANSMIT_PROPERTY( m_NounContext );
	ADD_TRANSMIT_PROPERTY( m_Traits );
	ADD_PROPERTY( m_nTeamId );
	ADD_PROPERTY( m_nUserId );
	//ADD_OBJECT_PROPERTY( m_Verbs );
END_PROPERTY_LIST();

//----------------------------------------------------------------------------

Noun::Noun() : m_pContext( NULL ), 
	m_pZone( NULL ), 
	m_bDetach( false ), 
	m_bPostInit( false ),
	m_Hull( true ),
	m_fRadius( 1.0f ), 
	m_fRadius2( 1.0f ), 
	m_Flags( 0 ), 
	m_Tick( 0 ), 
	m_nTeamId( 0 ), 
	m_nUserId( 0 ), 
	m_bInitialized( false ),
	m_bSyncronized( false ), 
	m_bTransfer( false ), 
	m_pCollisionContext( NULL ), 
	m_vVelocity( 0.0f ),
	m_vWorldVelocity( 0.0f ), 
	m_vZonePosition( Vector3::ZERO ),
	m_nWorldTick( -1 ), 
	m_WorldPosition( Vector3::ZERO ),
	m_WorldFrame( Matrix33::IDENTITY )
{
	attachTrait( new TraitReplication() );
}

Noun::~Noun()
{
	// if we hit this, then this noun was not detached from the world correctly..
	ASSERT( m_pCollisionContext == NULL );
}

//----------------------------------------------------------------------------

bool Noun::write( const OutStream & output )
{
	return NodeTransform::write( output );
}

bool Noun::read( const InStream & input )
{
	if (! NodeTransform::read( input ) )
		return false;

	// update the frame of this object..
	m_Frame = Matrix33( m_vOrientation );

	// ensure the parent pointer is set correctly on our traits
	for(size_t i=0;i<m_Traits.size();)
	{
		if (! m_Traits[i].valid() )
		{
			LOG_ERROR( "Noun", "Noun::read() - NULL Trait object." );
			m_Traits.erase( m_Traits.begin() + i );
			continue;
		}

		m_Traits[i]->setParent( this );
		++i;
	}

	// make sure traits are sorted by name
	std::sort( m_Traits.begin(), m_Traits.end(), SortTraits );

	m_bPostInit = postInitialize();
	// set the last position to the current position
	setLastPosition( position() );

	return true;
}

//----------------------------------------------------------------------------

void Noun::onAttached()
{
	// call the base class
	NodeTransform::onAttached();
	// clear the detach flag, just in case a detached noun is re-attached
	m_bDetach = false;
	// get our zone
	BaseNode * pParent = parent();
	m_pZone = NULL;
	while( pParent != NULL && m_pZone == NULL )
	{
		m_pZone = WidgetCast<NodeZone>( pParent );
		pParent = pParent->parent();
	}

	// inform our zone that we are attached
	if ( m_pZone != NULL )
		m_pZone->onAttachNoun( this );
}

void Noun::onDetaching()
{
	// turn on our detach flag
	m_bDetach = true;
	// inform our zone that we are detaching
	if ( m_pZone != NULL )
	{
		m_pZone->onDetachNoun( this );
		m_pZone = NULL;
	}

	// call the base class last
	NodeTransform::onDetaching();
}

//----------------------------------------------------------------------------

void Noun::preRender( RenderContext & context,
		const Matrix33 & frame,
		const Vector3 & position )
{
#if ENABLE_VISIBILITY_CHECK
	bool bAmbient = (m_nNodeFlags & BaseNode::NF_AMBIENT) != 0;

	// NOTE: We use the worldHull() function, because it will calculate the world hull if needed..
#if USE_SPHERE_VISIBLE
	if ( bAmbient || context.sphereVisible( context.worldToView( worldHull().center() ), worldHull().radius() ) )
		NodeTransform::preRender( context, frame, position );
#else
	if ( bAmbient || context.boxVisible( context.worldToView( worldHull() ) ) )
		NodeTransform::preRender( context, frame, position );
#endif
#else
	NodeTransform::preRender( context, frame, position );
#endif
}

void Noun::render( RenderContext &context, 
		const Matrix33 & frame, 
		const Vector3 & position )
{
	NodeTransform::render( context, frame, position );
	
	NounContext * pContext = nounContext( false );
	if ( pContext != NULL)
	{
		// render the scene
		Scene * pScene = pContext->scene( false );
		if ( pScene != NULL )
			pScene->render( context, frame, position );
	}
}

void Noun::setFrame( const Matrix33 &frame )
{
	NodeTransform::setFrame( frame );
	m_vOrientation = frame.getEuler();
}

//----------------------------------------------------------------------------

bool Noun::isRootNoun() const
{
	return WidgetCast<NodeZone>( parent() ) != NULL;
}

Noun * Noun::zoneParent() const
{
	const BaseNode * pCheck = this;
	while( pCheck != NULL )
	{
		BaseNode * pParent = pCheck->parent();
		if ( WidgetCast<NodeZone>( pParent ) )
			return (Noun *)pCheck;		// return the object above the NodeZone object

		// move up the heirarchy
		pCheck = pParent;
	}

	return NULL;
}

// returns this nouns position relative to the provided zone
Vector3 Noun::zonePosition( NodeZone * pZone ) const
{
	NodeZone * pOurZone = zone();
	if ( pOurZone == NULL || pZone == NULL )
		return position();			// we have no zone, just return our current position
	else if ( pOurZone == pZone )
		return zonePosition();		// same zones, just return the zone position

	return (pOurZone->position() - pZone->position()) + zonePosition();
}

bool Noun::inLockedZone() const
{
	NodeZone * pZone = zone();
	if ( pZone != NULL )
		return pZone->locked();

	return false;
}

bool Noun::isParentNoun() const
{
	return WidgetCast<Noun>( parent() ) != NULL;
}

bool Noun::isParent( Noun * pNoun ) const
{
	Noun * pParent = parentNoun();
	while( pParent != NULL )
	{
		if ( pParent == pNoun )
			return true;
		pParent = pParent->parentNoun();
	}

	return false;
}

Noun * Noun::parentNoun() const
{
	return WidgetCast<Noun>( parent() );
}

void Noun::invalidateWorld() const
{
	m_nWorldTick = -1;
	for(int i=0;i<childCount();++i)
	{
		Noun * pChild = WidgetCast<Noun>( child(i) );
		if ( pChild != NULL )
			pChild->invalidateWorld();
	}
}

void Noun::calculateWorld() const
{
	BaseNode * pBaseNode = parent();

	m_vWorldVelocity = m_vVelocity;
	if ( WidgetCast<Noun>( pBaseNode ) )
	{
		Noun * pParent = (Noun *)pBaseNode;

		Vector3 vTranslate( pParent->worldFrame() % position() );

		m_nWorldTick = m_Tick;
		m_WorldPosition = pParent->worldPosition() + vTranslate;
		m_WorldFrame = pParent->worldFrame() % frame();
		m_vWorldVelocity += pParent->worldVelocity();
		m_vZonePosition = pParent->zonePosition() + position();
	}
	else
	{
		m_nWorldTick = m_Tick;
		m_WorldPosition = NodeTransform::worldPosition( (BaseNode *)this );
		m_WorldFrame = NodeTransform::worldFrame( (BaseNode *)this );
		m_vZonePosition = position();
	}

	m_WorldHull.setBox( hull(), m_WorldFrame, m_WorldPosition );
}

const BoxHull & Noun::worldHull() const
{
	if (! isWorldValid() )
		calculateWorld();
	return m_WorldHull;
}

const Vector3 & Noun::velocity() const
{
	return m_vVelocity;
}

const Vector3 & Noun::worldVelocity() const
{
	if (! isWorldValid() )
		calculateWorld();
	return m_vWorldVelocity;
}

//----------------------------------------------------------------------------

CharString Noun::displayName( bool a_bAllowTags ) const
{
	return BaseNode::name();
}

NounType Noun::nounType() const
{
	return TYPE_EFFECT;
}

bool Noun::canCollide() const
{
	return false;
}

bool Noun::collidable() const
{
	return false;
}

int Noun::teamId() const
{
	if ( m_nTeamId != 0 )
		return m_nTeamId;
	else if ( isParentNoun() )
		return parentNoun()->teamId();

	return 0;
}

int Noun::userId() const
{
	if ( m_nUserId != 0 )
		return m_nUserId;
	else if ( isParentNoun() )
		return parentNoun()->userId();

	return 0;
}

bool Noun::isLocal() const
{
	if ( context() && context()->isClient() )
		return userId() == context()->user()->userId();

	return userId() == 0;
}

bool Noun::isRemote() const
{
	return !isLocal();
}

//----------------------------------------------------------------------------

bool Noun::postInitialize()
{
	if ( m_NounContext.key() != WidgetKey((qword)0) && !m_NounContext.valid( true, false ) )
		return false;		// loading noun context, keep calling postInitialize() until it's loaded..

	if ( m_NounContext.valid() )
	{
		m_Hull = m_NounContext->hull();
		m_fRadius = m_Hull.radius();
		m_fRadius2 = m_fRadius * m_fRadius;

		// give this object a name if none was serialized...
		if ( name()[0] == 0 )
			setName( m_NounContext->name() );
	}

	return true;
}

void Noun::setTick( dword nTick, bool bRemoveVerbs /*= true*/ )
{
	// set the current tick
	m_Tick = nTick;

	// optionally, remove all verbs occurring before the new tick, this prevents
	// Verb::onDetach() from being called on the next update for this Noun
	if ( bRemoveVerbs )
	{
		VerbList::iterator iVerb = m_Verbs.begin();
		while( iVerb != m_Verbs.end() )
		{
			if ( (*iVerb)->tick() < nTick )
				m_Verbs.erase( iVerb++ );
			else
				++iVerb;
		}
	}
}

void Noun::setOrientation( const Euler & a_vOrientation )
{
	m_vOrientation = a_vOrientation;
	m_Frame = Matrix33( m_vOrientation );
}

int Noun::attachTrait( Trait * pTrait )
{
	if ( pTrait == NULL )
		return -1;

	pTrait->setParent( this );
	if ( m_bInitialized )
		pTrait->initialize();

	const ClassKey & nKey = pTrait->factory()->classKey();
	for(size_t i=0;i<m_Traits.size();i++)
	{
		Trait * pOldTrait = m_Traits[i];
		if ( pOldTrait->factory()->classKey() == nKey )
		{
			m_Traits[i] = pTrait;
			return i;
		}
	}

	// new trait, insert the trait, keep sorted by name
	m_Traits.push_back( pTrait );
	std::sort( m_Traits.begin(), m_Traits.end(), SortTraits );
	updateVersion();

	return m_Traits.size() - 1;
}

void Noun::detachTrait( int n )
{
	if ( n >= 0 && n < (int)m_Traits.size() )
	{
		m_Traits.erase( m_Traits.begin() + n );
		updateVersion();
	}
}

void Noun::detachAllTraits()
{
	if ( m_Traits.size() > 0 )
	{
		m_Traits.clear();
		updateVersion();
	}
}

void Noun::setSyncronized( bool bSyncronized )
{
	m_bSyncronized = bSyncronized;
	for(int i=0;i<childCount();i++)
	{
		Noun * pChild = WidgetCast<Noun>( child(i) );
		if ( pChild != NULL )
			pChild->setSyncronized( bSyncronized );
	}
}

void Noun::setTransfer( bool bTransfer )
{
	m_bTransfer = bTransfer;
	for(int i=0;i<childCount();i++)
	{
		Noun * pChild = WidgetCast<Noun>( child(i) );
		if ( pChild != NULL )
			pChild->setTransfer( bTransfer );
	}
}

void Noun::newKey( bool recurse /*= true*/ )
{
	if ( recurse )
	{
		for(int i=0;i<childCount();i++)
		{
			Noun * pChild = WidgetCast<Noun>( child(i) );
			if ( pChild != NULL )
				pChild->newKey();
		}
	}

	// generate new key
	setKey( WidgetKey() );
}

bool Noun::attachVerb( Verb * pVerb )
{
	if (! canAttachVerb( pVerb ) )
		return false;

	pVerb->setTarget( this );
	onAttachVerb( pVerb );

	// careful, if you remove the inLockedZone() then you'll end up with a huge memory leak because a server
	// will never call simulate( dword nTick ) on a Noun not in a locked zone.
	if ( !pVerb->queue() || !inLockedZone() )
	{
		//TRACE( CharString().format("Executing %s, noun = %s", pVerb->factory()->className(), name()) );

		// notify the noun the verb is being detached
		onDetachVerb( pVerb);
		// verb is not queued, execute verb now
		pVerb->onExecute();
	}
	else
	{
		// push the verb into the queue to be execute by simulate( dword nTick )
		m_Verbs.push_back( pVerb );
	}

	return true;
}

void Noun::detachVerbs()
{
	VerbList::iterator iVerb = m_Verbs.begin();
	while( iVerb != m_Verbs.end() )
	{
		Verb * pVerb = *iVerb;
		if ( pVerb->tick() <= m_Tick )
		{
			//TRACE( CharString().format("Executing %s, noun = %s", pVerb->factory()->className(), name()) );
			//PROFILE_START( pVerb->factory()->className() );
			// notify ourselves
			onDetachVerb( pVerb );
			// execute the verb
			pVerb->onExecute();
			// remove from queue
			m_Verbs.erase( iVerb++ );
			//PROFILE_END();
		}
		else
			++iVerb;
	}
}

void Noun::releaseVerbs()
{
	// remove all verbs from the queue
	m_Verbs.clear();
}

//----------------------------------------------------------------------------

void Noun::setWorldPosition( const Vector3 & position )
{
	// convert the world position into a local position by enumerating all parent nodes
	m_WorldPosition = position;
	Vector3 localPosition( position );

	BaseNode * pBaseNode = parent();
	while( pBaseNode != NULL )
	{
		NodeTransform * pNode = WidgetCast<NodeTransform>( pBaseNode );
		if ( pNode != NULL )
			localPosition = pNode->frame() * (localPosition - pNode->position());

		pBaseNode = pBaseNode->parent();
	}

	setPosition( localPosition );
}

void Noun::setWorldFrame( const Matrix33 & frame )
{
	m_WorldFrame = frame;
	Matrix33 localFrame( frame );

	BaseNode * pBaseNode = parent();
	while( pBaseNode != NULL )
	{
		NodeTransform * pNode = WidgetCast<NodeTransform>( pBaseNode );
		if ( pNode != NULL )
			localFrame = pNode->frame() * localFrame;

		pBaseNode = pBaseNode->parent();
	}

	setFrame( localFrame );
}

void Noun::setVelocity( const Vector3 & velocity )
{
	m_vVelocity = velocity;
}

void Noun::setLastPosition( const Vector3 & position )
{
	m_LastPosition = position;
}

void Noun::setCollisionContext( void * pContext )
{
	m_pCollisionContext = pContext;
}

void Noun::setTeamId( int nTeamId )
{	
	m_nTeamId = nTeamId;
}

void Noun::setUserId( int userId )
{
	m_nUserId = userId;
}

//----------------------------------------------------------------------------

void Noun::detachSelf()
{
	BaseNode * pParent = parent();
	if ( pParent != NULL )
	{
		pParent->detachNodeSwap( this );
	}
	else
	{
		TRACE( "Noun::detachSelf() - NULL parent!" );
	}
}

bool Noun::canAttachVerb( Verb * pVerb )
{
	if ( pVerb == NULL )
		return false;		// invalid verb pointer
	if (! pVerb->canAttach( this ) )
		return false;		// verb will not attach to this type of noun
	if ( m_pContext == NULL )
		return false;		// not attached to a WorldContext
	if (! m_pContext->canAttachVerb( this, pVerb ) )
		return false;		// WorldContext::User does not allow this verb

	return true;
}

void Noun::onAttachVerb( Verb * pVerb )
{
	ASSERT( m_pContext != NULL );

	// set the verb tick if not set
	if ( inLockedZone() && pVerb->tick() == 0 )
		pVerb->setTick( tick() );
	// notify the user that a verb has been attached to a noun
	m_pContext->onAttachVerb( pVerb );
}

void Noun::onDetachVerb( Verb * pVerb )
{}

void Noun::onTransferZones( NodeZone * pNewZone, NodeZone * pOldZone )
{
	// notify our traits
	for(int i=0;i<traitCount();++i)
		trait(i)->onTransferZones( pNewZone, pOldZone );
}

void Noun::onMovement( dword nTick, NodeZone * pZone, const Vector3 & vPosition, const Euler & vOrientation, bool bGlobal )
{
	TraitMovement * pTrait = FindTrait<TraitMovement>( this );
	if ( pTrait )
		pTrait->receiveMovement( nTick, pZone, vPosition, vOrientation, bGlobal );
}

void Noun::setup()
{
	// setup child objects
	for(int i=0;i<childCount();i++)
		((Noun *)child(i))->setup();
}

void Noun::initialize()
{
	CTRACE( m_bInitialized, "Noun::initialize() called on noun that is already initialized!" );

	m_bInitialized = true;

	// initialize our traits
	for(size_t i=0;i<m_Traits.size();i++)
		m_Traits[i]->initialize();
	// get the radius of this object
	m_bPostInit = postInitialize();
}


void Noun::release()
{
	// release our traits
	for(size_t i=0;i<m_Traits.size();i++)
		m_Traits[i]->release();
	// release child objects
	for(int i=0;i<childCount();i++)
		((Noun *)child(i))->release();

	m_bInitialized = false;
}

void Noun::simulate( dword nTick )
{
	if (! m_bInitialized )
		initialize();
	if (! m_bPostInit )
		m_bPostInit = postInitialize();

	// simulate our traits, do this before we update our current tick...
	for(size_t i=0;i<m_Traits.size();)
	{
		Trait * pTrait = m_Traits[i];
		if ( pTrait == NULL || pTrait->detach() )
		{
			detachTrait( i );
			continue;
		}

		pTrait->simulate( nTick );
		++i;
	}

	// simulate our children
	for(int i=0;i<childCount();)
	{
		Noun * pChild = (Noun *)child(i);
		if ( pChild->detach() )
		{
			detachNodeSwap( i );		// remove child, swap with last child...
			continue;
		}

		pChild->simulate( nTick );
		++i;
	}

	// lastly, update the current tick
	m_Tick = nTick;
	// execute and detach attached verbs
	detachVerbs();
}

// called after all nouns in a zone have been simulated... do post simulation functions in here
void Noun::update()
{
	// update our children...
	for(int i=0;i<childCount();++i)
		((Noun *)child(i))->update();
}

void Noun::collision( Noun * pCollide, const Vector3 & intersect )
{}

void Noun::onWorldTransform( const Vector3 & transform )
{
	// called when zone positions are changed, nouns should update 
	// any internal world positions by adding the vectors
	invalidateWorld();
	
	// recurse into child objects
	for(int i=0;i<childCount();++i)
		((Noun *)child(i))->onWorldTransform( transform );
}

//----------------------------------------------------------------------------

bool Noun::intersect( const Vector3 & from, const Vector3 & ray, Vector3 & intersect )
{
	// perform ray-to-sphere intersection, from Graphic Gems, page 388
	Vector3 EO( worldPosition() - from );
	float r = radius();
	float r2 = r * r;

	if ( EO.magnitude() > (ray.magnitude() + r) )
		return false;	// not close enough to intersect

	Vector3 V( ray );
	V.normalize();		// make it a unit vector

	float v = EO | V;
	float v2 = v * v;
	float disc = r2 - ((EO | EO) - v2);

	if ( disc < 0 )
		return false;	// does intersect the sphere

	float d = sqrt( disc );
	if ( v < d )
		return false;	// we are inside or the sphere is behind us

	// intersects the sphere 
	intersect = from + (V * (v - d));
	return true;
}

//----------------------------------------------------------------------------

bool Noun::isServer() const
{
	if ( m_pContext != NULL )
		return m_pContext->isServer();
	return false;
}

bool Noun::isSleeping() const
{
	if ( m_pContext != NULL )
		return m_pContext->isSleeping();
	return false;
}

bool Noun::isClient() const
{
	if ( m_pContext != NULL )
		return m_pContext->isClient();
	return false;
}

void Noun::resync()
{
	if ( m_pContext != NULL )
		m_pContext->onUpdateNoun( this );
}

void Noun::privateChat( const char * pChat )
{
	if ( userId() != 0 )
		Verb::Ref( new VerbChat( this, Verb::PRIVATE, pChat ) );
}

void Noun::teamChat( const char * pChat )
{
	Verb::Ref( new VerbChat( this, Verb::TEAM, pChat ) );
}

void Noun::factionChat( const char * pChat )
{
	Verb::Ref( new VerbChat( this, Verb::FACTION, pChat ) );
}

void Noun::globalChat( const char * pChat )
{
	Verb::Ref( new VerbChat( this, Verb::GLOBAL, pChat ) );
}

void Noun::localChat( const char * pChat )
{
	Verb::Ref( new VerbChat( this, Verb::LOCAL, pChat ) );
}

void Noun::clientChat( const char * pChat )
{
	if ( m_pContext != NULL && m_pContext->isClient() )
		m_pContext->user()->onChat( this, pChat );
}

bool Noun::intersect( float r, const Vector3 & from, const Vector3 & ray, Vector3 & intersect ) const
{
	return Noun::intersect( worldPosition(), radius() + r, from, ray, intersect );
}

int Noun::factionId() const
{
	int myTeamId = teamId();
	Context::Ref pContext = context();
	if ( pContext.valid() && pContext->isTeamValid( myTeamId ) )
		return pContext->team( myTeamId ).factionId;

	return 0;
}

const char * Noun::teamName() const
{
	int myTeamId = teamId();
	Context::Ref pContext = context();
	if ( pContext.valid() && pContext->isTeamValid( myTeamId ) )
		return pContext->team( myTeamId ).name;

	return "Unknown";
}

bool Noun::isFriend( const Noun * pWhom ) const
{
	if ( pWhom != NULL )
		return factionId() == pWhom->factionId();
	return false;
}

bool Noun::isEnemy( const Noun * pWhom ) const
{
	if ( pWhom != NULL )
	{
		int nHisFactionId = pWhom->factionId();
		return nHisFactionId != 0 && nHisFactionId != factionId();
	}

	return false;
}

bool Noun::isTeam( const Noun * pWhom ) const
{
	if ( pWhom != NULL )
		return teamId() == pWhom->teamId();
	return false;
}

void Noun::setDetach( bool bDetach /*= true*/ )
{
	m_bDetach = bDetach;
}

Material * Noun::icon() const
{
	return WidgetCast<Material>( nounContext()->resource( "ICON" ) );
}

//----------------------------------------------------------------------------

bool Noun::validateNoun( const Noun * pNoun )
{
	if ( pNoun == NULL )	
		return false;		// invalid pointer
	if ( WidgetCast<NounTarget>( pNoun ) != NULL )
		return true;		// NounTarget objects are always valid
	if ( pNoun->zone() == NULL )
		return false;		// object is detached from the world
	
	return true;
}

Noun * Noun::readTarget( const InStream & input )
{
	bool nounTarget;
	input >> nounTarget;

	if ( nounTarget )
	{
		NounTarget * pTarget = NULL;
		input >> pTarget;

		return pTarget;
	}

	WidgetKey target;
	input >> target;

	return WidgetCast<Noun>( Widget::findWidget( target ) );
}

bool Noun::writeTarget( const OutStream & output, Noun * pTarget )
{
	if ( WidgetCast<NounTarget>( pTarget ) )
	{
		output << true;
		output << pTarget;
	}
	else
	{
		output << false;
		if ( pTarget != NULL )
			output << pTarget->key();
		else
			output << NULL_WIDGET;
	}
	return true;
}

// origin = center of the sphere
// radius = radius of the sphere
// from = start of the ray
// ray = direction and magnitude of the ray
// intersect = returned intersection point if any...
bool Noun::intersect( const Vector3 & origin, float radius, const Vector3 & from, const Vector3 & ray, Vector3 & intersect )
{
	// perform ray-to-sphere intersection, from Graphic Gems, page 388
	Vector3 EO( origin - from );

	// add the radius of this object
	if ( EO.magnitude() > (ray.magnitude() + radius) )
		return false;	// not close enough to intersect

	float r2 = radius * radius;

	Vector3 V( ray );
	V.normalize();		// make it a unit vector

	float v = EO | V;
	float v2 = v * v;
	float disc = r2 - ((EO | EO) - v2);

	if ( disc < 0 )
		return false;	// does intersect the sphere

	float d = sqrt( disc );
	if ( v < -d )
		return false;	// we are inside or the sphere is behind us

	// intersects the sphere 
	intersect = from + (V * (v - d));
	return true;
}

//----------------------------------------------------------------------------
// EOF
