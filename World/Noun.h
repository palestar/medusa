/*
	Noun.h

	This base class is used by the worldContext to represent a person, place, or thing
	(c)1999 PaleStar, Richard Lyle
*/

#ifndef NOUN_H
#define NOUN_H

#include "Math/Euler.h"
#include "Render3D/NodeTransform.h"
#include "Network/LogClient.h"

#include "TargetReference.h"
#include "NounContext.h"
#include "Verb.h"
#include "Trait.h"
#include "WorldDll.h"

//-------------------------------------------------------------------------------

class WorldContext;		// forward declare
class NodeZone;

//----------------------------------------------------------------------------

// Types of nouns affect how the noun is syncronized between worlds, when and if it's culled, and how it's updated or put to sleep
enum NounType
{
	TYPE_EFFECT,	
	/*
		This noun type is used for nouns that are local effects such as explosions, debris, and other objects. This type
		of object is NEVER sent to other clients or servers.
	*/

	TYPE_SERVER_OBJECT,
	/*
		This type of noun is only for the server side only. These types of nouns are attached to the world as
		helpers. This object is sent to other servers, but never sent to the clients.
	*/

	TYPE_LOCAL_OBJECT,
	/*
		This type of noun is sent to clients in the local area. If a client leaves the local area of the object,
		then a message is sent to the client to delete the object.
	*/

	TYPE_OBJECT,
	/*
		This type of noun is sent to to all clients and servers. Clients outside the local area, receive a proxy object 
		instead of the full object. If a client enters the local area around this type of noun, the full object is sent
		to the client. If a client leaves the local area of the object, it is left attached to the world as a proxy object.
	*/

	TYPE_LANDMARK,
	/*
		This object is reservered for primary landmarks in the game, such as planets, stars, and nebula. This type of 
		noun is sent to all clients and servers when added to the world. If a client enters the local area of this
		object, then the server should send a current copy of this noun to the client. Behavior of this type is very much
		like TYPE_OBJECT, except a the entire object is sent instead of a proxy for clients outside the local area.
	*/
};

/*
	NOTES: The 2 virtuals canCollide() and collidable() have very important effects on the performance of the collision 
	detection system. Very large objects (i.e. planets, stars) should always return false for canCollide(), but true for collidable(). 
	The reason is simple, if canCollide() returns true, then all the objects within the radius of that object are checked 
	for collision, this can be alot of objects for larger objects, and thus slowing down the entire collision detection system.

	Small objects (projectiles, ships) should return true on canCollide(), since their radius is typically smaller, it's much 
	faster to find all the large objects they may collide against, since it will be a very small area being checked.
*/

//----------------------------------------------------------------------------

class DLL Noun : public NodeTransform
{
public:
	DECLARE_WIDGET_CLASS();
	DECLARE_PROPERTY_LIST();

	// Types
	typedef WorldContext					Context;

	typedef Reference< Noun >				Ref;
	typedef WidgetReference< Noun >			wRef;
	typedef TargetReference< Noun >			tRef;
	typedef WeakReference< Noun >			WeakRef;

	// Construction
	Noun();
	virtual ~Noun();

	// Widget interface
	bool					read( const InStream & );
	bool					write( const OutStream & );

	// BaseNode interface
	virtual BoxHull			hull() const;
	virtual float			radius() const;					// radius of this object
	virtual float			radius2() const;				// radius squared

	virtual void			onAttached();
	virtual void			onDetaching();

	virtual void			preRender( RenderContext & context,
								const Matrix33 & frame,
								const Vector3 & position );
	virtual void			render( RenderContext &context, 
								const Matrix33 & frame, 
								const Vector3 & position );

	// NodeTransform interface
	virtual void			setFrame( const Matrix33 &frame );

	// Accessors
	NounContext *			nounContext( bool a_bBlocking = true ) const;			// get the noun context
	dword					tick() const;					// current noun tick
	dword					flags() const;					// noun flags
	Context *				context() const;				// get this noun WorldContext

	const Euler &			orientation() const;			// get the orientation of this noun (x = pitch, y = yaw, z = roll)

	int						traitCount() const;				// number of traits attached to this noun
	Trait *					trait( int n ) const;			// get trait n

	bool					syncronized() const;			// has the noun been syncronized with others
	bool					transfer() const;				// true if the noun is being moved from one zone to another
	bool					detach() const;					// is this noun flagged to be detached from the zone

	bool					isRootNoun() const;				// is this noun's parent the zone
	NodeZone *				zone() const;					// get our current zone
	Noun *					zoneParent() const;				// who is our zone parent
	const Vector3 &			zonePosition() const;			// get our position relative to our current zone
	Vector3					zonePosition( NodeZone * pZone ) const;	// get our position relative to the given zone
	bool					inLockedZone() const;			// is the zone this noun is in locked

