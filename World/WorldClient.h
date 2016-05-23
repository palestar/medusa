/*
	WorldClient.h

	This object is used to connect to an WorldServer object over a Socket object.

	If you use loginproxy() then this client connects as a light-weight client to the server.
	* A proxy client can select a team, set focus, see objects, and send verbs.. but you can't spawn with a proxy login. 
	* Proxy	mode is used by clients to see the contents of other servers and by AI servers to off-load AI processing onto another process.
	* In proxy mode the full context is NOT sent to the client.

	(c)1999 Palestar, Richard Lyle
*/

#ifndef UNIVERSE_CLIENT_H
#define UNIVERSE_CLIENT_H

#include "Standard/Event.h"
#include "Network/Client.h"
#include "Network/UDPServer.h"

#include "GCQ/MetaClient.h"
#include "Profile.h"
#include "Noun.h"
#include "Verb.h"
#include "WorldContext.h"
#include "WorldTimer.h"
#include "Group.h"
#include "UserStorage.h"
#include "WorldDll.h"

//----------------------------------------------------------------------------

const dword		MIN_BPS = 3 * 1024;
const dword		MAX_BPS = 32 * 1024;
const float		MIN_FOCUS = 1000.0f;
const float		MAX_FOCUS = 5000.0f;

//----------------------------------------------------------------------------

class DLL WorldClient : public Widget, public Client, public UDPServer, public WorldContext::User
{
public:
	DECLARE_WIDGET_CLASS();

	// Types
	typedef Reference< WorldClient >	Ref;
	typedef MetaClient::Server			Server;
	typedef WorldContext				Context;

	typedef Array< WidgetKey >			WidgetKeys;

	typedef WorldContext::Team			Team;
	typedef WorldContext::Condition		Condition;
	typedef UserStorage::Storage		Storage;

	enum {
		SERVER_CLIENTID = 1,
		CLIENT_PING_TIME = 30,					// how often to send a TCP ping to the server..
		CLIENT_UDP_PING_TIME = 5,				// how often to send a UDP ping to the server..
		CLIENT_UDP_DOWN_TIME = 16,				// if no UDP is received for this amount of time, UDP is consider down..
		MAX_GROUP_MEMBERS = 8,					// maximum size of a player group
	};

	enum Messages
	{
		// Client messages
		CLIENT_RECV_VERSION			= 0x00,	// server is sending it's version
		CLIENT_RECV_CK_DICT			= 0x01,	// server is sending it's current class key dictionary
		CLIENT_RECV_SERVER_STATUS	= 0x02,	// server is sending the server status
		CLIENT_SERVER_TRANSFER		= 0x03,	// server is sending the client to another server
		CLIENT_LOGIN				= 0x04,	// Server has logged in the client
		CLIENT_RECV_QUEUEPOS		= 0x05,	// server is sending the client their queue position

		CLIENT_SET_PROFILE			= 0x06,	// server is sending updated user information
		CLIENT_SET_FOCUS			= 0x07,	
		CLIENT_SET_FOCUS_AREA		= 0x08,
		CLIENT_SET_TARGET			= 0x09,
		CLIENT_SET_SELF				= 0x0a,
		CLIENT_SET_TEAM				= 0x0b,		
		CLIENT_RECV_CHAT			= 0x0c,	// server is sending direct chat to the client
		
		CLIENT_RECV_STORAGE			= 0x0d,	// server is sending the UserStorage object to the client
		CLIENT_ADD_STORAGE			= 0x0e,	// add item to player storage
		CLIENT_DEL_STORAGE			= 0x0f,	// delete item from storage

		// Server messages
		SERVER_SEND_VERSION			= 0x40, // client is requesting a version from the server
		SERVER_LOGIN				= 0x41,	// Client is logging into server
		SERVER_SET_MODERATE			= 0x42, // client is requesting moderator mode
		SERVER_SEND_PROFILE			= 0x43, // requesting the current player profile
		SERVER_SEND_SERVER_STATUS	= 0x44,	// client is requesting detailed status information
		SERVER_TRANSFER				= 0x45,	// client is requesting to be transfered to another server

