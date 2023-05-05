/*
	WorldServer.h

	This object is used to route verbs for one or more clients
	(c)1999 Palestar Development, Richard Lyle
*/

#ifndef WORLD_SERVER_H
#define WORLD_SERVER_H

#include "Network/Server.h"
#include "Network/LogClient.h"
#include "Network/ProfilerServer.h"
#include "Network/UDPServer.h"
#include "GCQ/MetaClient.h"
#include "Standard/Event.h"
#include "Standard/SortedArray.h"
#include "Standard/Timer.h"
#include "Factory/WidgetWrap.h"
#include "File/FileFIFO.h"

#include "WorldContext.h"
#include "ProfileUpdateQueue.h"
#include "Profile.h"
#include "WorldClient.h"
#include "UserStorage.h"

#include <set>
#include <map>

#include "WorldDll.h"

//----------------------------------------------------------------------------

class DLL WorldServer : public Widget, public Server, public UDPServer, 
	public WorldContext::User
{
public:
	DECLARE_WIDGET_CLASS();

	// Types
	typedef WorldClient::ServerStatus		ServerStatus;
	typedef WorldClient::ClientStatus		ClientStatus;
	typedef WorldClient::Storage			Storage;
	typedef std::map< WidgetKey, dword >	UpdateHash;
	typedef std::set< Noun::Ref >			NounArray;
	typedef std::set< dword >				ClientArray;

	typedef WorldContext::Team				Team;
	typedef WorldContext::Condition			Condition;

	// configuration structure for the WorldServer to initialize.
	struct Context
	{
		Context();
		Context( const Context & copy );

		CharString				sConfigFile;			// file containing our settings
		dword					nServerFlags;			// flags required to connect as a server
		dword					nClientFlags;			// flags required to be a client
		dword					nModFlags;				// moderator flags

		bool					bPublish;				// do we register with this server on the metaserver
		CharString				sMetaAddress;			// metaserver address
		int						nMetaPort;

		WidgetKey				nUniverseContext;		// worldContext context to load

		int						maxPlayers;				// maximum number of players, should be less than or equal to the number of clients
		int						maxRank;				// maximum rank allowed
		bool					bBalanced;				// keep fleets balanced
		bool					bClan;					// group clan members on the same team
		bool					bTutorial;				// allow players without login

		CharString				user;					// user account to login with
		CharString				password;				// password for user account
		dword					nSessionId;				// our sessionId
		dword					gameId;					// our game id

		CharString				sLogAddress;			// LogServer address
		int						nLogPort;				// LogServer port

		CharString				name;					// name of this server
		CharString				description;			// server description
		CharString				address;				// server address
		int						port;					// server port
		int						maxClients;				// maximum number of clients
		int						nCompression;			// compression level, -1 for default
		dword					nDisconnectTime;		// how many seconds after player disconnects they are removed (default 120)
		CharString				runScript;				// script to run when the server starts

		bool					bPersistant;			// save worldContext to local file, bMaster must be true
		CharString				sSaveFile;				// file to save worldContext data into 
		CharString				sStorage;				// directory to save user storage
		bool					bEditor;				// true if this server is an editor
		bool					bEnableUDP;				// enable UDP

		dword					nServerRole;			// server role bits
		CharString				sMasterAddress;			// address and port of server we are working for if any
		int						nMasterPort;
		
		bool					bUseShipFlags;			// enable bypass of the following ship types
		CharString				sShipFlags;				// list of ship types to bypass spawn checking

		Context &				operator=(const Context & copy );
	};

	struct ClientContext
	{
		ClientContext();
		ClientContext( const ClientContext & copy );

		dword					nClientId;				// client id for this context
		Socket::Ref				pSocket;				// clients socket
		bool					bModerate;				// is client in moderator mode
		bool					bProxy;					// is client a proxy connection
		bool					bServer;				// is server connection

		int						nQueuePosition;			// if > 0 then this client is in the queue to join
		bool					bConnectedUDP;			// true if we have a UDP connection
		dword					nLastUDP;				// last time we received a UDP packet
		dword					nLastFocusTime;			// last time focus was changed
		dword					nLastTime;				// last time rate was checked in ms
		dword					nLastBytesSent;			// last known number of bytes sent 
		dword					nLatency;				// latency to the client
		dword					nLatencyUDP;			// latency to the client by UDP
		int						nPings;					// number of pings currently pending

		dword					nUserId;				// our user id
		MetaClient::Profile		MetaProfile;			// our meta profile
		Profile::Ref			pProfile;				// profile object
		UserStorage::Ref		pStorage;				// persisted data for this user
		int						nFactionId;				// faction for this player
		int						nTeamId;				// team of the player
		Noun::wRef				pFocus;					// the viewers focus position
		float					fFocusArea;				// size of the viewers focus area
		Noun::wRef				pTarget;				// the players current target
		Noun::wRef				pSelf;					// the players object
		NounArray				localNouns;				// what nouns are local to this client
		UpdateHash				updateHash;				// any nouns sent to this client is put into this hash along with their version number..

		dword					nLoginTime;				// what time did the player login
		dword					nLogoffTime;			// when did the players client disconnect
		bool					bServerTransfer;		// true if client is being transfered to another server

		ClientContext &			operator=( const ClientContext & copy );
	};

	// servers embedded into this server, their port is offset from the primary port
	enum {
		PROFILER_SERVER_PORT = 2,
		VOICE_SERVER_PORT = 3,
		BROKER_SERVER_PORT = 4,
	};


	// Construction
	WorldServer();
	virtual ~WorldServer();

	// Accessors
	virtual dword		version() const = 0;					// get the server version, must match the client version..

	bool				active() const;
	const Context &		context() const;
	
	CharString			clientName( dword nClientId ) const;	// get client name
	WorldContext *		worldContext() const;					// get the world simulation

	// Mutators
	virtual bool		start( const Context & context );		// start the server
	virtual bool		update();								// returns true when running, returns false to shutdown
	virtual bool		stop();									// stops the server

	bool				sendChat( const char * pChat, dword nFromId = 0 );			// send chat to all connected clients

	void				addTeam( dword nClientId, Team & team );
	void				setTeamFlags( dword nClientId, int nTeamId, dword nFlags );
	void				deleteTeam( dword nClientId, int nTeamId );
	void				addCondition( dword nClientId, Condition & condition );
	void				deleteCondition( dword nClientId, int nConditionId );
	void				setTimeLimit( dword nClientId, dword nTimeLimit );

	void				addWorld( dword nClientId, WidgetWrap & world );
	void				deleteWorld( dword nClientId, const WidgetKey & nWorld );
	void				addZone( dword nClientId, WidgetWrap & zone );
	void				deleteZone( dword nClientId, const WidgetKey & nZone );

	// WorldServerContext.cpp
	bool				contextInitialize( dword nClientId, const WidgetWrap & worldContext );
	void				contextEnd( dword nClientId, const char * pEndMessage );
	void				contextName( dword nClientId, const char * pName );
	void				contextDescription( dword nClientId, const char * pDesc );
	void				contextType( dword nClientId, int nType );
	void				contextScript( dword nClientId, const char * pScript );

protected:
	// Types
	typedef std::map< dword, ClientContext >							ClientContextHash;			// ClientID -> ClientContext
	typedef std::map< dword, dword >									UserClientHash;				// UserID -> ClientID
	typedef std::map< WidgetKey, ClientArray >							NounClientHash;
	typedef std::map< dword, ClientArray >								TeamClientHash;
	typedef std::map< dword, ClientArray >								FactionClientHash;
	typedef std::map< dword, dword >									ClientUserHash;
	typedef std::map< dword, UserStorage::Ref >							UserStorageHash;
	typedef std::map< WidgetKey, Group::Ref >							GroupMap;

	struct LoginJob
	{
		LoginJob() : 
			bDone( false ), 
			nClientId( 0 ), 
			nSessionId( 0 ), 
			bTransfer( false ),
			bProxy( false ),
			bServer( false ),
			nProxyId( NULL_WIDGET ),
			nProxyFaction( 0 )
		{}

		bool			bDone;								// true when job is done
		dword			nClientId;
		dword			nSessionId;
		bool			bTransfer;							// true if this is a transfer or reconnection
		bool			bServer;							// true if this is a server login
		bool			bProxy;								// true if this is a proxy login
		WidgetKey		nProxyId;
		int				nProxyFaction;

		MetaClient::Profile	
						profile;
		MetaClient::Clan
						clan;

		LoginJob &		operator=( const LoginJob & copy );
	};

	// Data
	bool				m_bActive;										// server active
	bool				m_bSleeping;									// true if no clients
	dword				m_StartTime;									// when did this server start
	Event				m_eShutdownReady;								// signaled when the last zone is unlocked
	bool				m_bEndGame;										// true when the end game condition is met
	bool				m_bStopServer;									// true if servers should stop
	Context				m_Context;										// server startup context
	int					m_nPlayers;										// how many players are currently playing..
	int					m_nPlayersInQueue;								// how many players are currently in the queue

	MetaClient			m_MetaClient;									// metaclient object
	MetaClient::Server	m_Server;										// server registration information

	ClientContextHash	m_ClientContext;								// client context hash
	UserClientHash		m_UserClientHash;								// UserID -> ClientID
	List< dword >		m_Moderators;									// linked list of clients in moderator mode
	List< dword >		m_DeleteClientList;								// linked list of disconnected clients pending removal by deleteClients()

	WorldContext::Ref	m_pWorldContext;								// the simulation object for the worldContext
	NounClientHash		m_NounClientHash;								// players local to the specified noun
	TeamClientHash		m_TeamClientHash;								// Team ID -> Clients
	FactionClientHash	m_FactionClientHash;							// Faction ID -> Clients
	
	LogClient			m_LogClient;									// our client to the LogServer
	ProfilerServer		m_ProfilerServer;								// our profiler server object

	ProfileUpdateQueue	m_ProfileUpdateQueue;
	UserStorageHash		m_UserStorage;									// storage for all users hashed by their user ID
	GroupMap			m_GroupMap;
	Queue< LoginJob >	m_LoginQueue;									// queue of clients waiting to login

	// timers used to control the updates to server subsystems...
	Timer				m_UpdateTimer;									// controls how fast WorldServer::update() can be called, will block the calling thread..
	Timer				m_PingTimer;									// pingClients()
	Timer				m_PingTimerUDP;									// pingClientsUDP()
	Timer				m_SaveTimer;									// saveWorld()
	Timer				m_StatsTimer;

	// stats data
	dword				m_nLastStatsLog;
	dword				m_nLastSentBytes;
	dword				m_nLastRecvBytes;

	// UDPServer interface
	virtual void		onUDP( u32 nAddress, u16 nPort, u8 nMessage, UDP * pUDP );
	virtual void		onClientUDP( u32 nClient, u8 nMessage, UDP * pUDP );

	// Server interface
	virtual void		onConnect( dword client );						// called when connection made
	virtual void		onReceive( dword client, byte message, const InStream & );
	virtual void		onDisconnect( dword client );					// called when connection lost

	// Mutators
	bool				validateClient( ClientContext & context, 
							dword nFlags = 0 );							// check the client flags, disconnects them on failure

	bool				isValid( dword nClientId );						// returns true if the client is logged in
	bool				isClient( dword nClientId );					// check that we can send to a client
	bool				isPlayer( dword nClientId );					// returns true if client is logged in..
	bool				isModerator( dword nClientId );

	dword				getClientId( dword nUserId );
	dword				getUserId( dword nClientId );					// get the nUserId for the specified client
	dword				getFlags( dword nClientId );					// get the rights for the specified client

	int					addTeam( const MetaClient::Clan & clan );

	void				sendContext( ClientContext & context );
	bool				sendStorage( ClientContext & context );

	virtual bool		addPlayerProfile( dword userId,
							const char * pField, float value );

	virtual bool		prepWorld();									// called before the WorldContext is started...
	virtual bool		loadWorld( bool bUseBackup );
	virtual bool		saveWorld( bool bForce = false );
	virtual bool		cleanWorld();

	// WorldServerLogin.cpp
	virtual bool		createProfile( ClientContext & context );		// over ridable factory functions for creation of a clients Profile object..
	virtual bool		createProxy( ClientContext & context, 
							const WidgetKey & a_nObjectId, int a_nFactionId );

	void				loginClients();
	bool				loginClient( const LoginJob & job );
	virtual void		onLogin( ClientContext & client );			// invoked after client has been logged into this server
	bool				logoffClient( dword nClientId );
	bool				deleteClients();							// enumerate disconnected clients, clean up expired client connections
	bool				deleteClient( ClientContext & client, 
							bool bCloseStorage );					// final call to cleanup a client connection, called by deletePlayers()
	bool				transferClient( dword nClientId,
							const WorldClient::TransferRequest & a_Req );

	virtual void		onDeleteClient( ClientContext & client );	// allow application to perform any final actions before a client is deleted..
	virtual bool		onTransferClient( ClientContext & client, 
							const WorldClient::TransferRequest & a_Req );

	// WorldServerCommand.cpp
	virtual bool		processCommand( ClientContext & context, const char * pChat );
	bool				processGlobalChat( ClientContext & context, const char * pChat, Color color );
	bool				processTeamChat( ClientContext & context, const char * pChat, Color color );
	bool				processFactionChat( ClientContext & context, const char * pChat, Color color );
	bool				processTargetChat( ClientContext & context, const char * pChat, Color color );
	bool				processAreaChat( ClientContext & context, const char * pChat, Color color );

	bool				sendChat( dword nClientId, const char * pChat, dword nFromId = 0 );
	bool				sendUserChat( dword nUserId, const char * pChat, dword nFromId  = 0);
	bool				sendUserChat( const Array< dword > & players, const char * pChat, dword nFromId = 0 );

	bool				checkMuted( ClientContext & context, const char * pChat );			// returns true if user is muted..
	bool				sendModeratorChat( const char * pChat, dword nFromId = 0 );			// send's message to all moderators
	bool				filterChat( ClientContext & context, CharString & a_sChat );		// filter the given chat message

	CharString			chatTime();

	// WorldServerUpdate.cpp
	bool				updateServer();	
	bool				pingClients();
	void				receivePong( ClientContext & context, dword nTime, bool bUDP );
	void				flushProfileQueue( bool bForce = false );
	void				logStats();

	// WorldServerMessage.cpp
	void				receiveMessage( ClientContext & context, bool bUDP, byte nMessage, const InStream & input );
	virtual void		onMessage( ClientContext & context, bool bUDP, byte nMessage, const InStream & input );
	bool				receiveChat( ClientContext & context, const char * pChat );

	void				setModerate( dword clientId, bool moderate );
	void				sendProfile( dword clientId );
	void				sendServerStatus( dword clientId );	// server server status to specified client

	bool				setClientFocus( dword nSenderId, dword nClientId, const WidgetKey & nNoun );
	bool				setClientFocusArea( dword nSenderId, dword nClientId, float fFocusArea );
	bool				setClientTarget( dword nSenderId, dword nClientId, const WidgetKey & nTarget );
	bool				setClientTeam( dword nSenderId, dword nClientId, int nTeamId );
	bool				setClientSelf( dword nSenderId, dword nClientId, const WidgetKey & nShip );

	virtual bool		selectTeam( dword nClientId, int nTeamId );
	virtual bool		canJoinTeam( ClientContext & client, int nTeamId );			// called by selectTeam() to control
	virtual void		onJoinTeam( ClientContext & client, int nTeamId );			// invoked after the client has been joined to the team
	virtual bool		selectSelf( dword nClientId, const WidgetKey & nSpawn, 
							const WidgetKey & nShip, const Array< WidgetKey > & cargo );
	virtual bool		canSpawn( ClientContext & context, 
							Noun * pSpawnPoint, 
							Noun * pSpawn, 
							Array< Noun::Ref > & Storage );
	virtual bool		onSpawn( ClientContext & context, 
							Noun * pSpawnPoint, 
							Noun * pSpawn, 
							bool bNewSpawn,
							Array< Noun::Ref > & Storage );

	bool				addStorage( dword nClientId, dword nUserId, const WidgetKey & nKey, const Storage & storage );
	bool				deleteStorage( dword nClientId, dword nUserId, const WidgetKey & nNoun );

	// WorldServerMeta.cpp
	bool				checkMetaServer();								// make sure we are connected to the metaserver, returns true if connected
	virtual bool		publishServer();								// send updated information back to the MetaServer
	bool				updateLoginQueue();								// process the login queue

	// WorldServerNoun.cpp
	bool				addNoun( dword nClientId, const WidgetKey & nParent,
							const WidgetKey & nNoun, const WidgetWrap & instance );
	bool				routeAddNoun( dword nClientId, Noun * pNoun );
	bool				deleteNoun( dword nClientId, const WidgetKey & nNoun, dword nLastTick );
	bool				routeDeleteNoun( dword nClientId, Noun * pNoun );

	bool				sendAddNoun( dword nClientId, const WidgetKey & nParent,
							const WidgetKey & nNoun, const WidgetWrap & instance );
	bool				sendUpdateNoun( dword nClientId, const WidgetKey & nParent,
							const WidgetKey & nNoun, const WidgetWrap & update );
	bool				sendDesyncNoun( dword nClientId, const WidgetKey & nNoun );
	bool				sendDeleteNoun( dword nClientId, const WidgetKey & nNoun, 
							dword nLastTick );

	ClientArray *		getNounClients( Noun * pNoun );					// get all clients local to this noun
	ClientArray *		getNounClients( const WidgetKey & nNoun );	
	NounArray *			getClientNouns( dword nClientId );				// get nouns local to the provided clients

	bool				updateClientNouns();							// update the local noun list for all clients
	bool				updateClientNouns( dword nClientId );


	bool				setNounPosition( dword nClientId, const WidgetKey & nNoun,
							const Vector3 & vPosition );
	bool				releaseNoun( Noun * pNoun );					// release all internal hash tables for a noun

	// WorldServerVerb.cpp
	bool				addVerb( ClientContext & context, const WidgetKey & nTarget, const WidgetWrap & verb, dword nUserId );
	bool				routeVerb( Verb * pVerb );

	OutStream			sendClient( dword nClientId, u8 nMessage, bool bUDP, bool bValidUser );

	bool				sendVerb( const ClientArray & clients, dword nSenderId,
							const WidgetKey & nTarget, const WidgetWrap & verb, bool bUDP, bool bCheckNoun );
	bool				sendVerb( const ClientArray & clients, Verb * pVerb );

	bool				sendVerb( dword nClientId, const WidgetKey & nTarget, 
							const WidgetWrap & verb, bool bUDP, bool bCheckNoun );
	bool				sendVerb( dword nClientId, Verb * pVerb );

	// WorldServerStorage.cpp
	bool				inStorage( dword nUserId, const WidgetKey & nKey ) const;
	UserStorage *		getStorage( dword nUserId ) const;
	Storage *			findStorage( dword nUserId, const WidgetKey & nKey ) const;
	Noun::Ref			getStorageNoun( dword nUserId, const WidgetKey & nKey ) const;
	dword				getStorageFactionId( dword nUserId, const WidgetKey & nKey ) const;
	Noun *				getStorageLocation( dword nUserId, const WidgetKey & nKey ) const;

	bool				loadStorage( ClientContext & context );
	virtual bool		onStorageLoaded( ClientContext & context, UserStorage * a_pStorage );
	bool				closeStorage( dword nUserId );
	bool				clearStorage();
	bool				flushStorage( bool a_bForce );					// flush all user storage to disk, called before shutdown..

	// WorldServerGroup.cpp
	bool				checkClientGroup( ClientContext & context );
	bool				groupInvite( ClientContext & context, Noun * pTarget );
	bool				groupDecline( ClientContext & context, Noun * pTarget );
	bool				groupRemove( ClientContext & context, Noun * pTarget );
	bool				groupPromote( ClientContext & context, Noun * pTarget );
	bool				sendGroupUpdate( Group * pGroup );				// send this group to all members, called when group is changed.
	bool				disbandGroup( Group * pGroup );
	void				validateGroup( Group * pGroup, bool bAllowUpdate = true );
	bool				groupConnected( Group * pGroup );				// returns true if any of the group members are connected
	Group *				getGroup( dword nUserId );

	virtual int			getNounType( Noun * pNoun );					// use to set the nType in GroupMember, should be overridden to provide a application
																		// specific type.

	// WorldContext::User interface
	virtual bool		isServer() const;								// is the user a server, returning false makes this object a client
	virtual bool		isSleeping() const;
	virtual dword		serverRole() const;	
	virtual int			maxPlayers() const;								// get the maximum number of players in this world
	virtual int			playerCount() const;							// get the current number of players in this world
	virtual int			spawnedScore( int a_nFactionId );
	virtual int			maxPlayerScoreAI();
	virtual int			baseTeamAi() const;

	virtual void		onChat( Noun * pNoun, const char * pMessage );

	virtual bool		canAttachVerb( Noun * pTarget, Verb * pVerb );
	virtual void		onAttachVerb( Verb * pVerb );					// called for outgoing verbs

	virtual void		onAddTeam( const Team & team );
	virtual void		onDeleteTeam( int nTeamId );
	virtual void		onAddCondition( const Condition & condition );
	virtual void		onDeleteCondition( int conditionId );

	virtual void		onAddWorld( WorldContext * pWorld );
	virtual void		onDeleteWorld( WorldContext * pWorld );
	virtual void		onAddZone( NodeZone * pZone );
	virtual void		onDeleteZone( NodeZone * pZone );
	virtual void		onAttachNoun( Noun * pNoun );
	virtual void		onUpdateNoun( Noun * pNoun );
	virtual void		onDetachNoun( Noun * pNoun );
	virtual void		onTransferNoun( Noun * pNoun, 
							NodeZone * pNewZone,
							NodeZone * pOldZone );
	virtual void		onWorldTransform( const Vector3 & transform );
	virtual void		onCondition( int conditionId );					// called when a condition is true

	virtual void		onServerTransfer( Noun * a_pWho, const char * a_pAddress, 
							int a_nPort, const char * a_pSpawnPoint );

	//-------------------------------------------------------------------------------

	class MetaUpdate : public SafeThread
	{
	public:
		// Data
		WorldServer *	m_pServer;
		// Construction
		MetaUpdate( WorldServer * pServer ) 
			: m_pServer( pServer )
		{}
		// Thread interface
		int	run()
		{
			return( m_pServer->metaUpdateDemon() );
		}
	};

	friend class Update;
	friend class MetaUpdate;

	int					metaUpdateDemon();

	MetaUpdate *		m_pMetaUpdate;		// update object
};