	bool					isParentNoun() const;			// is our parent a noun object
	bool					isParent( Noun * pNoun ) const;	// is the noun our parent
	Noun *					parentNoun() const;				// get our parent noun, may be NULL

	bool					isWorldValid() const;			// returns true if world position is currently calculated
	void					invalidateWorld() const;		// set the world calculations as invalid..
	virtual void			calculateWorld() const;			// calculates the current world position and frame
	const Vector3 &			worldPosition() const;			// get the world position
	const Matrix33 &		worldFrame() const;				// get the world frame
	const BoxHull &			worldHull() const;				// get the bounding box in the world
	const Vector3 &			velocity() const;				// noun base velocity
	const Vector3 &			worldVelocity() const;			// get the velocity of this object

	void *					collisionContext() const;	

	int						teamId() const;					// team ID of this Noun
	int						userId() const;					// user ID of the player, 0 if none
	bool					isLocal() const;				// returns true if noun is controlled locally
	bool					isRemote() const;				// returns true if noun is under remote control

	virtual CharString		displayName( bool a_bAllowTags ) const;	// return the displayable name for this Noun, by default this is the BaseNode::name()..
	virtual NounType		nounType() const;				// what type of noun is this
	virtual bool			collidable() const;				// can this object collide
	virtual bool			canCollide() const;				// can this object collide with other collidable() objects
	const Vector3 &			lastPosition() const;			// last known position of the noun

	qword					profileTime() const;			// amount of CPU used to update this object

	// Mutators
	virtual bool			postInitialize();				// this is invoked after the object has been attached into the world, any asset loading or additional initialization 

	void					setNounContext( NounContext::Link pContext );
	void					setTick( dword tick, bool bRemoveVerbs = true );	// set current noun tick, if bRemoveVerbs is true, any attached verbs occuring before the tick are removed
	void					setFlags( dword flags );
	void					setContext( Context * pContext );
	void					setZone( NodeZone * pZone );

	void					setOrientation( const Euler & a_vOrientation );

	int						attachTrait( Trait * pTrait );	// add trait to this noun, returns the index of the trait
	void					detachTrait( int n );			// remove a trait from this noun
	void					detachAllTraits();

	void					setSyncronized( bool bSyncronized );
	void					setTransfer( bool bTransfer );

	void					newKey( bool recurse = true );	// take a new widget key

	bool					attachVerb( Verb * pVerb );
	void					detachVerbs();
	void					releaseVerbs();

	void					setWorldPosition( 
								const Vector3 & position );
	void					setWorldFrame(
								const Matrix33 & frame );
	void					setVelocity( 
								const Vector3 & velocity );

	void					setLastPosition( 
								const Vector3 & position );	// set the last position, used by collision detection / BSP
	void					setCollisionContext(
								void * pContext );

	void					setTeamId( int nTeamId );
	void					setUserId( int userId );

	// Noun Interface
	virtual void			detachSelf();						// detach ourselves from the worldContext
	virtual bool			canAttachVerb( Verb * pVerb );
	virtual void			onAttachVerb( Verb * pVerb );
	virtual void			onDetachVerb( Verb * pVerb );
	virtual void			onTransferZones( NodeZone * pNewZone, NodeZone * pOldZone );

	virtual void			onMovement( dword nTick, 
								NodeZone * pZone, 
								const Vector3 & vPosition, 
								const Euler & vOrientation, 
								bool bGlobal );

	virtual void			setup();						// called after noun is created, called only once 
	virtual void			initialize();					// called before a noun is attached to a zone
	virtual void			release();						// called after a noun is detached from a zone, it will never be used again after this is called
	virtual void			simulate( dword nTick );		// called to simulate the noun
	virtual void			update();						// post simulation update.. called after all nouns have been simulated and collisions have been detected.

	virtual void			collision( Noun * pCollide,
								const Vector3 & intersect );	// handle collision between two colliable objects, intersect position is in world space
	virtual void			onWorldTransform( 
								const Vector3 & transform );	// called when zone positions are changed
																// nouns should update any internal world positions by adding the vectors
	virtual bool			intersect( const Vector3 & from, 
								const Vector3 & ray, 
								Vector3 & intersect );

	// Helpers
	bool					isServer() const;
	bool					isSleeping() const;
	bool					isClient() const;

	void					resync();						// calls WorldContext::User::onUpdate(), which normally forces the server to resend the object out to all clients.

	void					privateChat( const char * pChat );		// sends chat to nouns owner, even if nouns owner is remote
	void					teamChat( const char * pChat );			// sends chat to the nouns team
	void					factionChat( const char * pChat );		// sends chat to the nouns faction
	void					globalChat( const char * pChat );		// sends chat to all clients
	void					localChat( const char * pChat );		// sends chat to all clients in the local area

