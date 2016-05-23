/*
	MetaClient.h

	The MetaClient object is used to connect to a MetaServer, either to publish a game server or to get
	the list of all available game servers

	(c)1999 PaleStar Development, Richard Lyle
*/

#ifndef META_CLIENT_H
#define META_CLIENT_H

#include "Standard/Event.h"
#include "Network/Client.h"
#include "Factory/WidgetWrap.h"
#include "GCQ/PBM.h"
#include "GCQ/Binary.h"
#include "GCQ/GCQDll.h"

//-------------------------------------------------------------------------------

class DLL MetaClient : public Client
{
public:
	enum Messages
	{
		CLIENT_RECV_KEY			= 0x00,		// SERVER_SEND_KEY
		CLIENT_LOGIN			= 0x01,		// SERVER_LOGIN SERVER_LOGIN_SESSION SERVER_LOGIN_CREATE
		CLIENT_RECV_GAMES		= 0x02,		// SERVER_SEND_GAMES
		CLIENT_RECV_SERVERS		= 0x03,		// SERVER_SEND_SERVERS
		CLIENT_RECV_ADDRESS		= 0x04,		// SERVER_SEND_ADDRESS
		CLIENT_RECV_PROFILES	= 0x05,		// SERVER_SEND_PROFILES
		CLIENT_RECV_PROFILE		= 0x06,		// SERVER_SEND_PROFILE
		CLIENT_RECV_CLANS		= 0x07,		// SERVER_SEND_CLANS
		CLIENT_RECV_CLAN		= 0x08,		// SERVER_SEND_CLAN
		CLIENT_JOB_DONE			= 0x09,		// SERVER_JOIN_CLAN, SERVER_CHANGE_PASSWORD, SERVER_CHANGE_NAME, SERVER_CHANGE_EMAIL, SERVER_EDIT_WATCHLIST
		CLIENT_RECV_ROOMS		= 0x0a,		// SERVER_SEND_ROOMS
		CLIENT_RECV_CHAT		= 0x0b,		// SERVER_SEND_CHAT
		CLIENT_ADD_ROOM			= 0x15,		// Client has joined a room chat
		CLIENT_DEL_ROOM			= 0x16,		// client has left a room
		CLIENT_ADD_PLAYER		= 0x0d,		// adding a player to a room
		CLIENT_DEL_PLAYER		= 0x0e,		// removing a player from a room
		CLIENT_RECV_GAME		= 0x10,		// SERVER_SELECT_GAME
		CLIENT_RECV_SELF		= 0x11,		// Server is sending updated profile information
		CLIENT_SEND_INSTALLER	= 0x13,		// Server is requesting the installer used for the game
		CLIENT_RECV_WATCHLIST	= 0xad,		// SERVER_SEND_WATCHLIST

		SERVER_CLAN_PROMOTE		= 0x78,		// client is promoting a member in their clan
		SERVER_CLAN_RECRUIT		= 0x79,		// client is recruiting a member in their clan
		SERVER_SEND_KEY			= 0x80,		// Client is requesting the public key
		SERVER_LOGIN			= 0x81,		// Client is attempting to login, only needed if you plan to register a server!
		SERVER_LOGIN_SESSION	= 0x82,		// Client is logging in with a sessionId
		SERVER_LOGIN_CREATE		= 0x83,		// Client is attempting to create a new login
		SERVER_LOGIN_PROXY		= 0xa6,		// Client is logging in another client indirectly
		SERVER_SEND_GAMES		= 0x84,		// Client is requesting the game list
		SERVER_SEND_SERVERS		= 0x85,		// Client requests a server list
		SERVER_REGISTER_SERVER	= 0x86,		// Client is registering a server
		SERVER_REGISTER_SERVER2	= 0xb2,		// Client is registering a server
		SERVER_REMOVE_SERVER	= 0x87,		// client is removing it's game
		SERVER_SEND_PROFILES	= 0x88,		// client requests a list of profiles
		SERVER_SEND_PROFILE		= 0x89,		// client requests a user profile
		SERVER_RECV_PROFILE		= 0x8a,		// client is sending a updated user profile
		SERVER_ADD_PROFILE		= 0xb3,		// client is sending field update
		SERVER_UPDATE_SCORE		= 0xb5,		// client is sending an updated score and rank
		SERVER_SEND_CLANS		= 0x8b,		// client is requesting a clan list
		SERVER_SEND_CLAN		= 0x8c,		// client requests clan information
		SERVER_SEND_MEMBERS		= 0x8d,		// client requests the clan members
		SERVER_RECV_CLAN		= 0x8e,		// client is sending clan information
		SERVER_CREATE_CLAN		= 0x8f,		// client is creatinig a clan
		SERVER_JOIN_CLAN		= 0x90,		// client is joining a clan
		SERVER_SEND_ADDRESS		= 0x91,		// Client is requesting it's true IP address
		SERVER_CHANGE_PASSWORD	= 0x93,		// client is changing it's password
		SERVER_CHANGE_NAME		= 0x94,		// client is changing it's username
		SERVER_CHANGE_EMAIL		= 0x95,		// client is changing it's email address
		SERVER_SET_STATUS		= 0x96,		// client is setting their status
		SERVER_SEND_FRIENDS		= 0x97,		// client is requesting their friend list
		SERVER_ADD_FRIEND		= 0x98,
		SERVER_DEL_FRIEND		= 0x99,
		SERVER_DEL_SELFFROMFRIEND = 0xb4,
		SERVER_SEND_IGNORES		= 0x9a,
		SERVER_ADD_IGNORE		= 0x9b,
		SERVER_DEL_IGNORE		= 0x9c,
		SERVER_BAN_USER			= 0x9d,
		SERVER_BAN_SERVER		= 0x9e,		
		SERVER_SEND_STAFFONLINE	= 0x9f,		// client is requesting the list of online staffmembers
		SERVER_SEND_WATCHLIST	= 0xac,		// client is requesting the watchlist
		SERVER_EDIT_WATCHLIST	= 0xae,		// client sends updated data for a watchlist entry