//----------------------------------------------------------------------------

inline bool WorldServer::active() const
{
	return m_bActive;
}

inline const WorldServer::Context & WorldServer::context() const
{
	return m_Context;
}

inline CharString WorldServer::clientName( dword nClientId ) const
{
	ClientContextHash::const_iterator iClient = m_ClientContext.find( nClientId );
	if ( iClient != m_ClientContext.end() )
		return (*iClient).second.MetaProfile.name;

	return "?";
}

inline WorldContext * WorldServer::worldContext() const
{
	return m_pWorldContext;
}

//----------------------------------------------------------------------------

inline WorldServer::Context::Context() :
	nServerFlags( 0 ),
	nClientFlags( 0 ),
	nModFlags( 0 ),
	bPublish( false ),
	nMetaPort( 0 ),
	nUniverseContext( (qword)0 ),
	maxPlayers( 0 ),
	maxRank( 0 ),
	bBalanced( false ),
	bClan( false ),
	bTutorial( false ),
	nSessionId( 0 ),
	gameId( 0 ),
	nLogPort( 0 ),
	port( 0 ),
	maxClients( 0 ),
	nCompression( 0 ),
	nDisconnectTime( 0 ),
	bPersistant( false ),
	bEditor( false ),
	bEnableUDP( true ),
	nServerRole( 0xffffffff ),
	bUseShipFlags( false )
{}

