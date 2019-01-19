/*
	WorldContext.h

	This object is used to hold the the simulation of the World, 
	it is used by the WorldClient and WorldServer classes.

	Basically, the simulation of the entire game is managed by this object. The user of this object
	provides a WorldContext::User object to get callbacks on certain events occuring 
	in the world. 

	It's the job of the user of this object to keep the other WorldContext objects accross the 
	network in sync with each other.

	The world is divided into zones, which represent a BoxHull area of the Universe. A zone
	is locked or unlocked by the user of this object to control which areas are being updated when
	update() is called. Zones also allow for a very large world, since all objects positions in each
	zone are relative to the center of the zone.

	Proxy worlds can be attached onto a WorldContext. This allows the client to render other worlds and 
	even view basic data about those worlds. 

	(c)2003 Palestar Development, Richard Lyle
*/

#ifndef WORLD_CONTEXT_H
#define WORLD_CONTEXT_H

#include "Standard/List.h"
#include "Standard/Queue.h"
#include "Resource/Text.h"
#include "Resource/Collection.h"

#include "Constants.h"
#include "NounTemplate.h"
#include "NodeZone.h"
#include "WorldTimer.h"
#include "WorldDll.h"

//----------------------------------------------------------------------------

// forward declare
typedef struct lua_State lua_State;
typedef struct lua_Debug lua_Debug;

//----------------------------------------------------------------------------

class DLL WorldContext : public Resource
{
public:
	DECLARE_WIDGET_CLASS();
	DECLARE_PROPERTY_LIST();

	// Types
	typedef NodeZone::NounCollision			NounCollision;
	typedef ResourceLink< WorldContext >	Link;
	typedef Reference< WorldContext >		Ref;
	typedef WeakReference< WorldContext >	WeakRef;
	typedef WidgetReference< WorldContext >	wRef;

	enum LogLevels {

		LOG_HIGH	= 0,		// very important messages, always logged
		LOG_NORMAL	= 1,		// normal messages
		LOG_LOW		= 2,		// low level messages, not necessary
		LOG_DEBUG	= 3,		// debug level messages, for debugging purposes
	};

	enum FilterFlags
	{
		FF_CLIENT	= 0x20,			// use this filter when sending the WorldContext to a client
	};
	enum TeamFlags
	{
		TF_NOJOIN	= 0x1,			// team is full, no joining is allowed
		TF_NOSPAWN	= 0x2,			// no AI controlled units allowed to be created
		TF_AI		= 0x4,			// AI is enabled for this team
	};
	struct Objective
	{
		Objective();
		Objective( const Objective & copy );

		CharString			description;	// complete description of objective
		int					factionId;		// team mask
		int					type;			// type of objective
		WidgetKey			noun;			// object of this objective

		Objective &			operator=( const Objective & copy );
	};
	struct Condition						// end condition
	{
		Condition();
		Condition( const Condition & copy );

		int					conditionId;	// id for this condition
		CharString			script;			// script executed when condition is reached
		CharString			message;		// message sent to all players when this condition meet
		WidgetKey			next;			// next WorldContext to load, NULL_WIDGET if none
		Array< Objective >	objectives;		// objectives to activate this condition
		bool				bActive;		// set to true once this has been triggered

		Condition &			operator=( const Condition & copy );
	};
	struct Team
	{
		Team();
		Team( const Team & copy );
		Team &				operator=( const Team & copy );

		int					nTeamId;
		dword				clanId;
		int					factionId;
		dword				flags;
		CharString			name;
		CharString			longName;
		CharString			about;
		Collection::Link	pTemplates;
	};
	struct ScriptContext
	{
		UniqueNumber		nScriptId;		// scriptId
		CharString			sName;			// script name
		lua_State *			pScript;		// LUA state object

		ScriptContext &		operator=(const ScriptContext & copy );
	};

	//! Inhert from this class and implement the registerFunctions() to hook into the script engine your own functions
	class DLL ScriptHook 
	{
	public:
		typedef std::list< ScriptHook * >		HookList;

		static HookList & GetHookList()
		{
			static HookList * p = new HookList();
			return *p;
		}

		//! Construction
		ScriptHook()
		{
			GetHookList().push_back( this );
		}
		virtual ~ScriptHook()
		{
			GetHookList().remove( this );
		}