		SERVER_SEND_ROOMS		= 0xa0,
		SERVER_SEND_PLAYERS		= 0xa1,
		SERVER_CREATE_ROOM		= 0xa2,
		SERVER_JOIN_ROOM		= 0xa3,
		SERVER_LEAVE_ROOM		= 0xa4,
		SERVER_RECV_CHAT		= 0xa5,	

		SERVER_LOGOFF			= 0xaa,		
		SERVER_SELECT_GAME		= 0xab,
		SERVER_RECV_INSTALLER	= 0xb0,		// client is sending the installer

		// debug functions
		SERVER_RECV_MDBG		= 0xfb,		// client is sending MID debug info


		// utility functions
		SEND_TIME				= 0xfc,		// send the current time
		RECV_TIME				= 0xfd,		// receive the current time
		PING					= 0xfe,		// ping message
		PONG					= 0xff,		// response to ping
	};
	enum Result
	{
		LOGIN_ILLEGAL			= -5,		// user name contains illegal characters or words
		LOGIN_BANNED			= -4,		// user banned
		LOGIN_DUPLICATE_LOGIN	= -3,		// user already logged in
		LOGIN_FAILED			= -2,		// bad username or password
		LOGIN_ERROR				= -1,		// internal error occured
		LOGIN_OKAY				= 1,

		RESULT_TIMEOUT			= -1,
		RESULT_ERROR			= -1,
		RESULT_OKAY				= 1,

		FORCE_DWORD				= 0x7fffffff,
	};
	struct Game
	{
		dword				id;
		CharString			name;
		CharString			newlogin;		// newlogin url
		CharString			home;			// home url
		CharString			download;		// download url
		CharString			admin;			// admin url
		CharString			manual;			// manual url
		CharString			clans;			// clan url
		CharString			profile;		// profile url
		CharString			news;			// news url
		CharString			forum;			// forum url

		CharString			registry;		// registry 
		CharString			address;		// mirror-server address and port
		int					port;		
		int					chatRoom;		// default chat room

		Game &				operator=( const Game & copy );
	};
	enum ServerType
	{
		META_SERVER				= 0x01,	
		MIRROR_SERVER			= 0x02,
		GAME_SERVER				= 0x03,
		GAME_SUB_SERVER			= 0x04,
		PROCESS_SERVER			= 0x05,
		PROFILER_SERVER			= 0x07,
	};
	struct Server
	{
		dword				gameId;			// game id
		dword				id;				// server id
		dword				type;			// type of server, see ServerType
		dword				flags;			// server flags
		CharString			name;			// server name
		CharString			shortDescription;	// short description
		CharString			description;		// server description
		CharString			address;		// address of the server
		int					port;			// port of the server

		int					maxClients;		// maximum number of clients
		int					clients;		// current number of clients
		dword				lastUpdate;		// last time server sent update
		CharString			data;			// server specific data

		Server &			operator=( const Server & copy );
		bool				operator==( const Server & compare ) const;

		Server() : gameId( 0 ),
			id( 0 ),
			type( 0 ),
			flags( 0 ),
			port( 0 ),
			maxClients( 0 ),
			clients( 0 ),
			lastUpdate( 0 )
		{}
	};
	enum ProfileFlags
	{
		ADMINISTRATOR		= 0x00000001,			// community admin
		SERVER				= 0x00000002,			// server account
		MODERATOR			= 0x00000004,			// game moderator
		DEMO				= 0x00000010,			// user has demo access
		BETA				= 0x00000020,			// user has beta access
		SUBSCRIBED			= 0x00000040,			// user has release access
		CLAN_MEMBER			= 0x00000080,			// user is a valid member of the clan
		CLAN_ADMIN			= 0x00000100,			// user is the admin of their clan
		NEWS_ADMIN			= 0x00000200,			// DECPRECATED
		EDITOR				= 0x00000400,			// user can edit web pages
		DEVELOPER			= 0x00000800,			// user can add to development log
		EVENT				= 0x00001000,			// user can hide/stay hidden