		SERVER_SET_FOCUS			= 0x46,	// Client is changing it's focus
		SERVER_SET_TARGET			= 0x47,	// Client is changing it's target
		SERVER_SET_TEAM				= 0x48,	// Client is setting it's team
		SERVER_SET_SELF				= 0x49,	// Client is changing it's self

		SERVER_SELECT_TEAM			= 0x4a,	// player is selecting their team
		SERVER_SELECT_SELF			= 0x4b,	// Player is selecting their NounTemplate
		SERVER_SELECT_PROXY			= 0x4c,	// client is sending their proxy ID to this server

		SERVER_REMOVE_SELF			= 0x4d,	// client is detaching their self
		SERVER_REMOVE_STORAGE		= 0x4e, // player is deleting their own storage
		SERVER_RECV_CHAT			= 0x4f,	// a client is sending chat/command to this server
		SERVER_RECV_VERB			= 0x50,	// Client is sending a verb for routing
		SERVER_SEND_NOUN			= 0x51,	// client is requesting the server send a specific noun by ID

		// Group Messages
		GROUP_UPDATE				= 0x71,	// sent by the server to update the group object
		GROUP_DEL					= 0x72,	// sent by the server to remove the group
		GROUP_INVITE				= 0x73,	// sent by the client to invite a player into a group
		GROUP_DECLINE				= 0x74,
		GROUP_REMOVE				= 0x75,	// sent by the client to leave their current group
		GROUP_PROMOTE				= 0x76,	// sent by the client/server to make another player the leader

		// Context messages
		CONTEXT_INIT				= 0x80,	
		CONTEXT_END					= 0x81,
		CONTEXT_SET_NAME			= 0x82,
		CONTEXT_SET_DESC			= 0x83,
		CONTEXT_SET_TYPE			= 0x84,
		CONTEXT_SET_SCRIPT			= 0x85,
		CONTEXT_ADD_TEAM			= 0x86,
		CONTEXT_SET_FLEET_FLAGS		= 0x87,
		CONTEXT_DEL_FLEET			= 0x88,
		CONTEXT_ADD_CONDITION		= 0x89,
		CONTEXT_DEL_CONDITION		= 0x8a,
		CONTEXT_SET_TIME_LIMIT		= 0x8b,
		CONTEXT_ADD_WORLD			= 0x8c,
		CONTEXT_DEL_WORLD			= 0x8d,
		CONTEXT_ADD_ZONE			= 0x8e,
		CONTEXT_DEL_ZONE			= 0x8f,
		CONTEXT_ADD_NOUN			= 0x90,
		CONTEXT_UPDATE_NOUN			= 0x91,
		CONTEXT_DESYNC_NOUN			= 0x92,			// clear the synchronized flag from a noun
		CONTEXT_DEL_NOUN			= 0x93,
		CONTEXT_ADD_VERB			= 0x94,
		CONTEXT_CRON				= 0x95,
		CONTEXT_RUN_SCRIPT			= 0x96,
		CONTEXT_KILL_SCRIPT			= 0x97,	

		// UDP negotiation
		UDP_START_CONNECT			= 0xfc,			// sent by the server to begin UDP negotiations..
		UDP_ACK_CONNECT				= 0xfd,			// sent by the client back to the server to open the UDP channel..

		// misc messages
		PING						= 0xfe,
		PONG						= 0xff
	};
	struct ClientStatus
	{
		CharString				name;
		dword					userId;
		dword					latency;
		CharString				address;			// will be empty if players is not a moderator

		ClientStatus() : userId( 0 ), latency( 0 )
		{}

		ClientStatus &			operator=( const ClientStatus & copy );
	};
	struct ServerStatus
	{
		dword					gameId;
		CharString				name;
		CharString				address;
		u16						port;
		dword					upTime;
		dword					bytesSent;
		dword					bytesRecv;
		dword					maxClients;
		int						maxRank;
		bool					bPersistant;
		bool					bUseShipFlags;
		CharString				sShipFlags;

		Array< ClientStatus >	clients;

		ServerStatus() : gameId( 0 ), port( 0 ), maxRank( 0 ), bPersistant( false ), bUseShipFlags( false )
		{}