		virtual void	registerFunctions( lua_State * pScript ) = 0;
	};

	// the user of this object must inherit from this interface and provide a pointer to this object to capture events from
	// this WorldContext object.
	class DLL User		
	{
	public:
		//! Construction
		virtual ~User()
		{
			// if this user is destroyed, make sure the pointer in WorldContext is set to NULL.
			if ( m_wpContext.valid() && m_wpContext->user() == this )
				m_wpContext->setUser( NULL );
		}

		virtual bool	isServer() const;								// is the user a server, returning false makes this object a client
		virtual bool	isSleeping() const;								// if true, then the context is in sleep mode and should use as little CPU as possible
		virtual dword	serverRole() const;								// get the server role bits
		virtual dword	userId() const;									// get the userId of the local player
		virtual int		maxPlayers() const;								// get the maximum number of players in this world
		virtual int		playerCount() const;							// get the current number of players in this world
		virtual int		spawnedScore( int a_nFactionId );				// returns a value representing the number of spawns for the given faction
		virtual int		maxPlayerScoreAI();

		virtual void	onChat( Noun * pNoun, const char * pMessage );

		virtual bool	canAttachVerb( Noun * pTarget, Verb * pVerb );
		virtual void	onAttachVerb( Verb * pVerb );					// called when a verb is attached to a noun/context

		virtual void	onAddCondition( const Condition & condition );
		virtual void	onDeleteCondition( int conditionId );
		virtual void	onCondition( int conditionId );					// called when a condition is true

		virtual void	onAddTeam( const Team & team );
		virtual void	onDeleteTeam( int nTeamId );

		virtual void	onAddWorld( WorldContext * pWorld );
		virtual void	onDeleteWorld( WorldContext * pWorld );
		virtual void	onAddZone( NodeZone * pZone );
		virtual void	onDeleteZone( NodeZone * pZone );
		virtual void	onAttachNoun( Noun * pNoun );
		virtual void	onUpdateNoun( Noun * pNoun );
		virtual void	onDetachNoun( Noun * pNoun );
		virtual void	onTransferNoun( Noun * pNoun, 
							NodeZone * pNewZone,
							NodeZone * pOldZone );
		virtual void	onWorldTransform( const Vector3 & transform );

		virtual void	onServerTransfer( Noun * pWho,
							const char * a_pAddress, int a_nPort, 
							const char * a_pSpawnPoint );
		virtual bool	saveStorage(Noun *a_pSelf);

	private:
		//! Data
		WeakRef			m_wpContext;

		friend class WorldContext;
	};

	//! Configuration
	static bool					sm_bEnableHDR;
	static bool					sm_bUpdateHDR;
	static bool					sm_bEnableShadows;
	static int					sm_nMaxShadowLights;

	// Constructions
	WorldContext();
	virtual	~WorldContext();
	
	// Widget interface
	bool						read( const InStream & input );
	bool						write( const OutStream & output );
	// Accessors
	User *						user() const;									// get the event handler
	bool						isServer() const;								// is this the server
	bool						isSleeping() const;								// is our context sleeping
	dword						serverRole() const;								// get server role bits
	bool						isClient() const;								// is this the client
	dword						tick() const;									// current tick

	int							type() const;									// type of world context
	const char *				name() const;									// name of this worldContext (scenerio)
	const char *				description() const;							// worldContext description 
	const char *				script() const;									// the script
	const BoxHull &				worldExtents() const;							// the worldContext extents
	const Vector3 &				position() const;								
	const char *				address() const;								// server information for this context
	u16							port() const;

	WorldContext *				parent() const;									// our parent WorldContext if added to another WorldContext
	int							worldCount() const;								// number of other worlds attached to this world
	WorldContext *				world( int n ) const;							// get world n
	WorldContext *				findWorld( const WidgetKey & nKey ) const;		// find a world by it's ID
	int							zoneCount() const;								// how many zones are in this context
	NodeZone *					zone( int n ) const;							// get zone n

	NodeZone *					activeZone() const;								// what zone is currently the center of the Universe
	const Vector3 &				zoneTransform() const;							// current zone transform