		AWAY				= 0x00010000,			// user is currently away
		MUTED				= 0x00020000,			// user is currently muted
		HIDDEN				= 0x00040000,			// user is hidden


		REGISTRATION		= DEMO | BETA | SUBSCRIBED,
		SUPER_ADMIN			= ADMINISTRATOR|SERVER|MODERATOR|EDITOR|DEVELOPER|REGISTRATION,
		STAFF				= ADMINISTRATOR|MODERATOR|DEVELOPER,
	};
	struct ShortProfile
	{
		ShortProfile() : userId( 0 ), flags( 0 )
		{}

		dword				userId;
		CharString			name;
		CharString			status;
		dword				flags;

		ShortProfile &		operator=( const ShortProfile & copy );
	};
	struct Field
	{
		CharString			name;				// name of this field
		dword				id;					// field id+
		CharString			value;				// value of the field converted to text

		Field &				operator=( const Field & copy );
	};
	struct Profile
	{
		Profile() : userId( 0 ),
			sessionId( 0 ),
			clanId( 0 ),
			flags( 0 ),
			score( 0.0f ),
			widgetId( 0 )
		{}

		dword				userId;			// users id
		dword				sessionId;		// users session id
		dword				clanId;			// clan id
		dword				flags;			// access flags

		CharString			name;	
		CharString			email;	
		CharString			status;

		Array< Field >		fields;			// fields
		float				score;			
		CharString			rank;
		dword				widgetId;		// user widget

		Field *				findField( const char * pName );				// returns NULL if field not found!
		const Field *		findField( const char * pName ) const;			// returns NULL if field not found!

		Profile &			operator=( const Profile & copy );
		DLL CharString &	operator[]( const char * pName );
		DLL const CharString &	operator[]( const char * pName ) const;
	};
	struct ShortClan
	{
		int					clanId;			// clan id
		CharString			name;			// clan name
		CharString			home;			// clan homepage

		ShortClan & operator=( const ShortClan & copy );
	};
	enum ClanFlags
	{
		CLAN_VALID			= 0x01,
		CLAN_CLOSED			= 0x02,
	};
	struct Clan
	{
		dword				clanId;			// clan id
		dword				flags;
		CharString			name;			// clan name
		CharString			longName;		// full clan name
		CharString			motto;			// clan motto i.e. "Death from above!"
		CharString			home;			// clan home page
		int					faction;		// clan faction

		Binary				data;			// game specific information
		float				score;			// current clan score

		Clan & operator=( const Clan & copy );
	};
	struct WatchList
	{
		dword				watchId;		// internal unique watchlist id
		dword				userId;			// id of the added user
		CharString			userName;		// name of the added user, at the time of being added
		CharString			currUsername;	// current name of the added user
		dword				addedBy;		// userId of the user who added him
		CharString			addedByName;	// current username of the user who added him
		dword				addedTime;		// standard 32-bit date/time the user got added ( encoded in seconds )
		dword				watchType;		// type: 0 - watch, 1 - kick, 2 - ban
		CharString			watchReason;	// reason for being added to the list
		CharString			postingLink;	// optional link to a forumposting which holds more details on the action commited
		CharString			userIP;			// users IP at the time the entry was created
		CharString			userMID;		// users MID at the time the entry was created
		bool				isActive;		// the active flag can be cleared so the user is no longer shown in the standard-list
		
		WatchList & operator=( const WatchList & copy );
	};
	struct WatchListQuery
	{
		CharString			namePattern;	// partial name to be searched ( current and changed names ). Leave blank for no namesearch
		dword				userId;			// search by id, set 0 to disable search by id
		bool				watch;			// also show watch-type entrys
		bool				kick;			// also show kick-type entrys
		bool				ban;			// also show ban-type entrys
		bool				linkMissing;	// only entrys which don´t have a link set
		bool				onlyActive;		// only entrys with active flag set
		dword				limit;			// only last n entries
		
		WatchListQuery & operator=( const WatchListQuery & copy );
	};
	enum Language
	{
		ENGLISH			= 1,
		GERMAN			= 2,
		FRENCH			= 3,
		SPANISH			= 4,
		CZECH			= 5,
		DUTCH			= 6,
		CHINESE			= 7,
		KOREAN			= 8,
		JAPANESE		= 9,
	};
	enum RoomFlags
	{
		FLAG_ROOM_STATIC		= 0x1,		// room is never removed when empty
		FLAG_ROOM_MODERATED		= 0x2,		// room is moderated
		FLAG_ROOM_PASSWORD		= 0x4,		// room requires a password to enter
		FLAG_ROOM_PRIVATE		= 0x8,		// room is hidden from view by default
	};
	struct Room
	{
		dword				roomId;
		CharString			name;
		dword				members;
		dword				language;
		dword				flags;