	void					clientChat( const char * pChat );		// calls WorldContext::User::onChat()

	bool					intersect( float r, 			// this intersect code gives the ray a radius, used for fat objects
								const Vector3 & from,	
								const Vector3 & ray, 
								Vector3 & intersect ) const;

	int						factionId() const;				// get the factionId of this object, if none then 0
	const char *			teamName() const;				// get the team name of this object if any

	bool					isFriend( const Noun * pWhom ) const;
	bool					isEnemy( const Noun * pWhom ) const;
	bool					isTeam( const Noun * pWhom ) const;		

	void					setDetach( bool bDetach = true);	// set the flag for detach

	dword					setFlags( dword flags, bool state );
	dword					clearFlags( dword flags );
	dword					addFlags( dword flags );
	bool					testFlags( dword test ) const;
	

	Trait *					findTrait( const ClassKey & nKey ) const;
	
	template<typename T>
	T *						attachTrait();					// this function will create or return an existing trait of the given type..
	template<typename T>
	T *						findTrait() const;
	template<typename T>
	bool					detachTrait();

	bool					detachTrait( Trait * pTrait );
	bool					detachTrait( const ClassKey & nkey );

	dword					seed( dword nRange = 0) const;	// get a 32-bit seed based on this objects WidgetKey

	float					mass() const;					// mass of this object
	Material *				icon() const;					// get noun icon
	
	// static
	static bool				validateNoun( const Noun * pNoun );		// returns true if the other noun is attached to a zone
	static Noun *			readTarget( const InStream & input );
	static bool				writeTarget( const OutStream & output, Noun * pTarget );
	
	//! This function detects a intersection of a ray with a sphere, it returns true if they intersect.
	//! origin = center of the sphere, radius = radius of the sphere, from = origin of the ray, ray = direction of the ray
	//! intersect = the intersection point.
	static bool				intersect( const Vector3 & origin, float radius,
								const Vector3 & from, const Vector3 & ray, 
								Vector3 & intersect );
	static NounContext		DEFAULT_CONTEXT;				// if no valid NounContect as been attached, this object is returned	


protected:
	//! Types
	typedef std::list< Verb::Ref >		VerbList;
	typedef std::vector< Trait::Ref >	TraitList;

	// Data
	NounContext::Link		m_NounContext;					// static/shared data
	dword					m_Flags;						// noun flags
	Euler					m_vOrientation;					// X = Pitch, Y = Yaw, Z = Roll
	TraitList				m_Traits;						// noun traits
	VerbList				m_Verbs;						// queued verbs
	int						m_nTeamId;
	int						m_nUserId;

	// non-serialized
	dword					m_Tick;							// current noun tick
	WeakReference<Context>	m_pContext;						// noun context
	WeakReference<NodeZone> m_pZone;						// noun zone
	bool					m_bInitialized;					// true if initialize has been called on this noun or not..
	bool					m_bPostInit;					// true if post initialize has completed..
	bool					m_bSyncronized;					// has the noun been syncronized with others
	bool					m_bTransfer;					// is the noun being transfered from one zone to another
	bool					m_bDetach;						// is the noun flagged for detach

	BoxHull					m_Hull;
	float					m_fRadius;
	float					m_fRadius2;
	Vector3					m_LastPosition;					// last updated position of the noun
	void *					m_pCollisionContext;			// a void pointer for the collision hash to use
	Vector3					m_vVelocity;					// velocity of this object relative to it's parent

	mutable dword			m_nWorldTick;					// tick when world position was last updated
	mutable Vector3			m_vZonePosition;				// position relative to our current zone
	mutable Vector3			m_WorldPosition;				// position in the world
	mutable Matrix33		m_WorldFrame;					// world rotation
	mutable BoxHull			m_WorldHull;
	mutable Vector3			m_vWorldVelocity;				
};

//----------------------------------------------------------------------------

inline BoxHull Noun::hull() const
{
	return m_Hull;
}

inline float Noun::radius() const
{
	return m_fRadius;
}

inline float Noun::radius2() const
{
	return m_fRadius2;
}

//----------------------------------------------------------------------------

inline NounContext * Noun::nounContext( bool a_bBlocking /*= true*/ ) const
{
	if ( m_NounContext.valid( true, a_bBlocking ) )
		return m_NounContext;

	return &DEFAULT_CONTEXT;
}

inline dword Noun::tick() const
{
	return m_Tick;
}

inline dword Noun::flags() const
{
	return m_Flags;
}

inline Noun::Context * Noun::context() const
{
	return m_pContext;
}

inline const Euler & Noun::orientation() const
{
	return m_vOrientation;
}