	int							conditionCount() const;
	int							conditionId( int n ) const;
	const Condition &			conditionN( int n ) const;
	const Condition &			condition( int condtionId ) const;
	bool						isConditionValid( int conditionId ) const;
	dword						timeLimit() const;								// time limit in ticks for this context, 0 if none

	int							teamCount() const;								// number of fleets
	int							teamId( int n ) const;							// get team ID n
	const Team &				teamN( int n ) const;
	const Team &				team( int nTeamId ) const;						// get team by their id
	int							teamFactionId( int nTeamId ) const;
	bool						isTeamValid( int nTeamId ) const;				// check that team is valid

	int							scriptCount() const;							// how many scripts are currently running
	const ScriptContext &		script( int n ) const;							// get script n

	int							lockedZoneCount() const;						// how many locked zones
	NodeZone *					lockedZone( int n ) const;						// get locked zone n

	// Mutators
	virtual void				onAttachNoun( Noun * pNoun );
	virtual void				onUpdateNoun( Noun * pNoun );
	virtual void				onDetachNoun( Noun * pNoun );
	virtual void				onTransferNoun( Noun * pNoun, 
									NodeZone * pNewZone,
									NodeZone * pOldZone );
	virtual void				onWorldTransform( const Vector3 & vAdd );		// move the worldContext
	virtual bool				onMergeUpdate( Noun * a_pUpdate, Noun * a_pTemplate );
	virtual bool				onMergeAttach( Noun * a_pAttach );
	virtual bool				onMergeRemove( Noun * a_pDetach );

	virtual bool				start();										// start this context, should be called before update()
	virtual bool				stop();											// stop/pause this context
	virtual bool				update();										// update all locked zones
	virtual void				updateSecond();									// called by update() every one second

	virtual bool				merge( WorldContext * a_pContext );				// merge another world context into this one..

	virtual void				render( RenderContext & context,				// render context
									const Matrix33 & frame,						// frame of the camera within the current zone
									const Vector3 & position,					// position of the camera within the current zone
									bool bProxy = false );						

	void						cron( dword nTick );							// syncronize the current tick from an outside source (i.e. the server)
	bool						lockZone( NodeZone * pZone, 
									bool setTick = true );						// lock the specified zone so it will get updated
	bool						unlockZone( NodeZone * pZone );					// remove a zone from the locked list, it will no longer get updated

	void						setUser( User * pUser );						// set the user
	void						setServer( bool isServer );						// set the server flag

	void						setTick( dword tick );
	void						setType( int nType );
	void						setName( const char * name );
	void						setDescription( const char * description );
	void						setScript( const char * pScript );
	void						calculateWorldExtents();						// update the worldContext box hull
	void						setPosition( const Vector3 & a_vPosition );
	void						setAddress( const char * pAddress, u16 nPort );
	void						clearAddress();

	int							addWorld( WorldContext * pContext );			// attach another context onto this context
	bool						deleteWorld( WorldContext * pContext );			// remove a conext from this context
	void						clearWorlds();									// remove all worlds form this context

	virtual NodeZone *			createZone();
	bool						createZones( const BoxHull & world, 
									Scene * a_pScenery );
	bool						mergeZones( WorldContext * a_pContext );
	bool						addZone( NodeZone * pZone );					// add a new zone
	bool						deleteZone( NodeZone * pZone );					// returns true if zone was deleted

	bool						setActiveZone( const Vector3 & worldPosition );	// set the active zone based on the current world position of the viewer
	void						setActiveZone( NodeZone * pZone );				// set the active zone

	bool						attachNoun( Noun * pNoun,						// if parent is null, position of noun should be set in world space
									BaseNode * pParent = NULL );
	bool						replaceNoun( Noun * pReplace, Noun * pWith );
	void						detachNoun( Noun * pNoun );

	bool						canAttachVerb( Noun * pNoun, Verb * pVerb );
	void						onAttachVerb( Verb * pVerb );

	bool						addCondition( Condition & condition );
	bool						deleteCondition( int conditionId );
	bool						deleteAllConditions();
	void						setTimeLimit( dword limit );					// set the time limit in ticks, 0 for no time limit

	int							checkConditions();								// check conditions
	void						executeCondition( int conditionId );			// condition is true
	virtual bool				checkObjective( const Objective & objective );	// returns true if the specific object is completed