		Room & operator=( const Room & copy );
	};
	struct Chat
	{
		dword				recpId;
		dword				roomId;
		CharString			author;
		dword				authorId;
		dword				time;
		CharString			text;

		Chat & operator=( const Chat & copy );
	};

	// Construction
	MetaClient();
	virtual ~MetaClient();

	// Client interface
	void				onConnect();										// called when connection made
	void				onReceive( byte message, const InStream & );				// called on incoming data
	void				onDisconnect();										// called when connection lost
	
	// Accessors
	bool				connected() const;									// are we connected
	bool				loggedIn() const;									// are we logged in
	const char *		publicKey() const;									// the public key

	const Profile &		profile() const;									// get my profile
	const Game &		game() const;										// get current game
	dword				gameId() const;										// get current gameid

	int					chatCount() const;
	const Chat &		chat( int n ) const;
	int					roomCount() const;									// current number of rooms we belong
	dword				room( int n ) const;								// get room n ID

	bool				inRoom( dword a_nRoomId ) const;					// are we joined to the given room ID already

	int					playerCount( dword a_nRoomId ) const;				// number of players in the given room
	const ShortProfile &
						player( dword a_nRoomId, int n ) const;				// get player n information

	bool				player( dword a_nRoomId, dword userId,
							MetaClient::ShortProfile & player ) const;		// get player information by uId
	bool				isIgnored( dword a_nUserId ) const;					// returns true if user is ignored by this user

	// Mutators
	int					open();												// connect to the best meta-server
	int					open( const char * serverAddress, int serverPort );	// connect to meta-server

	int					login( const char * pID, const char * pPW );		// login into the server
	int					login( dword sessionId );							// login with sessionId

	int					loginByProxy( const char * pID, 
							const char * pMD5,
							Profile & profile );							// indirect login through another server
	int					loginByProxy( dword sessionId, 
							Profile & profile );						
	void				logoff();
	
	//! Load the given word filter from disk, if none is loaded then chat will not be filtered.
	bool				loadWordFilter( const char * a_pFilter );
	void				clearWordFilter();
	//! Filter the passed in text against the currently loaded word filters
	void				filterWords( CharString & a_sText );

	int					create( const Profile & newProfile, 
							const char * pPW );								// create new login, additional information on error

	int					getGames( Array< Game > & games );					// get the game list
	int					selectGame( dword gameId );							// selects a game
	int					getServers( const char * pName, dword gameId, dword type, 
							Array< Server > & servers );					// get the a server list for a specific game
	int					registerServer( Server & server );					// creates/updates a game, set's the game.id value
	int					removeServer();										// unregisters our game

	int					editWatchList( int command, dword watchId,
							const char * pOldText, const char * pNewText,
							bool bOldStatus = false );						// Send updated data for a watchlist entry
	int					getWatchList( WatchListQuery wQuery,				// returns the part of the watchlist,
							Array< WatchList  > & watchList );				// specified by wQuery from the database
	int					getProfiles( const char * pName, 
							Array< ShortProfile > & profiles );				// search for profiles
	int					getProfileBySession( dword sessionId,
							Profile & profile );
	int					getProfile( dword userId,
							Profile & profile );
	int					putProfile( const Profile & profile );				// update a user profile
	dword				putProfileJob( const Profile & profile );			// same as putProfile(), but returns a job ID
	bool				addProfile( dword userId, dword fieldId, 
							float value );									// send incremental update to a player profile
	bool				updateScore( dword userId, float fScore,
							const char * pRank );							// send the final score/rank for a given player..

	int					getClans( const char * pName,
							Array< ShortClan > & clans );			
	int					getClan( dword clanId, 
							Clan & clan );
	int					getClan( dword clanId,
							Array< ShortProfile > & members );
	int					putClan( const Clan & clan );					
	int					createClan( const Clan & clan );
	int					joinClan( dword clanId );
	int					promoteClanMember(dword clanID, dword promotee);
	int					recruitClanMember(dword clanID, dword promotee);

	int					changePassword( const char * pPw );
	int					changeName( const char * pName );
	int					changeEmail( const char * pEmail );

	void				sendStatus( dword userId, const char * pStatus );
	void				sendStatus( const char * pStatus );

	int					getFriends( Array< ShortProfile > & friends );
	int					addFriend( dword userId );
	int					deleteFriend( dword userId );
	int					deleteSelfFromFriend( const char * who );			// remove yourself from another players friendlist

	int					getIgnores( Array< ShortProfile > & ignores );
	int					addIgnore( dword userId );
	int					deleteIgnore( dword userId );

	int					getStaffOnline( Array< ShortProfile > & staff );