inline WorldServer::Context::Context(const Context & copy )
{
	operator=( copy );
}

inline WorldServer::Context & WorldServer::Context::operator=(const Context & copy )
{
	sConfigFile = copy.sConfigFile;
	nServerFlags = copy.nServerFlags;
	nClientFlags = copy.nClientFlags;
	nModFlags = copy.nModFlags;

	bPublish = copy.bPublish;
	sMetaAddress = copy.sMetaAddress;
	nMetaPort = copy.nMetaPort;

	nUniverseContext = copy.nUniverseContext;

	maxPlayers = copy.maxPlayers;
	maxRank = copy.maxRank;
	bBalanced = copy.bBalanced;
	bClan = copy.bClan;
	bTutorial = copy.bTutorial;

	user = copy.user;
	password = copy.password;
	nSessionId = copy.nSessionId;
	gameId = copy.gameId;

	sLogAddress = copy.sLogAddress;
	nLogPort = copy.nLogPort;

	name = copy.name;
	description = copy.description;
	address = copy.address;
	port = copy.port;
	maxClients = copy.maxClients;
	nCompression = copy.nCompression;
	nDisconnectTime = copy.nDisconnectTime;

	runScript = copy.runScript;
	bPersistant = copy.bPersistant;
	sSaveFile = copy.sSaveFile;
	sStorage = copy.sStorage;
	bEditor = copy.bEditor;
	bEnableUDP = copy.bEnableUDP;

	nServerRole = copy.nServerRole;
	sMasterAddress = copy.sMasterAddress;
	nMasterPort = copy.nMasterPort;

	bUseShipFlags = copy.bUseShipFlags;
	sShipFlags = copy.sShipFlags;

	return *this;
}