	bool						addTeam( Team & team );
	void						setTeamFlags( int nTeamId, dword nFlags );
	bool						deleteTeam( int nTeamId );

	void						pushChat( const char * pChat );					// push chat message

	bool						canJoin( int nTeamId );
	bool						canSpawn( int nTeamId );

	bool						proximityCheck( const Vector3 & vOrigin, 
									float fRadius, Array< NounCollision > & nodes );	// returns a list of nouns within the specified sphere from all zones
	bool						proximityCheck( const Vector3 & vOrigin, 
									float fRadius, Array< NounCollision > & nodes,
									const ClassKey & nType );	
	bool						proximityCheckLocked( const Vector3 & vOrigin, 
									float fRadius, Array< NounCollision > & nodes );	// proximity check of locked zones only

	Widget *					getWidget( const ClassKey & a_nClass, bool a_bCreate = true );
	
	//! Grab a widget that is attached to this context by it's type. These widgets are used to hold
	//! data generically attached to a WorldContext.
	template<typename T>
	T *							getWidget( bool a_bCreate = true )
	{
		return static_cast<T *>( getWidget( T::classKey(), a_bCreate ) );
	}

	// Helpers
	int							teamClan( dword nClanId, int nFactionId );		// get the nTeamId for the specified clan/faction
	int							teamFaction( int nTeamId );						// get the faction for a team
	int							factionTeam( int factionId );					// get the first available team for this faction, 0 if none

	bool						findNouns( Array<Noun::Ref> & a_Nouns,			// returns all nouns that match the given class and/or team ID
									ClassKey a_nKey = NULL_CLASS,
									int a_nTeamID = -1,
									int a_nFactionID = -1,
									bool a_bRecursive = false );			
	Noun *						findNoun( const char * pName );					// find a noun, use by the script engine
	Noun *						findNounByKey( const WidgetKey & a_nKey );		// find a noun by it's key 
	NounTemplate *				findNounTemplate( Noun * pNoun );				// find the NounTemplate object for the provided object
	NodeZone *					findZone( const Vector3 & vWorldPosition );		// find the zone containing the provided world position

	// Static
	static bool					saveContext( const char * pFileName, WorldContext * pContext );
	static WorldContext *		loadContext( const char * pFileName );
	static User *				nullUser();										// return the NULL user object, which isn't actually NULL..

	// WorldContextScript.cpp
	virtual void				registerFunctions( lua_State * pScript );		// register functions with a running script

	bool						scriptCall( const char * pCall );				// global call into all running scripts
	bool						scriptCall( UniqueNumber nScriptId, 
									const char * pCall, 
									bool bAllowNewContext = false );			// call function inside a running script, returns true if call was made
	bool						runScript( UniqueNumber nScriptId, 
									const char * pName,
									const char * pCall );						// runs the specified script, returns the scriptId

	bool						killScript( UniqueNumber nScriptId );			// deletes the specified script, returns true if the script is killed

	bool						findScriptId( lua_State * pScript, 
									UniqueNumber & nScriptId );

	CharString					scriptStatus();									// returns a string containing the status of all running scripts

	bool						isScriptActive( lua_State * pScript );		
	void						updateScripts();
	void						releaseScripts();

	// WorldContextScript.cpp
	static const char *			loadChunk( lua_State * pScript, void * ud, size_t * size );
	static int					DoString( lua_State * pScript, CharString sText );
	static int					DoCall( lua_State * pScript, CharString sFunc, Array<CharString> & arArguments );
	static CharString			TranslateLuaError( int error );
	static WorldContext *		GetContext( lua_State * pScript );
	static Noun *				GetNoun( lua_State * pScript, const char * pId );
	static CharString			dumpLocals( lua_State * pScript, lua_Debug * ar, int level );
	static void					Alert( lua_State * pScript, const char * pMessage, const char * pDetails = NULL );
	static void					ResetExecutionDepth( lua_State * pScript );
	static int					GetExecutionDepth( lua_State * pScript );
	static void					IncreaseExecutionDepth( lua_State * pScript );
	static void					DecreaseExecutionDepth( lua_State * pScript );
	static void					LuaHook( lua_State * pScript, lua_Debug * ar );