	int					banServer( dword serverId, dword duration );

	int					getRooms( Array< Room > & rooms );					// get a list of rooms in the current lobby
	int					getPlayers( dword roomId, 
							Array< ShortProfile > & players );				// get players in a room

	dword				joinBestRoom();										// find the default best room and join it, returns the roomId
	int					createRoom( const char * pName,
							const char * pPW,
							bool a_bStatic = false,							// static rooms are not removed automatically when empty
							bool a_bModerated = false,						// moderated rooms default everyone to muted
							bool a_bPrivate = false );				// create a new room, optionally with a password
	dword				joinRoom( dword roomId, const char * pPW = NULL );				// join an existing room, returns the roomId or 0 on failure
	int					leaveRoom( dword nRootId = 0 );						// leave all rooms or a specific room

	bool				waitPlayers( dword a_nRoomId, bool block = true );	// wait for change in player list

	int					putData( dword userId, const Binary & data );
	dword				putDataJob( dword userId, const Binary & data );
	int					getData( dword userId, Binary & data );
	void				deleteData( dword userId );

	dword				getTime();											// get the server current time

	// MetaClientCommand.cpp
	void				sendChat( dword roomId, dword recpId, 
							const char * pMessage, bool sendLocal = true );	// send chat
	void				sendChat( dword roomId, 
							const char * pMessage );	// send chat
	void				sendPrivateChat( dword recpId, 
							const char * pMessage );						// send chat
	void				sendGlobalChat( const char * pMessage );			// send global chat
	void				sendLocalChat( const char * pMessage );				// send chat to self only

	void				sendLocalChat( dword roomId, dword recpId, 
							const char * pMessage );						// send chat to self only

	void				sendChatWhisper( const char * pMessage,
							const ShortProfile & pSProfReceiver );			// whisper to someone
	void				sendChatSend( const char * pMessage,
							const ShortProfile & pSProfReceiver );			// 'send' a message to someone
	void				flushChat();										// remove all current chat messages
	bool 				waitChat( bool block = true );						// wait for incoming chat message

	// MetaClientMID.cpp
	int					mdbg();												// debug function to get extended info on the MID

	// Helpers
	int					getAddress( CharString & address );						// get my external IP address
	bool				findRoom( const char * pString, Room & found );					// find room by name

	// Static
	static dword		getSessionId( const char * pUser, const char * pPW );
	static dword		version();									

	static dword		LanguageToLCID( Language nLang );
	static Language		LCIDToLanguage( dword nLCID );

	static const char *	populationText( int nClients, int nMaxClients );		// returns a generic description of the population level

private:
	//! Types
	typedef std::set< dword >		IgnoreSet;
	typedef std::map< CharString, CharString >		
									WordFilter;

	struct ChatRoom 
	{
		dword				m_nRoomId;
		Array< ShortProfile >
							m_Players;
		Event				m_PlayersUpdated;
	};
	typedef std::map< dword, ChatRoom >	ChatRoomMap;

	// Data
	bool				m_Connected;	
	bool				m_LoggedIn;
	
	CharString			m_PublicKey;
	Profile				m_Profile;											// my profile
	Game				m_Game;

	WordFilter			m_WordFilter;										// our language filter
	Array< Chat >		m_Chat;												// received chat messages!
	Event				m_ChatReceived;										
	ChatRoomMap			m_ChatRoomMap;

	int					m_ServerTimeDiff;									// add this to the local time to get the server time
	IgnoreSet			m_Ignored;											// set of user ID's we are currently ignoring

	static dword		s_CurrentVersion;									// current version of this client

	// MetaClientMID.cpp
	static CharString	getMID();

	// Helpers
	static CharString	flagsText( dword flags );
	static CharString	profileText( const Array< ShortProfile > & profiles );

	bool				findClan( const char * pPattern, Array< ShortClan > & clans );
	bool				findUser( const char * pPattern, Array< ShortProfile > & found );
	bool				findUserExactFirst( const char * pPattern, Array< ShortProfile > & found );
	bool				findRoomUser( const char * pPattern, Array< ShortProfile > & found );
};

//----------------------------------------------------------------------------

inline bool MetaClient::connected() const
{
	return( m_Connected );
}

inline bool MetaClient::loggedIn() const
{
	return( m_LoggedIn );
}

inline const char * MetaClient::publicKey() const
{
	return( m_PublicKey );
}

inline const MetaClient::Profile & MetaClient::profile() const
{
	return m_Profile;
}

inline const MetaClient::Game & MetaClient::game() const
{
	return m_Game;
}

inline dword MetaClient::gameId() const
{
	return m_Game.id;
}

inline int MetaClient::chatCount() const
{
	return m_Chat.size();
}

inline const MetaClient::Chat & MetaClient::chat( int n ) const
{
	return m_Chat[ n ];
}

inline int MetaClient::roomCount() const
{
	return m_ChatRoomMap.size();
}