//----------------------------------------------------------------------------

inline WorldServer::ClientContext::ClientContext() :
	nClientId( 0 ),
	bModerate( false ),
	bProxy( false ),
	bServer( false ),
	nQueuePosition( 0 ),
	bConnectedUDP( false ),
	nLastUDP( 0 ),
	nLastFocusTime( 0 ),
	nLastTime( 0 ),
	nLastBytesSent( 0 ),
	nLatency( 0 ),
	nLatencyUDP( 0 ),
	nPings( 0 ),
	nUserId( 0 ),
	nFactionId( 0 ),
	nTeamId( 0 ),
	fFocusArea( 0.0f ),
	nLoginTime( 0 ),
	nLogoffTime( 0 ),
	bServerTransfer( false )
{}

inline WorldServer::ClientContext::ClientContext( const ClientContext & copy )
{
	operator=( copy);
}

inline WorldServer::ClientContext & WorldServer::ClientContext::operator=( const ClientContext & copy )
{
	nClientId = copy.nClientId;
	pSocket = copy.pSocket;
	bModerate = copy.bModerate;
	bProxy = copy.bProxy;
	bServer = copy.bServer;
	nQueuePosition = copy.nQueuePosition;
	bConnectedUDP = copy.bConnectedUDP;
	nLastUDP = copy.nLastUDP;
	nLastFocusTime = copy.nLastFocusTime;
	nLastTime = copy.nLastTime;
	nLastBytesSent = copy.nLastBytesSent;
	nLatency = copy.nLatency;
	nLatencyUDP = copy.nLatencyUDP;
	nPings = copy.nPings;
	MetaProfile = copy.MetaProfile;
	pProfile = copy.pProfile;
	pStorage = copy.pStorage;
	nUserId = copy.nUserId;
	nFactionId = copy.nFactionId;
	nTeamId = copy.nTeamId;
	pFocus = copy.pFocus;
	fFocusArea = copy.fFocusArea;
	pTarget = copy.pTarget;
	pSelf = copy.pSelf;
	localNouns = copy.localNouns;
	updateHash = copy.updateHash;
	nLoginTime = copy.nLoginTime;
	nLogoffTime = copy.nLogoffTime;
	bServerTransfer = copy.bServerTransfer;
	return *this;
}

//----------------------------------------------------------------------------

inline WorldServer::LoginJob & WorldServer::LoginJob::operator=( const LoginJob & copy )
{
	bDone = copy.bDone;
	nClientId = copy.nClientId;
	nSessionId = copy.nSessionId;
	profile = copy.profile;
	clan = copy.clan;
	bTransfer = copy.bTransfer;
	bProxy = copy.bProxy;
	return *this;
}

//-------------------------------------------------------------------------------

#endif

//----------------------------------------------------------------------------
// EOF