		ServerStatus &			operator=( const ServerStatus & copy );
	};
	struct TransferRequest 
	{
		CharString			m_sAddress;
		u16					m_nPort;
		CharString			m_sSpawnPoint;
		WidgetKey			m_nSelf;
		WidgetKeys			m_Items;

		TransferRequest() : m_nPort( 0 ), m_nSelf( (qword)0 )
		{}
	};
	class DLL UserWorldClient
	{
	public:
		virtual void	onConnected();							// connection established with the server
		virtual void	onLogin( bool bSuccess );				// called when client is logged into server
		virtual void	onUpdateQueuePosition();				// called anytime the queue position has changed
		virtual void	onMetaLogin();							// called after we are connected with the meta server
		virtual void	onContextReceived();					// called when all game data has been received, textures should now be pre-cached usually
		virtual void	onContextEnd( const char * pEndMessage );	// end of game message has been sent by sever
		virtual void	onDisconnected();						// lost connection to the server

		virtual void	onWorldTransform( const Vector3 & transform );

		virtual void	onServerTransfer( const TransferRequest & a_Req );		// server is redirection this client to a different server
		virtual void	onProfile( Profile * pProfile );		// server sent updated profile info
		virtual void	onServerStatus( ServerStatus & status);	// server sent updated server status info

		virtual void	onChat( const char * pMessage, dword nFromId );		// called when chat is received
		virtual void	onFlushChat();							// called before chat buffer is cleared

		virtual void	onAttachVerb( Verb * pVerb );			// called when verb is received, after it is already pushed into the context

		virtual void	onSetFocus( Noun * pFocus );
		virtual void	onSetFocusArea( float fFocusArea );
		virtual void	onSetTarget( Noun * pTarget );
		virtual void	onSetSelf( Noun * pSelf );
		virtual void	onSetTeam( int nTeamId );

		virtual void	onReceiveStorage( UserStorage * a_pStorage );
		virtual void	onAddStorage( const WidgetKey & nKey );
		virtual void	onDeleteStorage( const WidgetKey & nKey );

		virtual void	onGroupInvite( Noun * a_pWho );			// invoked when invited to a group, groupInvite() should be called to accept. 
		virtual void	onGroupUpdate( Group * a_pGroup );		// invoked when the group changes
		virtual void	onGroupLeave();							// called when the local player leaves their group
	};

	// Construction
	WorldClient();
	virtual ~WorldClient();

	// Client interface
	virtual void		onConnect();								// called when connection made
	virtual void		onReceive( byte type, const InStream & );			// called on incoming message
	virtual void		onDisconnect();								// called when connection lost
	// UDPServer interface
	virtual void		onUDP( u32 nAddress, u16 nPort, u8 nMessage, UDP * pUDP );
	virtual void		onClientUDP( u32 nClient, u8 nMessage, UDP * pUDP );

	// Accessors
	virtual dword		version() const = 0;				// version of this client, must match the server...
	
	UserWorldClient *	user() const;						// get the user of this class
	bool				active() const;						// is this client active
	bool				connected() const;					// are we currently connected
	bool				enableUDP() const;					
	bool				connectedUDP() const;
	bool				loggedIn() const;					// are we logged into the server
	bool				proxy() const;						// true if this is a proxy client
	const WidgetKey &	proxyId() const;					// ID of our proxy object
	int					proxyFactionId() const;				// Id of our porxy faction
	bool				server() const;						// true if this is a server client
	bool				isBanned() const;					// returns true if this account is banned 
	bool				inQueue() const;					// returns true if we are stuck in the login queue
	int					queuePosition() const;				// get our current queue position
	int					queueSize() const;					// get the current queue size
	dword				serverVersion() const;				// get the server version
	dword				latency() const;					// what is our latency
	dword				bps() const;						// our current bps setting
	int					maxRank() const;					// get maximum allowed rank

	dword				sessionId() const;					// our session id
	MetaClient *		metaClient() const;					// get the meta client object

	Profile *			profile() const;					// our user information
	bool				canModerate() const;				// are we a moderator
	const ServerStatus & serverStatus() const;				// server status

	dword				tick() const;						// the client tick
	Context *			context() const;					// the worldContext