inline dword MetaClient::room( int n ) const
{
	ChatRoomMap::const_iterator iRoom = m_ChatRoomMap.begin();
	for( ;iRoom != m_ChatRoomMap.end() && n > 0; ++iRoom );

	return iRoom != m_ChatRoomMap.end() ? iRoom->second.m_nRoomId : 0;
}

inline bool MetaClient::inRoom( dword a_nRoomId ) const
{
	AutoLock lock( const_cast<CriticalSection *>( &m_Lock ) );
	ChatRoomMap::const_iterator iRoom = m_ChatRoomMap.find( a_nRoomId );
	return iRoom != m_ChatRoomMap.end();
}

inline int MetaClient::playerCount( dword a_nRoomId ) const
{
	ChatRoomMap::const_iterator iRoom = m_ChatRoomMap.find( a_nRoomId );
	if ( iRoom != m_ChatRoomMap.end() )
		return iRoom->second.m_Players.size();

	return 0;
}

inline const MetaClient::ShortProfile & MetaClient::player( dword a_nRoomId, int n ) const
{
	ChatRoomMap::const_iterator iRoom = m_ChatRoomMap.find( a_nRoomId );
	if ( iRoom != m_ChatRoomMap.end() )
		return iRoom->second.m_Players[ n ];

	static ShortProfile INVALID;
	return INVALID;
}

inline bool MetaClient::player( dword a_nRoomId, dword a_nUserId, MetaClient::ShortProfile & player ) const
{
	ChatRoomMap::const_iterator iRoom = m_ChatRoomMap.find( a_nRoomId );
	if ( iRoom != m_ChatRoomMap.end() )
	{
		const Array<ShortProfile> & players = iRoom->second.m_Players;
		for(int i=0;i<players.size();i++)
			if ( players[i].userId == a_nUserId )
			{
				player = players[i];
				return true;
			}
	}

	return false;
}

inline bool MetaClient::isIgnored( dword a_nUserId ) const
{
	return m_Ignored.find( a_nUserId ) != m_Ignored.end();
}

//----------------------------------------------------------------------------

inline const InStream & operator>>( const InStream & input, MetaClient::Game & game )
{
	input >> game.id;
	input >> game.name;
	input >> game.newlogin;
	input >> game.home;
	input >> game.download;
	input >> game.admin;
	input >> game.manual;
	input >> game.clans;
	input >> game.profile;
	input >> game.news;
	input >> game.forum;
	input >> game.registry;
	input >> game.address;
	input >> game.port;
	input >> game.chatRoom;
	return input;
}

inline const OutStream & operator<<( const OutStream & output, const MetaClient::Game & game )
{
	output << game.id;
	output << game.name;
	output << game.newlogin;
	output << game.home;
	output << game.download;
	output << game.admin;
	output << game.manual;
	output << game.clans;
	output << game.profile;
	output << game.news;
	output << game.forum;
	output << game.registry;
	output << game.address;
	output << game.port;
	output << game.chatRoom;
	return output;
}

inline MetaClient::Game & MetaClient::Game::operator=( const Game & copy )
{
	id			= copy.id;
	name		= copy.name;
	newlogin	= copy.newlogin;
	home		= copy.home;
	download	= copy.download;
	admin		= copy.admin;
	manual		= copy.manual;
	clans		= copy.clans;
	profile		= copy.profile;
	news		= copy.news;
	forum		= copy.forum;
	registry	= copy.registry;
	address		= copy.address;
	port		= copy.port;
	chatRoom	= copy.chatRoom;
	return *this;
}

//----------------------------------------------------------------------------

inline const InStream & operator>>( const InStream & input, MetaClient::Server & server )
{
	input >> server.gameId;
	input >> server.id;
	input >> server.type;
	input >> server.flags;
	input >> server.name;
	input >> server.shortDescription;
	input >> server.description;
	input >> server.address;
	input >> server.port;
	input >> server.maxClients;
	input >> server.clients;
	input >> server.lastUpdate;
	input >> server.data;
	return input;
}

inline const OutStream & operator<<( const OutStream & output, const MetaClient::Server & server )
{
	output << server.gameId;
	output << server.id;
	output << server.type;
	output << server.flags;
	output << server.name;
	output << server.shortDescription;
	output << server.description;
	output << server.address;
	output << server.port;
	output << server.maxClients;
	output << server.clients;
	output << server.lastUpdate;
	output << server.data;
	return output;
}

inline MetaClient::Server & MetaClient::Server::operator=( const Server & copy )
{
	gameId				= copy.gameId;
	id					= copy.id;
	type				= copy.type;
	flags				= copy.flags;
	name				= copy.name;
	shortDescription	= copy.shortDescription;
	description			= copy.description;
	address				= copy.address;
	port				= copy.port;
	maxClients			= copy.maxClients;
	clients				= copy.clients;
	lastUpdate			= copy.lastUpdate;
	data				= copy.data;
	return *this;
}