	static int					ScriptAlert( lua_State * pScript );
	static int					enableAlerts( lua_State * pScript );
	static int					enableTrace( lua_State * pScript );
	static int					randomNumber( lua_State * pScript );
	static int					isClient( lua_State * pScript );
	static int					isServer( lua_State * pScript );
	static int					teamCount( lua_State * pScript );
	static int					teamId( lua_State * pScript );
	static int					getTeamFaction( lua_State * pScript );
	static int					getTeamName( lua_State * pScript );
	static int					getName( lua_State * pScript );
	static int					getKey( lua_State * pScript );
	static int					getClass( lua_State * pScript );
	static int					nodeCount( lua_State * pScript );
	static int					getNode( lua_State * pScript );
	static int					isNoun( lua_State * pScript );		// isNoun( name );
	static int					getTeamId( lua_State * pScript );
	static int					setTeamId( lua_State * pScript );
	static int					isFaction( lua_State * pScript );		// isFaction( name, faction );
	static int					getFactionTeam( lua_State * pScript );
	static Verb::Scope			getVerbScope( int nNumber );
	static int					remoteCall( lua_State * pScript );	// remoteCall( target, scope, call );
	static int					getDistance( lua_State * pScript );	// getDistance( obj1, obj2 );
	static int					getFaction( lua_State * pScript );
	static int					createSpawn( lua_State * pScript );
	static int					getPosition( lua_State * pScript );	// getPosition( object );
	static int					setPosition( lua_State * pScript );	// setPosition( object, x, y, z );
	static int					spawnNoun( lua_State * pScript );
	static int					detachNoun( lua_State * pScript );
	static int					getRadius( lua_State * pScript );
	static int					getUserId( lua_State * pScript );
	static int					getAllNouns( lua_State * pScript );	// getAllNouns( nounId, range ); returns a table 1..n
	static int					getAllNounsRelative( lua_State * pScript );	// getAllNounsRelative( shipId, range ); returns a table 1..n
	static int					getConstants( lua_State * pScript );	// getConstants(); returns a hashtable( name, value );

	static int					listProperties( lua_State * pScript );
	static int					setProperty( lua_State * pScript );
	static int					getProperty( lua_State * pScript );

	static int					startTimer( lua_State * pScript );
	static int					flushChat( lua_State * pScript );
	static int					pushChat( lua_State * pScript );
	static int					sendChat( lua_State * pScript );
	static int					trapKey( lua_State * pScript );
	static int					watchKey( lua_State * pScript );

protected:
	// Types
	typedef Array< NodeZone::Ref >						ZoneArray;
	typedef HashArray< WidgetKey, NodeZone::Ref >		ZoneHash;
	typedef HashArray< int, Team >						TeamHash;
	typedef HashArray< dword, Condition >				ConditionHash;
	typedef Array< WorldContext::Ref >					WorldArray;
	typedef std::map< qword, Widget::Ref >				WidgetMap;

	// Data
	dword						m_Tick;						// current tick
	int							m_nType;					// WorldContext type
	CharString					m_Name;						// worldContext name
	CharString					m_Description;				// worldContext description
	CharString					m_sScript;					// script text 
	BoxHull						m_WorldExtents;				// the world extents
	Vector3						m_vPosition;				// 3D center position of this context
	CharString					m_sAddress;					// server information
	u16							m_nPort;

	WeakRef						m_pParent;					// parent context
	WorldArray					m_Worlds;					// other worlds
	ZoneArray					m_ZoneArray;				// regions of this world
	NodeZone::wRef				m_ActiveZone;				// current active zone
	Vector3						m_ZoneTransform;			// current zone transform

	int							m_NextConditionId;
	ConditionHash				m_Conditions;
	dword						m_TimeLimit;				// time limit
	TeamHash					m_Teams;

	// non-serialize
	User *						m_pUser;					// the event client for this object
	WorldTimer					m_Timer;					// timer object
	dword						m_TickSecond;				// last time second tick occured

	Array< NodeZone::wRef >		m_LockedZones;				// currently locked zones
	int							m_nNextTeamId;
	Array< ScriptContext >		m_Scripts;					// currently running scripts