	int					factionId() const;					// our factionId
	int					teamId() const;						// the players team
	Noun *				self() const;						// the players ship
	Noun *				focus() const;						// our focus object, by default the players ship
	Noun *				target() const;						// our current target

	int					chatCount() const;					// number of chat messages buffered
	const char *		chat( int n ) const;				// get chat message

	int					storageCount() const;				// how many items do we have in storage
	WidgetKey			storageKey( int n ) const;			// get the key for stored item
	const Storage &		storage( int n ) const;				// get storage item n
	Storage *			storage( const WidgetKey & nKey ) const;	// get storage by key
	Noun *				storageInstance( int n ) const;		// unwrap stored item

	Group *				group() const;						// get our current group, may return NULL
	bool				isGroupLeader() const;				// returns true if we are the group leader
	bool				isGroupMember( Noun * pNoun ) const;// returns true if object is a member of our group
	bool				isServerTransfer() const;			// if true, then we are being transfered to a new server..

	// Mutators
	virtual bool		prepWorld();						// called before the WorldContext is started...

	bool				setContext( Context * pContext );
	void				setUser( UserWorldClient * pUser );	// set the user 
	void				setEnableUDP( bool a_bEnableUDP );
	void				setEnableFilter( bool a_bFilter );
	void				setFilterFile( const char * a_pFile );

	bool				connect( const char * a_pAddress, int a_nPort );			// connect to this server
	bool				login( dword a_nSessionID );	
	bool				loginAsProxy( dword a_nSessionID, const WidgetKey & a_nObjectId,
							int a_nFactionId );					// login with a proxy connection
	bool				loginAsServer( dword a_nSessionID );	// login as a server connection

	bool				transfer( const TransferRequest & a_Request );	// transfer to a new server...
	bool				reconnect();						// reconnect to same server after disconnection
	bool				waitForContext();					// block until the context is received

	bool				setModerate( bool moderate );		// set moderator mode

	bool				sendProfile();						// request latest profile information
	bool				sendServerStatus();					// request latest server information

	bool				selectTeam( int nTeamId );			// select specified team
	bool				requestTransfer( const TransferRequest & a_Request );

	virtual bool		selectSelf( const WidgetKey & a_nSpawn, 
							const WidgetKey & a_nSelf,
							const WidgetKeys & items );
	virtual void		onSetSelf( Noun * pSelf );
	bool				selectProxy( const WidgetKey & a_nProxyId, int a_nFactionId );

	bool				removeSelf();
	bool				removeStorage( 
							const WidgetKeys & remove );	// remove items from player nouns

	virtual bool		canGroupInvite( Noun * pTarget );
	virtual bool		canGroupRemove( Noun * pTarget );
	virtual bool		canGroupPromote( Noun * pTarget );

	bool				groupInvite( Noun * pTarget );		// invite the given target into this players group.
	bool				groupDecline( Noun * pTarget );		// decline invite from player.
	bool				groupPromote( Noun * pTarget );		// promote another player to leader
	bool				groupRemove( Noun * pTarget );		// remove a player from the group, only works on others if client is the leader
	bool				groupLeave();						// leave the current group, calls groupRemove()

	bool				sendVerb( Noun * pTarget, 
							Verb * pVerb );					// send verb to server
	virtual void		setFocus( Noun * pFocus, bool bForce = false );			// set focus object
	virtual void		setTarget( Noun * pTarget, bool bForce = false );		// set target object

	void				update();							// position of the camera within the current zone
	virtual void		onUpdate();							// hook for child class to do updates as well

	// WorldClientCommand.cpp
	bool				sendChat( const char * pChat );		// send chat to the server
	void				sendChatReal( const char * pChat );	// send chat to server, do not check for commands
	bool				receiveChat( const char * pChat, dword nFromId = 0 );	// receive chat from the server
	virtual bool		pushMetaChat( const MetaClient::Chat & a_Chat );
	virtual bool		pushChat( const char * pChat, dword nFromId = 0 );		// push message into the local chat buffer
	void				flushChat();						// clear chat buffer

	virtual bool		processCommand( const char * pChat, dword nFromId = 0 );