inline bool MetaClient::Server::operator==( const Server & compare ) const
{
	return( address == compare.address && port == compare.port );
}

//----------------------------------------------------------------------------

inline MetaClient::ShortProfile & MetaClient::ShortProfile::operator=( const MetaClient::ShortProfile & copy )
{
	userId = copy.userId;
	name = copy.name;
	status = copy.status;
	flags = copy.flags;
	return *this;
}

inline const InStream & operator>>( const InStream & input, MetaClient::ShortProfile & profile )
{
	input >> profile.userId;
	input >> profile.name;
	input >> profile.status;
	input >> profile.flags;
	return input;
}

inline const OutStream & operator<<( const OutStream & output, const MetaClient::ShortProfile & profile )
{
	output << profile.userId;
	output << profile.name;
	output << profile.status;
	output << profile.flags;
	return output;
}

//----------------------------------------------------------------------------

inline const InStream & operator>>( const InStream & input, MetaClient::Field & field )
{
	input >> field.id;
	input >> field.name;
	input >> field.value;
	return input;
}

inline const OutStream & operator<<( const OutStream & output, const MetaClient::Field & field )
{
	output << field.id;
	output << field.name;
	output << field.value;
	return output;
}

inline MetaClient::Field & MetaClient::Field::operator=( const Field & copy )
{
	id = copy.id;
	name = copy.name;
	value = copy.value;
	return *this;
}

//----------------------------------------------------------------------------

inline MetaClient::Profile & MetaClient::Profile::operator=( const MetaClient::Profile & copy )
{
	userId = copy.userId;
	sessionId = copy.sessionId;
	clanId = copy.clanId;
	flags = copy.flags;
	name = copy.name;
	email = copy.email;
	status = copy.status;
	fields = copy.fields;
	score = copy.score;
	rank = copy.rank;
	widgetId = copy.widgetId;
	return *this;
}

inline const InStream & operator>>( const InStream & input, MetaClient::Profile & profile )
{
	input >> profile.userId;
	input >> profile.sessionId;
	input >> profile.clanId;
	input >> profile.flags;
	input >> profile.name;
	input >> profile.email;
	input >> profile.status;
	input >> profile.fields;
	input >> profile.score;
	input >> profile.rank;
	input >> profile.widgetId;
	return input;
}

inline const OutStream & operator<<( const OutStream & output, const MetaClient::Profile & profile )
{
	output << profile.userId;
	output << profile.sessionId;
	output << profile.clanId;
	output << profile.flags;
	output << profile.name;
	output << profile.email;
	output << profile.status;
	output << profile.fields;
	output << profile.score;
	output << profile.rank;
	output << profile.widgetId;
	return output;
}

inline MetaClient::Field * MetaClient::Profile::findField( const char * pName )
{
	for(int i=0;i<fields.size();++i)
	{
		if ( fields[i].name == pName )
			return &fields[i];
	}
	return NULL;
}

inline const MetaClient::Field * MetaClient::Profile::findField( const char * pName ) const
{
	for(int i=0;i<fields.size();++i)
	{
		if ( fields[i].name == pName )
			return &fields[i];
	}
	return NULL;
}

//----------------------------------------------------------------------------

inline MetaClient::ShortClan & MetaClient::ShortClan::operator=( const MetaClient::ShortClan & copy )
{
	clanId = copy.clanId;
	name = copy.name;
	home = copy.home;
	return *this;
}

inline const InStream & operator>>( const InStream & input, MetaClient::ShortClan & clan )
{
	input >> clan.clanId;
	input >> clan.name;
	input >> clan.home;
	return input;
}

inline const OutStream & operator<<( const OutStream & output, const MetaClient::ShortClan & clan )
{
	output << clan.clanId;
	output << clan.name;
	output << clan.home;
	return output;
}

//----------------------------------------------------------------------------

inline MetaClient::Clan & MetaClient::Clan::operator=( const MetaClient::Clan & copy )
{
	clanId = copy.clanId;
	flags = copy.flags;
	name = copy.name;
	longName = copy.longName;
	motto = copy.motto;
	home = copy.home;
	data = copy.data;
	score = copy.score;
	faction = copy.faction;
	return *this;
}

inline const InStream & operator>>( const InStream & input, MetaClient::Clan & clan )
{
	input >> clan.clanId;
	input >> clan.flags;
	input >> clan.name;
	input >> clan.longName;
	input >> clan.motto;
	input >> clan.home;
	input >> clan.data;
	input >> clan.score;
	input >> clan.faction;
	return input;
}

inline const OutStream & operator<<( const OutStream & output, const MetaClient::Clan & clan )
{
	output << clan.clanId;
	output << clan.flags;
	output << clan.name;
	output << clan.longName;
	output << clan.motto;
	output << clan.home;
	output << clan.data;
	output << clan.score;
	output << clan.faction;
	return output;
}