	Queue< Verb::Ref >			m_Verbs;					// queued verbs

	DisplayEffect::Ref			m_pHDR;						// HDR effect
	WidgetMap					m_WidgetMap;				// generic widget storage

	// deprecated
	ZoneHash					m_Zones;

	void						initializeZones();
	void						renderZones( RenderContext & context, const Matrix33 & frame, const Vector3 & translate );
	void						initializeNounTemplates();
	static int					loadLuaLibs( lua_State * pScript );		// Load all needed libs from lua
	static void                 ActivateJITOpt( lua_State * pScript );

};

//----------------------------------------------------------------------------

inline WorldContext::User * WorldContext::user() const
{
	return m_pUser;
}

inline bool WorldContext::isServer() const
{
	return m_pUser->isServer();
}

inline bool WorldContext::isSleeping() const
{
	return m_pUser->isSleeping();
}

inline dword WorldContext::serverRole() const
{
	return m_pUser->serverRole();
}

inline bool WorldContext::isClient() const
{
	return !m_pUser->isServer();
}

inline dword WorldContext::tick() const
{
	return( m_Tick );
}

inline int WorldContext::type() const
{
	return m_nType;
}

inline const char * WorldContext::name() const
{
	return m_Name;
}

inline const char * WorldContext::description() const
{
	return m_Description;
}

inline const char * WorldContext::script() const
{
	return m_sScript;
}

inline const BoxHull & WorldContext::worldExtents() const
{
	return m_WorldExtents;
}

inline const Vector3 & WorldContext::position() const
{
	return m_vPosition;
}

inline const char * WorldContext::address() const
{
	return m_sAddress;
}

inline u16 WorldContext::port() const
{
	return m_nPort;
}

inline WorldContext * WorldContext::parent() const
{
	return m_pParent;
}

inline int WorldContext::worldCount() const
{
	return m_Worlds.size();
}

inline WorldContext * WorldContext::world( int n ) const
{
	return m_Worlds[ n ];
}

inline WorldContext * WorldContext::findWorld( const WidgetKey & nKey ) const
{
	for(int i=0;i<m_Worlds.size();++i)
		if ( m_Worlds[i]->key() == nKey )
			return m_Worlds[i];
	return NULL;
}

inline int WorldContext::zoneCount() const
{
	return m_ZoneArray.size();
}

inline NodeZone * WorldContext::zone( int n ) const
{
	return m_ZoneArray[ n ];
}

inline NodeZone * WorldContext::activeZone() const
{
	return m_ActiveZone;
}

inline const Vector3 & WorldContext::zoneTransform() const
{
	return m_ZoneTransform;
}

inline int WorldContext::conditionCount() const
{
	return m_Conditions.array().size();
}

inline int WorldContext::conditionId( int n ) const
{
	return m_Conditions.array()[ n ];
}

inline const WorldContext::Condition & WorldContext::conditionN( int n ) const
{
	return m_Conditions[ n ];
}

inline const WorldContext::Condition & WorldContext::condition( int conditionId ) const
{
	return m_Conditions.hash()[ conditionId ];
}

inline bool WorldContext::isConditionValid( int conditionId ) const
{
	return m_Conditions.hash().find( conditionId ).valid();
}

inline dword WorldContext::timeLimit() const
{
	return m_TimeLimit;
}

inline int WorldContext::teamCount() const
{
	return m_Teams.array().size();
}

inline int WorldContext::teamId( int n ) const
{
	return m_Teams.array()[ n ];
}

inline const WorldContext::Team & WorldContext::teamN( int n ) const
{
	return m_Teams[ n ];
}

inline const WorldContext::Team & WorldContext::team( int nTeamId ) const
{
	return m_Teams.hash()[ nTeamId ];
}

inline int WorldContext::teamFactionId( int nTeamId ) const
{
	TeamHash::Iterator iTeam = m_Teams.find( nTeamId );
	if ( iTeam.valid() )
		return (*iTeam).factionId;

	return 0;
}

inline bool WorldContext::isTeamValid( int nTeamId ) const
{
	return m_Teams.hash().find( nTeamId ).valid();
}

inline int WorldContext::scriptCount() const
{
	return m_Scripts.size();
}