	// Helpers
	Noun *				validateTarget( Noun * pTarget );	// check a target (depreciated, use validateNoun )
	virtual Noun *		validateNoun( Noun * pNoun );

	int					metaUpdateDemon();					// update information from the metaserver

	// WorldContext::User interface
	virtual bool		isServer() const;							
	virtual dword		userId() const;

	virtual void		onChat( Noun * pNoun, const char * pMessage );

	virtual bool		canAttachVerb( Noun * pTarget, Verb * pVerb );
	virtual void		onAttachVerb( Verb * pVerb );		// called when a verb is attached

	virtual void		onAddTeam( const Team & team );
	virtual void		onDeleteTeam( int nTeamId );
	virtual void		onAddCondition( const Condition & condition );
	virtual void		onDeleteCondition( int conditionId );

	virtual void		onAddZone( NodeZone * pZone );
	virtual void		onDeleteZone( NodeZone * pZone );
	virtual void		onAttachNoun( Noun * pNoun );
	virtual void		onDetachNoun( Noun * pNoun );
	virtual void		onTransferNoun( Noun * pNoun, 
							NodeZone * pNewZone,
							NodeZone * pOldZone );
	virtual void		onWorldTransform( const Vector3 & transform );
	virtual void		onCondition( int conditionId );					// called when a condition is true
	virtual void		onServerTransfer( Noun * pWho,
							const char * a_pAddress, int a_nPort, 
							const char * a_pSpawnPoint );

protected:
	// Types
	typedef Queue< Verb::Ref >						VerbQueue;
	typedef HashArray< WidgetKey, Storage >			StorageHash;

	// Data
	UserWorldClient *	m_pUser;			// user of this object
	bool				m_Active;			// is client active
	bool				m_Connected;		// true if we have a TCP connection
	bool				m_bEnableUDP;		// true if UDP is enabled
	bool				m_bEnableFilter;	// true if we should filter all chat text
	CharString			m_sFilterFile;		// file containing the filter
	bool				m_bConnectedUDP;	// true if our UDP connection to the server is active
	bool				m_LoggedIn;			// true if we are logged in
	bool				m_bProxy;			// true if this is a proxy login
	WidgetKey			m_nProxyId;			// id of our proxy object
	int					m_nProxyFactionId;	// id of our proxy faction
	bool				m_bServer;			// true if we are logged in as a server
	bool				m_bBanned;			// true if this client is banned
	int					m_nQueuePosition;
	int					m_nQueueSize;
	Event				m_LoginEvent;
	dword				m_ServerVersion;	// true if client and server versions are the same
	dword				m_nLatency;			// how many ms does it take for a round trip
	dword				m_nLatencyUDP;		// UDP latency
	dword				m_BPS;				// our current bytes per second rate

	dword				m_SessionId;		// my session key with the meta-server
	MetaClient  *		m_pMetaClient;		// connection to the main server
	dword				m_nDefaultRoomId;	// default room for any meta commands

	Profile::Ref		m_pProfile;			// my user information
	ServerStatus		m_ServerStatus;		// server status
	Context::Ref		m_pWorldContext;			// the worldContext
	Vector3				m_vContextTransform;	// context zones are offset by this vector
	Event				m_ContextReceived;

	int					m_nTeamId;			// our nTeamId
	Noun::wRef			m_pSelf;			// the players ship
	Noun::wRef			m_pFocus;			// our focus
	float				m_fFocusArea;
	Noun::wRef			m_pTarget;			// our target

	Array< CharString >	m_Chat;				// chat buffer
	UserStorage::Ref	m_pStorage;			// user persisted data / group

	int					m_LastChatMessage;	// last message index received from the metaserver
	dword				m_ReconnectTime;	// how long until we attempt to reconnect to the metaserver
	dword				m_nLastPingTime;	// last time we sent a PING
	dword				m_nLastPingTimeUDP;	// last time we sent a UDP_PING
	dword				m_nLastUDPTime;		// last time we received any UDP packet

	bool				m_bServerTransfer;	// if true then we are currently transferring to a new server
	TransferRequest		m_TransferReq;		// transfer request data

	// Mutators
	virtual void		updateMeta();		// this is invoked by the metaUpdateDemon() once per second from a thread