//----------------------------------------------------------------------------

inline MetaClient::WatchList & MetaClient::WatchList::operator=( const MetaClient::WatchList & copy )
{
	watchId		= copy.watchId;
	userId		= copy.userId;
	userName	= copy.userName;
	currUsername= copy.currUsername;
	addedBy		= copy.addedBy;
	addedTime	= copy.addedTime;
	addedByName = copy.addedByName;
	watchType	= copy.watchType;
	watchReason	= copy.watchReason;
	postingLink = copy.postingLink;
	userIP		= copy.userIP;
	userMID		= copy.userMID;
	isActive	= copy.isActive;
	return *this;
}

inline const InStream & operator>>( const InStream & input, MetaClient::WatchList & watchList )
{
	input >> watchList.watchId;	
	input >> watchList.userId;		
	input >> watchList.userName;	
	input >> watchList.currUsername;	
	input >> watchList.addedBy;		
	input >> watchList.addedTime;		
	input >> watchList.addedByName;		
	input >> watchList.watchType;	
	input >> watchList.watchReason;		
	input >> watchList.postingLink;
	input >> watchList.userIP;
	input >> watchList.userMID;
	input >> watchList.isActive;
	return input;		
}

inline const OutStream & operator<<( const OutStream & output, const MetaClient::WatchList & watchList )
{
	output << watchList.watchId;	
	output << watchList.userId;		
	output << watchList.userName;	
	output << watchList.currUsername;	
	output << watchList.addedBy;		
	output << watchList.addedTime;		
	output << watchList.addedByName;		
	output << watchList.watchType;	
	output << watchList.watchReason;		
	output << watchList.postingLink;
	output << watchList.userIP;
	output << watchList.userMID;
	output << watchList.isActive;
	return output;		
}

//----------------------------------------------------------------------------

inline MetaClient::WatchListQuery & MetaClient::WatchListQuery::operator=( const MetaClient::WatchListQuery & copy )
{
	namePattern	= copy.namePattern;
	userId		= copy.userId;
	watch		= copy.watch;
	kick		= copy.kick;
	ban			= copy.ban;
	linkMissing	= copy.linkMissing;
	onlyActive	= copy.onlyActive;
	limit		= copy.limit;
	return *this;
}

inline const InStream & operator>>( const InStream & input, MetaClient::WatchListQuery & watchListQuery )
{
	input >> watchListQuery.namePattern;
	input >> watchListQuery.userId;
	input >> watchListQuery.watch;
	input >> watchListQuery.kick;
	input >> watchListQuery.ban;
	input >> watchListQuery.linkMissing;
	input >> watchListQuery.onlyActive;
	input >> watchListQuery.limit;
	return input;
}

inline const OutStream & operator<<( const OutStream & output, MetaClient::WatchListQuery & watchListQuery )
{
	output << watchListQuery.namePattern;
	output << watchListQuery.userId;
	output << watchListQuery.watch;
	output << watchListQuery.kick;
	output << watchListQuery.ban;
	output << watchListQuery.linkMissing;
	output << watchListQuery.onlyActive;
	output << watchListQuery.limit;
	return output;
}

//----------------------------------------------------------------------------

inline MetaClient::Room & MetaClient::Room::operator=( const MetaClient::Room & copy )
{
	roomId = copy.roomId;
	name = copy.name;
	members = copy.members;
	language = copy.language;
	flags = copy.flags;
	return *this;
}

inline const InStream & operator>>( const InStream & input, MetaClient::Room & room )
{
	input >> room.roomId;
	input >> room.name;
	input >> room.members;
	input >> room.language;
	input >> room.flags;
	return input;
}

inline const OutStream & operator<<( const OutStream & output, const MetaClient::Room & room )
{
	output << room.roomId;
	output << room.name;
	output << room.members;
	output << room.language;
	output << room.flags;
	return output;
}

//----------------------------------------------------------------------------

inline MetaClient::Chat & MetaClient::Chat::operator=( const MetaClient::Chat & copy )
{
	recpId = copy.recpId;
	roomId = copy.roomId;
	author = copy.author;
	authorId = copy.authorId;
	time = copy.time;
	text = copy.text;
	return *this;
}

inline const InStream & operator>>( const InStream & input, MetaClient::Chat & msg )
{
	input >> msg.recpId;
	input >> msg.roomId;
	input >> msg.author;
	input >> msg.authorId;
	input >> msg.time;
	input >> msg.text;
	return input;
}

inline const OutStream & operator<<( const OutStream & output, const MetaClient::Chat & msg )
{
	output << msg.recpId;
	output << msg.roomId;
	output << msg.author;
	output << msg.authorId;
	output << msg.time;
	output << msg.text;
	return output;
}

//----------------------------------------------------------------------------



#endif

//-------------------------------------------------------------------------------
// EOF