inline int Noun::traitCount() const
{
	return m_Traits.size();
}

inline Trait * Noun::trait( int n ) const
{
	return m_Traits[ n ];
}

inline bool Noun::syncronized() const
{
	return m_bSyncronized;
}

inline bool Noun::transfer() const
{
	return m_bTransfer;
}

inline bool Noun::detach() const
{
	return m_bDetach;
}

inline NodeZone * Noun::zone() const
{
	return m_pZone;
}

inline const Vector3 & Noun::zonePosition() const
{
	if (! isWorldValid() )
		calculateWorld();
	return m_vZonePosition;
}

inline bool Noun::isWorldValid() const
{
	return m_Tick == m_nWorldTick;
}

inline const Vector3 & Noun::worldPosition() const
{
	if (! isWorldValid() )
		calculateWorld();
	return m_WorldPosition;
}

inline const Matrix33 & Noun::worldFrame() const
{
	if (! isWorldValid() )
		calculateWorld();
	return m_WorldFrame;
}

inline const Vector3 & Noun::lastPosition() const
{
	return m_LastPosition;
}

inline void * Noun::collisionContext() const
{
	return m_pCollisionContext;
}

//----------------------------------------------------------------------------

inline void Noun::setNounContext( NounContext::Link pContext )
{
	m_NounContext = pContext;
}

inline void Noun::setFlags( dword flags )
{
	m_Flags = flags;
}

inline void Noun::setContext( Context * pContext )
{
	m_pContext = pContext;
}

inline void Noun::setZone( NodeZone * pZone )
{
	m_pZone = pZone;
}

//----------------------------------------------------------------------------

inline dword Noun::setFlags( dword flags, bool state )
{
	if ( state )
		return m_Flags |= flags;		// OR flags
	else
		return m_Flags &= ~flags;		// NOT flags
}

inline dword Noun::clearFlags( dword flags )
{
	return m_Flags &= ~flags;
}

inline dword Noun::addFlags( dword flags )
{
	return m_Flags |= flags;
}

inline bool Noun::testFlags( dword test ) const
{
	return (m_Flags & test) != 0;
}

inline Trait * Noun::findTrait( const ClassKey & nKey ) const
{
	for(size_t i=0;i<m_Traits.size();i++)
	{
		Trait * pTrait = m_Traits[i];
		if ( pTrait != NULL && pTrait->factory()->classKey() == nKey )
			return m_Traits[i];
	}

	return NULL;
}

template<typename T>
inline T * Noun::attachTrait()
{
	T * pTrait = findTrait<T>();
	if ( pTrait == NULL )
	{
		pTrait = new T();
		attachTrait( pTrait );
	}

	return pTrait;
}

template<typename T>
inline T * Noun::findTrait() const
{
	return (T *)findTrait( T::classKey() );
}

template<typename T>
inline bool Noun::detachTrait()
{
	return detachTrait( T::classKey() );
}

inline bool Noun::detachTrait( Trait * pTrait )
{
	if (! pTrait )
		return false;

	for(size_t i=0;i<m_Traits.size();i++)
		if ( m_Traits[i].pointer() == pTrait )
		{
			detachTrait( i );
			return true;
		}

	return false;
}

inline bool Noun::detachTrait( const ClassKey & nKey )
{
	for(size_t i=0;i<m_Traits.size();i++)
		if ( m_Traits[i]->factory()->classKey() == nKey )
		{
			detachTrait( i );
			return true;
		}

	return false;
}

inline dword Noun::seed( dword nRange /*= 0*/ ) const
{
	dword nSeed = ((dword)key()) + tick();
	if ( nRange > 0 )
		nSeed %= nRange;
	return nSeed;
}

inline float Noun::mass() const
{
	return nounContext()->mass();
}

//----------------------------------------------------------------------------

template<class T>
inline T * FindTrait( const Noun * pTarget )
{
	return (T *)pTarget->findTrait( T::classKey() );
}

template<class T>
inline T * GetTrait( Noun * pTarget )
{
	T * pTrait = FindTrait<T>( pTarget );
	if (! pTrait )
	{
		// add the trait if not found...
		pTrait = new T();
		pTarget->attachTrait( pTrait );
	}

	return pTrait;
}

// find a node by class
template<class T>
inline T * FindNode( const BaseNode * pNode )
{
	if ( WidgetCast<T>( pNode ) != NULL )
		return (T *)pNode;
	
	for(int i=0;i<pNode->childCount();i++)
	{
		BaseNode * pChild = FindNode<T>( pNode->child(i) );
		if ( pChild != NULL )
			return (T *)pChild;
	}

	return NULL;
}

//-------------------------------------------------------------------------------



#endif

//----------------------------------------------------------------------------
// EOF