	// WorldClientMessages.cpp
	virtual void		receiveMessage( bool bUDP, u8 nMessage, const InStream & input );
	virtual void		onMessage( bool bUDP, u8 nMessage, const InStream & input );

	// Helpers
	static CharString	filterMetaChat( const char * pText );

	//-------------------------------------------------------------------------------


	class MetaUpdate : public SafeThread
	{
	public:
		// Data
		WorldClient *	m_pClient;
		// Construction
		MetaUpdate( WorldClient * pClient ) 
			: m_pClient( pClient )
		{}
		// Thread interface
		int	run()
		{
			return( m_pClient->metaUpdateDemon() );
		}
	};

	MetaUpdate *		m_pMetaUpdate;		// update object

	//-------------------------------------------------------------------------------
};

//----------------------------------------------------------------------------

inline WorldClient::UserWorldClient * WorldClient::user() const
{
	return m_pUser;
}

inline bool WorldClient::active() const
{
	return m_Active;
}

inline bool WorldClient::connected() const
{
	return m_Connected;
}

inline bool WorldClient::enableUDP() const
{
	return m_bEnableUDP;
}

inline bool WorldClient::connectedUDP() const
{
	return m_bConnectedUDP;
}

inline bool WorldClient::loggedIn() const
{
	return m_LoggedIn;
}

inline bool WorldClient::proxy() const
{
	return m_bProxy;
}

inline const WidgetKey & WorldClient::proxyId() const
{
	return m_nProxyId;
}

inline int WorldClient::proxyFactionId() const
{
	return m_nProxyFactionId;
}

inline bool WorldClient::server() const
{
	return m_bServer;
}

inline bool WorldClient::isBanned() const
{
	return m_bBanned;
}

inline bool WorldClient::inQueue() const
{
	return m_nQueuePosition > 0;
}

inline int WorldClient::queuePosition() const
{
	return m_nQueuePosition;
}

inline int WorldClient::queueSize() const
{
	return m_nQueueSize;
}

inline dword WorldClient::serverVersion() const
{
	return m_ServerVersion;
}

inline dword WorldClient::latency() const
{
	return m_nLatency;
}

inline dword WorldClient::bps() const
{
	return m_BPS;
}

inline int WorldClient::maxRank() const
{
	return m_ServerStatus.maxRank;
}

inline dword WorldClient::sessionId() const
{
	return( m_SessionId );
}

inline MetaClient * WorldClient::metaClient() const
{
	return( m_pMetaClient );
}

inline Profile * WorldClient::profile() const
{
	return( m_pProfile );
}

inline const WorldClient::ServerStatus & WorldClient::serverStatus() const
{
	return m_ServerStatus;
}

inline bool WorldClient::canModerate() const
{
	return m_pProfile ? (m_pProfile->flags() & MetaClient::MODERATOR) != 0 : false;
}

inline dword WorldClient::tick() const
{
	return m_pWorldContext->tick();
}

inline WorldClient::Context * WorldClient::context() const
{
	return( m_pWorldContext );
}

inline int WorldClient::factionId() const
{
	if ( context()->isTeamValid( m_nTeamId ) )
		return context()->team( m_nTeamId ).factionId;
	return 0;
}

inline int WorldClient::teamId() const
{
	return m_nTeamId;
}

inline Noun * WorldClient::self() const
{
	return m_pSelf;
}

inline Noun * WorldClient::focus() const
{
	return m_pFocus;
}

inline Noun * WorldClient::target() const
{
	return m_pTarget;
}

inline int WorldClient::chatCount() const
{
	return m_Chat.size();
}

inline const char * WorldClient::chat( int n ) const
{
	return m_Chat[ n ];
}

inline int WorldClient::storageCount() const
{
	return m_pStorage->storageCount();
}

inline WidgetKey WorldClient::storageKey( int n ) const
{
	return m_pStorage->storageKey( n );
}

inline const WorldClient::Storage & WorldClient::storage( int n ) const
{
	return m_pStorage->storage( n );
}