inline const WorldContext::ScriptContext & WorldContext::script( int n ) const
{
	return m_Scripts[ n ];
}

inline int WorldContext::lockedZoneCount() const
{
	return m_LockedZones.size();
}

inline NodeZone * WorldContext::lockedZone( int n ) const
{
	return m_LockedZones[ n ];
}

//---------------------------------------------------------------------------------------------------

inline Widget * WorldContext::getWidget( const ClassKey & a_nClass, bool a_bCreate /*= true*/ )
{
	WidgetMap::iterator iWidget = m_WidgetMap.find( a_nClass );
	if ( iWidget != m_WidgetMap.end() )
		return iWidget->second;

	if (! a_bCreate )
		return NULL;

	Widget * pWidget = Factory::createWidget( a_nClass );
	if ( pWidget != NULL )
		m_WidgetMap[ a_nClass ] = pWidget;

	return pWidget;
}


//---------------------------------------------------------------------------------------------------

inline WorldContext::Objective::Objective()
{}

inline WorldContext::Objective::Objective( const Objective & copy )
{
	operator=( copy );
}

inline WorldContext::Objective & WorldContext::Objective::operator=( const Objective & copy )
{
	description = copy.description;
	factionId = copy.factionId;
	type = copy.type;
	noun = copy.noun;
	return *this;
}

inline const InStream & operator>>( const InStream & input, WorldContext::Objective & objective )
{
	input >> objective.description;
	input >> objective.factionId;
	input >> objective.type;
	input >> objective.noun;
	return input;
}

inline const OutStream & operator<<( const OutStream & output, const WorldContext::Objective & objective )
{
	output << objective.description;
	output << objective.factionId;
	output << objective.type;
	output << objective.noun;
	return output;
}

//---------------------------------------------------------------------------------------------------

inline WorldContext::Condition::Condition() : bActive( false )
{}

inline WorldContext::Condition::Condition( const Condition & copy ) : bActive( false )
{
	operator=( copy );
}

inline WorldContext::Condition & WorldContext::Condition::operator=( const Condition & copy )
{
	conditionId = copy.conditionId;
	script = copy.script;
	message = copy.message;
	objectives = copy.objectives;
	next = copy.next;
	return *this;
}

inline const InStream & operator>>( const InStream & input, WorldContext::Condition & condition )
{
	input >> condition.conditionId;
	input >> condition.script;
	input >> condition.message;
	input >> condition.objectives;
	input >> condition.next;
	return input;
}

inline const OutStream & operator<<( const OutStream & output, const WorldContext::Condition & condition )
{
	output << condition.conditionId;
	output << condition.script;
	output << condition.message;
	output << condition.objectives;
	output << condition.next;
	return output;
}

//----------------------------------------------------------------------------


inline WorldContext::Team::Team()
{}

inline WorldContext::Team::Team( const Team & copy )
{
	operator=( copy );
}

inline WorldContext::Team & WorldContext::Team::operator=( const Team & copy )
{
	nTeamId = copy.nTeamId;
	clanId = copy.clanId;
	factionId = copy.factionId;
	flags = copy.flags;
	name = copy.name;
	longName = copy.longName;
	about = copy.about;
	pTemplates = copy.pTemplates;
	return *this;
}

inline const InStream & operator>>( const InStream & input, WorldContext::Team & team )
{
	input >> team.nTeamId;
	input >> team.clanId;
	input >> team.factionId;
	input >> team.flags;
	input >> team.name;
	input >> team.longName;
	input >> team.about;
	input >> team.pTemplates;
	return input;
}

inline const OutStream & operator<<( const OutStream & output, const WorldContext::Team & team )
{
	output << team.nTeamId;
	output << team.clanId;
	output << team.factionId;
	output << team.flags;
	output << team.name;
	output << team.longName;
	output << team.about;
	output << team.pTemplates;
	return output;
}

//----------------------------------------------------------------------------

inline WorldContext::ScriptContext & WorldContext::ScriptContext::operator=(const ScriptContext & copy )
{
	nScriptId = copy.nScriptId;
	sName = copy.sName;
	pScript = copy.pScript;
	return *this;
}

//----------------------------------------------------------------------------



#endif

//----------------------------------------------------------------------------
// EOF