inline WorldClient::Storage * WorldClient::storage( const WidgetKey & nKey ) const
{
	StorageHash::Iterator iStorage = m_pStorage->getStorageHash().find( nKey );
	if ( iStorage.valid() )
		return &(*iStorage);

	return NULL;
}

inline Noun * WorldClient::storageInstance( int n ) const
{
	return (Noun *)storage( n ).instance.unwrap();
}

inline Group * WorldClient::group() const
{
	return m_pStorage->group();
}

inline bool WorldClient::isGroupLeader() const
{
	Group * pGroup = group();
	if (! pGroup )
		return true;

	for(int i=0;i<pGroup->memberCount();++i)
	{
		GroupMember * pMember = pGroup->member(i);
		if ( pMember->isLeader() && pMember->userId() == userId() )
			return true;
	}

	return false;
}

inline bool WorldClient::isGroupMember( Noun * pNoun ) const
{
	Group * pGroup = group();
	if (! pGroup || !pNoun )
		return false;

	for(int i=0;i<pGroup->memberCount();++i)
		if ( pGroup->member(i)->userId() == pNoun->userId() )
			return true;

	return false;
}


inline bool WorldClient::isServerTransfer() const
{
	return m_bServerTransfer;
}

//----------------------------------------------------------------------------

inline WorldClient::ClientStatus & WorldClient::ClientStatus::operator =( const ClientStatus & copy )
{
	name = copy.name;
	userId = copy.userId;
	latency = copy.latency;
	address = copy.address;
	return *this;
}

inline const InStream & operator>>( const InStream & input, WorldClient::ClientStatus & status )
{
	input >> status.name;
	input >> status.userId;
	input >> status.latency;
	input >> status.address;
	return input;
}

inline const OutStream & operator<<( const OutStream & output, const WorldClient::ClientStatus & status )
{
	output << status.name;
	output << status.userId;
	output << status.latency;
	output << status.address;
	return output;
}

//----------------------------------------------------------------------------

inline WorldClient::ServerStatus & WorldClient::ServerStatus::operator =( const ServerStatus & copy )
{
	gameId = copy.gameId;
	name = copy.name;
	address = copy.address;
	port = copy.port;
	upTime = copy.upTime;
	bytesSent = copy.bytesSent;
	bytesRecv = copy.bytesRecv;
	maxClients = copy.maxClients;
	maxRank = copy.maxRank;
	clients = copy.clients;
	bPersistant = copy.bPersistant;
	bUseShipFlags = copy.bUseShipFlags;
	sShipFlags = copy.sShipFlags;
	return *this;
}

inline const InStream & operator>>( const InStream & input, WorldClient::ServerStatus & status )
{
	input >> status.gameId;
	input >> status.name;
	input >> status.address;
	input >> status.port;
	input >> status.upTime;
	input >> status.bytesSent;
	input >> status.bytesRecv;
	input >> status.maxClients;
	input >> status.maxRank;
	input >> status.clients;
	input >> status.bPersistant;
	input >> status.bUseShipFlags;
	input >> status.sShipFlags;
	return input;
}

inline const OutStream & operator<<( const OutStream & output, const WorldClient::ServerStatus & status )
{
	output << status.gameId;
	output << status.name;
	output << status.address;
	output << status.port;
	output << status.upTime;
	output << status.bytesSent;
	output << status.bytesRecv;
	output << status.maxClients;
	output << status.maxRank;
	output << status.clients;
	output << status.bPersistant;
	output << status.bUseShipFlags;
	output << status.sShipFlags;
	return output;
}

//---------------------------------------------------------------------------------------------------

inline const InStream & operator>>( const InStream & input, WorldClient::TransferRequest & req )
{
	input >> req.m_sAddress;
	input >> req.m_nPort;
	input >> req.m_sSpawnPoint;
	input >> req.m_nSelf;
	input >> req.m_Items;
	return input;
}

inline const OutStream & operator<<( const OutStream & output, const WorldClient::TransferRequest & req )
{
	output << req.m_sAddress;
	output << req.m_nPort;
	output << req.m_sSpawnPoint;
	output << req.m_nSelf;
	output << req.m_Items;
	return output;
}


//----------------------------------------------------------------------------

#endif

//----------------------------------------------------------------------------
// EOF
