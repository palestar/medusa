/*
	MetaClient.cpp

	The MetaClient object is used to connect to a MetaServer, either to publish a game server or to get
	the list of all available game servers

	(c)1999 PaleStar Development, Richard Lyle
*/

#include "Debug/Assert.h"
#include "Network/FileSocket.h"
#include "Standard/MD5.h"
#include "Standard/Time.h"
#include "Standard/Registry.h"
#include "Standard/Settings.h"
#include "Standard/CommandLine.h"
#include "File/FileDisk.h"
#include "System/Locale.h"
#include "GCQ/MetaClient.h"

#if defined(_WIN32)
#include <windows.h>
#else

#define LANG_NEUTRAL                     0x00
#define LANG_INVARIANT                   0x7f

#define LANG_AFRIKAANS                   0x36
#define LANG_ALBANIAN                    0x1c
#define LANG_ARABIC                      0x01
#define LANG_ARMENIAN                    0x2b
#define LANG_ASSAMESE                    0x4d
#define LANG_AZERI                       0x2c
#define LANG_BASQUE                      0x2d
#define LANG_BELARUSIAN                  0x23
#define LANG_BENGALI                     0x45
#define LANG_BULGARIAN                   0x02
#define LANG_CATALAN                     0x03
#define LANG_CHINESE                     0x04
#define LANG_CROATIAN                    0x1a
#define LANG_CZECH                       0x05
#define LANG_DANISH                      0x06
#define LANG_DIVEHI                      0x65
#define LANG_DUTCH                       0x13
#define LANG_ENGLISH                     0x09
#define LANG_ESTONIAN                    0x25
#define LANG_FAEROESE                    0x38
#define LANG_FARSI                       0x29
#define LANG_FINNISH                     0x0b
#define LANG_FRENCH                      0x0c
#define LANG_GALICIAN                    0x56
#define LANG_GEORGIAN                    0x37
#define LANG_GERMAN                      0x07
#define LANG_GREEK                       0x08
#define LANG_GUJARATI                    0x47
#define LANG_HEBREW                      0x0d
#define LANG_HINDI                       0x39
#define LANG_HUNGARIAN                   0x0e
#define LANG_ICELANDIC                   0x0f
#define LANG_INDONESIAN                  0x21
#define LANG_ITALIAN                     0x10
#define LANG_JAPANESE                    0x11
#define LANG_KANNADA                     0x4b
#define LANG_KASHMIRI                    0x60
#define LANG_KAZAK                       0x3f
#define LANG_KONKANI                     0x57
#define LANG_KOREAN                      0x12
#define LANG_KYRGYZ                      0x40
#define LANG_LATVIAN                     0x26
#define LANG_LITHUANIAN                  0x27
#define LANG_MACEDONIAN                  0x2f   // the Former Yugoslav Republic of Macedonia
#define LANG_MALAY                       0x3e
#define LANG_MALAYALAM                   0x4c
#define LANG_MANIPURI                    0x58
#define LANG_MARATHI                     0x4e
#define LANG_MONGOLIAN                   0x50
#define LANG_NEPALI                      0x61
#define LANG_NORWEGIAN                   0x14
#define LANG_ORIYA                       0x48
#define LANG_POLISH                      0x15
#define LANG_PORTUGUESE                  0x16
#define LANG_PUNJABI                     0x46
#define LANG_ROMANIAN                    0x18
#define LANG_RUSSIAN                     0x19
#define LANG_SANSKRIT                    0x4f
#define LANG_SERBIAN                     0x1a
#define LANG_SINDHI                      0x59
#define LANG_SLOVAK                      0x1b
#define LANG_SLOVENIAN                   0x24
#define LANG_SPANISH                     0x0a
#define LANG_SWAHILI                     0x41
#define LANG_SWEDISH                     0x1d
#define LANG_SYRIAC                      0x5a
#define LANG_TAMIL                       0x49
#define LANG_TATAR                       0x44
#define LANG_TELUGU                      0x4a
#define LANG_THAI                        0x1e
#define LANG_TURKISH                     0x1f
#define LANG_UKRAINIAN                   0x22
#define LANG_URDU                        0x20
#define LANG_UZBEK                       0x43
#define LANG_VIETNAMESE                  0x2a

#endif

//----------------------------------------------------------------------------

dword	MetaClient::s_CurrentVersion = 20121205;	// current version of this client

//-------------------------------------------------------------------------------

MetaClient::MetaClient()
{
	m_Connected = false;
	m_LoggedIn = false;
	m_ServerTimeDiff = 0;
}

MetaClient::~MetaClient()
{
	logoff();
}

//----------------------------------------------------------------------------

void MetaClient::onConnect()
{
	m_Connected = true;
	m_LoggedIn = false;
	m_ChatRoomMap.clear();
}

/**
 *  Handles all incoming messages from the server
 **/
void MetaClient::onReceive( byte message, const InStream & input )
{
	switch( message )
	{
	case CLIENT_RECV_KEY:
		{
			dword job;
			input >> job;
			CharString key;
			input >> key;

			AutoLock lock( &m_Lock );
			m_PublicKey = key;
			signalJob( job, RESULT_OKAY );
		}
		break;
	case CLIENT_LOGIN:
		{
			dword job;
			input >> job;
			dword version;
			input >> version;

			int result;
			input >> result;
			if ( result == LOGIN_OKAY )
			{
				Profile profile;
				input >> profile;

				AutoLock lock( &m_Lock );
				m_LoggedIn = true;
				m_Profile = profile;
			}

			signalJob( job, result );
		}
		break;
	case CLIENT_RECV_GAMES:
		{
			dword job;
			input >> job;
			Array< Game > games;
			input >> games;

			AutoLock lock( &m_Lock );
			if ( jobResult( job ) != NULL )
				*((Array< Game > *)jobResult( job )) = games;
			signalJob( job, RESULT_OKAY );
		}
		break;
	case CLIENT_RECV_SERVERS:
		{
			dword job;
			input >> job;
			Array< Server > servers;
			input >> servers;

			AutoLock lock( &m_Lock );
			if ( jobResult( job ) != NULL )
				*((Array< Server > *)jobResult( job )) = servers;
			signalJob( job, RESULT_OKAY );
		}
		break;
	case CLIENT_RECV_ADDRESS:
		{
			dword job;
			input >> job;
			CharString address;
			input >> address;

			AutoLock lock( &m_Lock );
			if ( jobResult( job ) != NULL )
				*((CharString *)jobResult( job )) = address;

			signalJob( job, RESULT_OKAY );
		}
		break;
	case CLIENT_RECV_WATCHLIST:
		{
			dword job;
			input >> job;
			Array< WatchList > watchList;
			input >> watchList;

			AutoLock lock( &m_Lock );
			if ( jobResult( job ) != NULL )
				*((Array< WatchList > *)jobResult( job )) = watchList;

			signalJob( job, RESULT_OKAY );
		}
		break;
	case CLIENT_RECV_PROFILES:
		{
			dword job;
			input >> job;
			Array< ShortProfile > profiles;
			input >> profiles;

			AutoLock lock( &m_Lock );
			if ( jobResult( job ) != NULL )
				*((Array< ShortProfile > *)jobResult( job )) = profiles;

			signalJob( job, RESULT_OKAY );
		}
		break;
	case CLIENT_RECV_PROFILE:
		{
			dword job;
			input >> job;
			int result;
			input >> result;

			if ( result == RESULT_OKAY )
			{
				Profile profile;
				input >> profile;

				AutoLock lock( &m_Lock );
				if ( jobResult( job ) != NULL )
					*((Profile *)jobResult( job )) = profile;
			}
			signalJob( job, result );
		}
		break;
	case CLIENT_RECV_CLANS:
		{
			dword job;
			input >> job;
			Array< ShortClan > clans;
			input >> clans;

			AutoLock lock( &m_Lock );
			if ( jobResult( job ) != NULL )
				*((Array< ShortClan > *)jobResult( job )) = clans;

			signalJob( job, RESULT_OKAY );
		}
		break;
	case CLIENT_RECV_CLAN:
		{
			dword job;
			input >> job;
			int result;
			input >> result;

			if ( result == RESULT_OKAY )
			{
				Clan clan;
				input >> clan;

				AutoLock lock( &m_Lock );
				if ( jobResult( job ) != NULL )
					*((Clan *)jobResult( job )) = clan;
			}

			signalJob( job, result );
		}
		break;
	case CLIENT_JOB_DONE:
		{
			dword job;
			input >> job;
			int jobComplete;
			input >> jobComplete;

			signalJob( job, jobComplete );
		}
		break;
	case CLIENT_RECV_ROOMS:
		{
			dword job;
			input >> job;
			Array< Room > rooms;
			input >> rooms;

			AutoLock lock( &m_Lock );
			if ( jobResult( job ) != NULL )
				*((Array< Room > *)jobResult( job )) = rooms;

			signalJob( job, true );
		}
		break;
	case CLIENT_RECV_CHAT:
		{
			Chat msg;
			input >> msg;

			AutoLock lock( &m_Lock );
			if (! isIgnored( msg.authorId ) )
			{
				filterWords( msg.text );

				m_Chat.push( msg );
				m_ChatReceived.signal();
			}
		}
		break;
	case CLIENT_ADD_ROOM:
		{
			dword nRoom;
			input >> nRoom;
			Array< ShortProfile > players;
			input >> players;

			AutoLock lock( &m_Lock );

			ChatRoom & room = m_ChatRoomMap[ nRoom ];
			room.m_nRoomId = nRoom;
			room.m_Players = players;
			room.m_PlayersUpdated.signal();
		}
		break;
	case CLIENT_DEL_ROOM:
		{
			dword roomId;
			input >> roomId;
			
			AutoLock lock( &m_Lock );
			m_ChatRoomMap.erase( roomId );
		}
		break;
	case CLIENT_ADD_PLAYER:
		{
			dword roomId;
			input >> roomId;
			ShortProfile player;
			input >> player;

			AutoLock lock( &m_Lock );
			if ( m_ChatRoomMap.find( roomId ) != m_ChatRoomMap.end() )
			{
				ChatRoom & room = m_ChatRoomMap[ roomId ];

				bool bPlayerFound = false;
				for(int i=0;i<room.m_Players.size();i++)
					if ( room.m_Players[i].userId == player.userId )
					{
						room.m_Players[i] = player;
						bPlayerFound = true;
						break;
					}

				if (! bPlayerFound )
					room.m_Players.push( player );
				room.m_PlayersUpdated.signal();
			}
		}
		break;
	case CLIENT_DEL_PLAYER:
		{
			dword roomId;
			input >> roomId;
			dword userId;
			input >> userId;

			AutoLock lock( &m_Lock );
			if ( m_ChatRoomMap.find( roomId ) != m_ChatRoomMap.end() )
			{
				ChatRoom & room = m_ChatRoomMap[ roomId ];
				for(int i=0;i<room.m_Players.size();i++)
					if ( room.m_Players[i].userId == userId )
					{
						room.m_Players.removeSwap( i );
						break;
					}
				room.m_PlayersUpdated.signal();
			}
		}
		break;
	case PING:
		{
			dword tick;
			input >> tick;

			send( PONG ) << tick;
		}
		break;
	case CLIENT_RECV_GAME:
		{
			dword job;
			input >> job;
			Game game;
			input >> game;

			AutoLock lock( &m_Lock );
			m_Game = game;
			signalJob( job, game.id );
		}
		break;
	case CLIENT_RECV_SELF:
		{
			Profile profile;
			input >> profile;

			AutoLock lock( &m_Lock );
			m_Profile = profile;
		}
		break;
	case CLIENT_SEND_INSTALLER:
		{
			Registry registry( Registry::LOCAL, m_Game.registry );

			// firstly, make sure the game has been installed
			if ( registry.get( "VERSION", (dword)0) != 0 )
			{
				CharString installer( registry.get( "INSTALLER", "PALESTAR" ) );
				if ( installer.length() > 0 )
					send( SERVER_RECV_INSTALLER ) << installer;
			}
		}
		break;
	case SEND_TIME:
		{
			dword job;
			input >> job;

			send( RECV_TIME ) << job << Time::seconds();
		}
		break;
	case RECV_TIME:
		{
			dword job;
			input >> job;
			dword time;
			input >> time;

			signalJob( job, time );
		}
		break;
	}
}

void MetaClient::onDisconnect()
{
	m_Connected = false;
	m_LoggedIn = false;
	m_ChatRoomMap.clear();
}

//----------------------------------------------------------------------------

const char *	PRIMARY_SERVER_ADDRESS = "meta-server.palestar.com";
const int		PRIMARY_SERVER_PORT = 9000;

/**
 *  Initiates the connection sequence to the primary server.
 *  Determines the metaserver with the least load and connects to it.
 **/
int MetaClient::open()
{
	Settings settings( "GCQ" );

	// get the address and port of the primary server, which can be overridden from the config.ini file
	CharString	address = settings.get( "metaAddress", PRIMARY_SERVER_ADDRESS);
	int	port = settings.get( "metaPort", PRIMARY_SERVER_PORT);

	TRACE( "MetaClient::open, connecting to primary server..." );
	if ( open( address, port ) < 0 )
		return RESULT_ERROR;

	//Array< Server > servers;
	//if ( getServers( "", 0, META_SERVER, servers ) < 0 )
	//	return RESULT_ERROR;

	//bool primary = true;

	//TRACE( "MetaClient::open, got server list..." );
	//while( true )
	//{
	//	int bestServer = -1;
	//	float bestScore = 1.0f;

	//	for(int i=0;i<servers.size();i++)
	//	{
	//		float score = float( servers[i].clients ) / servers[i].maxClients;
	//		if ( score < bestScore )
	//		{
	//			bestScore = score;
	//			bestServer = i;
	//		}
	//	}

	//	if ( bestServer < 0 )
	//		return RESULT_ERROR;
	//	
	//	Server & server = servers[bestServer];
	//	if ( primary && server.address == address && server.port == port )
	//		break;		// already connected to the best server

	//	// disconnect from current server
	//	logoff();
	//	// no longer connected to the primary server
	//	primary = false;

	//	// connect to the best server
	//	if ( open( server.address, server.port ) == RESULT_OKAY )
	//		break;

	//	TRACE( "MetaClient::open, failed to open server, trying next server..." );
	//	servers.remove( bestServer );
	//}

	return RESULT_OKAY;
}

/**
 *  Connects to a specific Metaserver
 **/
int MetaClient::open( const char * serverAddress, int serverPort )
{
	m_Connected = false;

	// open connection to server
	if (! Client::open( new Socket("ZLIB"), serverAddress, serverPort ) )
		return RESULT_ERROR;

	// get the public key
	dword job = createJob();
	send( SERVER_SEND_KEY ) << job;

	// wait for the public key
	if ( waitJob( job ) != RESULT_OKAY )
		return RESULT_ERROR;

	return RESULT_OKAY;
}

/**
 *  Sends login request to the MetaServer using username and password.
 **/
int MetaClient::login( const char * pID, const char * pPW )
{
	if (! m_Connected )
		return RESULT_ERROR;
	m_LoggedIn = false;

	// create a new job
	dword job = createJob();
	// generate the private key
	CharString privateKey( MD5( CharString(pPW) ).checksum() );
	// on connection the ProfileServer sent us a random key, use this
	// random key to generate a md5 hash value with the real password (private key)
	CharString md5( MD5( CharString(m_PublicKey + privateKey) ).checksum() );
	// send the current time to the server, so it can calculate the timezone difference
	send( SERVER_LOGIN ) << job << version() << getMID() << CharString( pID ) << md5;

	int nResult = waitJob( job );
	if ( nResult != RESULT_OKAY )
		return nResult;

	Array< ShortProfile > ignores;
	if ( getIgnores( ignores ) != RESULT_OKAY )
		return RESULT_ERROR;

	m_Ignored.clear();
	for(int i=0;i<ignores.size();++i)
		m_Ignored.insert( ignores[i].userId );

	return RESULT_OKAY;
}

/**
 *  Login to the MetaServer by SessionId ( m_Profile.sessionid )
 *  Also submits the local MID stored in the registry so the users PC can be identified.
 **/

int MetaClient::login( dword sessionId )
{
	if (!m_Connected)
		return RESULT_ERROR;

	dword job = createJob();
	send( SERVER_LOGIN_SESSION ) << job << version() << getMID() << sessionId;

	int nResult = waitJob( job );
	if ( nResult != RESULT_OKAY )
		return nResult;

	Array< ShortProfile > ignores;
	if ( getIgnores( ignores ) != RESULT_OKAY )
		return RESULT_ERROR;

	m_Ignored.clear();
	for(int i=0;i<ignores.size();++i)
		m_Ignored.insert( ignores[i].userId );

	return RESULT_OKAY;
}


/**
 *  Indirect login by using another server. Uses username and password
 **/
int MetaClient::loginByProxy( const char * pID, const char * pMD5, Profile & profile )
{
	if (! m_LoggedIn )
		return RESULT_ERROR;

	dword job = createJob( &profile );
	send( SERVER_LOGIN_PROXY ) << job << false << CharString(pID) << CharString(pMD5);

	return waitJob( job );
}

/**
 *  Indirect login by using another server. Uses the SessionId ( m_Profile.sessionid )
 **/
int MetaClient::loginByProxy( dword sessionId, Profile & profile )
{
	if (!m_LoggedIn )
		return RESULT_ERROR;

	dword job = createJob( &profile );
	send( SERVER_LOGIN_PROXY ) << job << true << sessionId;

	return waitJob( job );
}

/**
 *  Sends logoff request to the server
 **/
void MetaClient::logoff()
{
	send( SERVER_LOGOFF );
	Client::update();
}

bool MetaClient::loadWordFilter( const char * a_pFilter )
{
	AutoLock lock( &m_Lock );
	m_WordFilter.clear();

	char * pFilters = FileDisk::loadTextFile( a_pFilter );
	if ( pFilters == NULL )
	{
		LOG_ERROR( "MetaClient", "Failed to load word filter from %s", a_pFilter );
		return false;
	}

	CharString sLine;

	CharString sParse( pFilters );
	while( sParse.tokenize( sLine, "\r\n" ) )
	{
		int nSeperator = sLine.find( '=' );
		if ( nSeperator < 0 )
			continue;
		
		CharString sKey( sLine );
		sKey.left( nSeperator );
		CharString sValue( sLine );
		sValue.mid( nSeperator + 1 );

		m_WordFilter[ sKey ] = sValue;		
	}

	FileDisk::unloadTextFile( pFilters );
	return true;
}

void MetaClient::clearWordFilter()
{
	AutoLock lock( &m_Lock );
	m_WordFilter.clear();
}

void MetaClient::filterWords( CharString & a_sText )
{
	AutoLock lock( &m_Lock );
	for( WordFilter::iterator iFilter = m_WordFilter.begin();
		iFilter != m_WordFilter.end(); ++iFilter )
	{
		a_sText.replace( iFilter->first, iFilter->second, true );
	}
}

int MetaClient::create( const Profile & newProfile, const char * pPW )
{
	if (!m_Connected)
		return RESULT_ERROR;

	// hash the password
	CharString privateKey( MD5( CharString( pPW ) ).checksum() );

	dword job = createJob();
	send( SERVER_LOGIN_CREATE ) << job << getMID() << newProfile << privateKey;

	return waitJob( job );
}

int MetaClient::getGames( Array< Game > & games )
{
	if (!m_Connected)
		return RESULT_ERROR;

	dword job = createJob( &games );
	send( SERVER_SEND_GAMES ) << job;

	return waitJob( job );
}

int MetaClient::selectGame( dword gameId )
{
	if (!m_LoggedIn)
		return RESULT_ERROR;

	dword job = createJob();
	send( SERVER_SELECT_GAME ) << job << gameId;		// server will send us an updated profile

	if ( waitJob( job ) < 0 )
		return RESULT_ERROR;							// failed to select game

	return RESULT_OKAY;
}

int MetaClient::getServers( const char * pName, dword gameId, dword type, Array< Server > & servers )
{
	if (!m_Connected)
		return RESULT_ERROR;

	dword job = createJob( &servers );
	send( SERVER_SEND_SERVERS ) << job << CharString( pName != NULL ? pName : "" ) << gameId << type ;

	return waitJob( job );
}

int MetaClient::registerServer( Server & server )
{
	if (!m_LoggedIn)
		return RESULT_ERROR;

	// send this game object to the server
	dword job = createJob();
	send( SERVER_REGISTER_SERVER2 ) << job << server;

	server.id = waitJob( job );
	return server.id;
}

int MetaClient::removeServer()
{
	if (!m_LoggedIn)
		return RESULT_ERROR;

	send( SERVER_REMOVE_SERVER );
	return RESULT_OKAY;
}

/**
 *  Returns the complete WatchList from the Database
 **/
int MetaClient::getWatchList( WatchListQuery wQuery, Array< WatchList > & watchList )
{
	if (!m_LoggedIn)
		return RESULT_ERROR;

	dword job = createJob( &watchList );
	send( SERVER_SEND_WATCHLIST ) << job << wQuery;

	return waitJob( job );
}

/**
 *	Send updated data for a watchlist entry, the old data is also transmitted
 *  to make sure it hasn´t been changed by another user in between to prevent dataloss
 **/
int MetaClient::editWatchList( int command, dword watchId, const char * pOldText, const char * pNewText, bool bOldStatus /*= false*/ )
{
	// 0 - set forumlink.
	// 1 - Toggle active.
	assert( command >= 0 && command <= 1 );
	
	if (!m_LoggedIn)
		return RESULT_ERROR;
	
	dword job = createJob();
	switch( command )
	{
		case 0: 
			send( SERVER_EDIT_WATCHLIST ) << job << command << watchId << CharString( pOldText ) << CharString( pNewText ); 
			break;
		case 1:	
			send( SERVER_EDIT_WATCHLIST ) << job << command << watchId << bOldStatus; 
			break;
	}

	return waitJob( job );
}

/**
 *  Returns the ShortProfile of all players matched by the playername searchpattern pName
 **/
int MetaClient::getProfiles( const char * pName, Array< ShortProfile > & profiles )
{
	if (!m_LoggedIn)
		return RESULT_ERROR;

	dword job = createJob( &profiles );
	send( SERVER_SEND_PROFILES ) << job << CharString( pName );

	return waitJob( job );
}

/**
 *  Returns the profile for the user matched by the sessionId
 **/
int MetaClient::getProfileBySession( dword sessionId, Profile & profile )
{
	if (!m_LoggedIn)
		return RESULT_ERROR;

	dword job = createJob( &profile );
	send( SERVER_SEND_PROFILE ) << job << true << sessionId;

	return waitJob( job );
}

/**
 *  Returns the profile for the user matched by the userId
 **/
int MetaClient::getProfile( dword userId, Profile & profile )
{
	if (!m_LoggedIn)
		return RESULT_ERROR;

	dword job = createJob( &profile );
	send( SERVER_SEND_PROFILE ) << job << false << userId;

	return waitJob( job );
}

/**
 *  Stores a Profile for a user (as set in the profile) within the game.
 *  Mainly used to edit a users profile.
 **/
int MetaClient::putProfile( const Profile & profile )
{
	return waitJob( putProfileJob( profile ) );
}

dword MetaClient::putProfileJob( const Profile & profile )
{
	if (!m_LoggedIn)
		return 0;

	dword job = createJob();
	send( SERVER_RECV_PROFILE ) << job << profile;

	return job;
}

bool MetaClient::addProfile( dword userId, dword fieldId, float value )
{
	if (!m_LoggedIn )
		return false;

	send( SERVER_ADD_PROFILE ) << userId << fieldId << value;
	return true;
}

bool MetaClient::updateScore( dword userId, float fScore, const char * pRank )
{
	if (! m_LoggedIn )
		return false;

	send( SERVER_UPDATE_SCORE ) << userId << fScore << CharString( pRank );
	return true;
}

int MetaClient::getClans( const char * pName, Array< ShortClan > & clans )
{
	if (!m_LoggedIn)
		return RESULT_ERROR;

	dword job = createJob( &clans );
	send( SERVER_SEND_CLANS ) << job << CharString( pName );

	return waitJob( job );
}

int MetaClient::getClan( dword clanId, Clan & clan )
{
	if (!m_LoggedIn)
		return RESULT_ERROR;

	dword job = createJob( &clan );
	send( SERVER_SEND_CLAN ) << job << clanId;

	return waitJob( job );
}

int MetaClient::getClan( dword clanId, Array< ShortProfile > & members )
{
	if (!m_LoggedIn)
		return RESULT_ERROR;

	dword job = createJob( &members );
	send( SERVER_SEND_MEMBERS ) << job << clanId;

	return waitJob( job );
}

int MetaClient::recruitClanMember(dword clanID, dword promotee)
{
	if(!m_LoggedIn)
		return RESULT_ERROR;

	dword job = createJob();
	send ( SERVER_CLAN_RECRUIT ) << job << clanID << promotee;

	return waitJob( job );
}

int MetaClient::promoteClanMember(dword clanID, dword promotee)
{
	if(!m_LoggedIn)
		return RESULT_ERROR;

	dword job = createJob();
	send ( SERVER_CLAN_PROMOTE ) << job << clanID << promotee;

	return waitJob( job );
}

int MetaClient::putClan( const Clan & clan )
{
	if (!m_LoggedIn)
		return RESULT_ERROR;

	dword job = createJob();
	send( SERVER_RECV_CLAN ) << job << clan;

	return waitJob( job );
}

int MetaClient::createClan( const Clan & clan )
{
	if (!m_LoggedIn)
		return RESULT_ERROR;

	dword job = createJob();
	send( SERVER_CREATE_CLAN ) << job << clan;

	return waitJob( job );
}

int MetaClient::joinClan( dword clanId )
{
	if (!m_LoggedIn)
		return RESULT_ERROR;

	dword job = createJob();
	send( SERVER_JOIN_CLAN ) << job << clanId;

	return waitJob( job );
}

int MetaClient::changePassword( const char * pPw )
{
	if (!m_LoggedIn)
		return RESULT_ERROR;

	dword job = createJob();
	send( SERVER_CHANGE_PASSWORD ) << job << CharString( MD5( CharString( pPw ) ).checksum() );

	return waitJob( job );
}

int MetaClient::changeName( const char * pName )
{
	if (!m_LoggedIn)
		return RESULT_ERROR;

	dword job = createJob();
	send( SERVER_CHANGE_NAME ) << job << CharString( pName );

	int result = waitJob( job );
	if(  result > 0 )
		m_Profile.name = pName;

	return result;
}

int MetaClient::changeEmail( const char * pEmail )
{
	if (!m_LoggedIn)
		return RESULT_ERROR;

	dword job = createJob();
	send( SERVER_CHANGE_EMAIL ) << job << CharString( pEmail );

	int result = waitJob( job );
	if(  result > 0 )
		m_Profile.email = pEmail;

	return result;
}

void MetaClient::sendStatus( dword userId, const char * pStatus )
{
	if (!m_LoggedIn)
		return;

	send( SERVER_SET_STATUS ) << userId << CharString( pStatus );
}

void MetaClient::sendStatus( const char * pStatus )
{
	sendStatus( 0, pStatus );
}

int	MetaClient::getFriends( Array< ShortProfile > & friends )
{
	if (!m_LoggedIn)
		return RESULT_ERROR;

	dword job = createJob( &friends );
	send( SERVER_SEND_FRIENDS ) << job;

	return waitJob( job );
}

int MetaClient::addFriend( dword userId )
{
	if (!m_LoggedIn)
		return RESULT_ERROR;

	dword job = createJob();
	send( SERVER_ADD_FRIEND ) << job << userId;

	return waitJob( job );
}

int MetaClient::deleteFriend( dword userId )
{
	if (!m_LoggedIn)
		return RESULT_ERROR;

	dword job = createJob();
	send( SERVER_DEL_FRIEND ) << job << userId;

	return waitJob( job );
}

int	MetaClient::deleteSelfFromFriend( const char * who )
{
	if (!m_LoggedIn)
		return RESULT_ERROR;

	dword job = createJob();
	send( SERVER_DEL_SELFFROMFRIEND ) << job << CharString( who);

	return waitJob( job );
}

int MetaClient::getIgnores( Array< ShortProfile > & ignores )
{
	if (!m_LoggedIn)
		return RESULT_ERROR;

	dword job = createJob( &ignores );
	send( SERVER_SEND_IGNORES ) << job;

	return waitJob( job );
}

int MetaClient::addIgnore( dword userId )
{
	if (!m_LoggedIn)
		return RESULT_ERROR;

	dword job = createJob();
	send( SERVER_ADD_IGNORE ) << job << userId;

	if ( waitJob( job ) < 0 )
		return RESULT_ERROR;

	m_Ignored.insert( userId );
	return RESULT_OKAY;
}

int MetaClient::deleteIgnore( dword userId )
{
	if (!m_LoggedIn)
		return RESULT_ERROR;

	m_Ignored.erase( userId );

	dword job = createJob();
	send( SERVER_DEL_IGNORE ) << job << userId;

	return waitJob( job );
}

int	MetaClient::getStaffOnline( Array< ShortProfile > & staff )
{
	if (!m_LoggedIn)
		return RESULT_ERROR;

	dword job = createJob( &staff );
	send( SERVER_SEND_STAFFONLINE ) << job;

	return waitJob( job );
}

int MetaClient::banServer( dword serverId, dword duration )
{
	if (!m_LoggedIn)
		return RESULT_ERROR;

	dword job = createJob();
	send( SERVER_BAN_SERVER ) << job << serverId << duration;

	return waitJob( job );
}

int MetaClient::getRooms( Array< Room > & rooms )
{
	if (!m_LoggedIn)
		return RESULT_ERROR;

	dword job = createJob( &rooms );
	send( SERVER_SEND_ROOMS ) << job;

	return waitJob( job );
}

int MetaClient::getPlayers( dword roomId, Array< ShortProfile > & players )
{
	if (!m_LoggedIn)
		return RESULT_ERROR;

	dword job = createJob( &players );
	send( SERVER_SEND_PLAYERS ) << job << roomId;

	return waitJob( job );
}

dword MetaClient::joinBestRoom()
{
	dword nBestRoom = 0;

	// get the current chat rooms, and join the correct language room
	Array< Room > rooms;
	if ( getRooms( rooms ) < 0 )
		return 0;							// failed to get rooms, return success anyway

	// find a room & language thats best for this user
	// primary language bits only, thats why we AND with 0x1ff
	CharString sRoomName;
	for(int i=0;i<rooms.size();i++)
	{
		Room & room = rooms[i];
		if ( (room.language == LCIDToLanguage( Locale::locale().LCID() ) 
			&& nBestRoom == 0) && (room.flags & FLAG_ROOM_PASSWORD) == 0 )
		{
			nBestRoom = rooms[i].roomId;
			sRoomName = rooms[i].name;
		}
	}

	if ( nBestRoom != 0 )
	{
		dword nRoomId = joinRoom( nBestRoom, "" );
		if ( nRoomId != 0 )
			sendStatus( CharString().format("Chatting in '%s'", sRoomName.cstr() ) );

		return nRoomId;
	}

	return 0;
}


int MetaClient::createRoom( const char * pName, const char * pPW, 
							bool a_bStatic /*= false*/,
							bool a_bModerated /*= false*/,
							bool a_bPrivate /*= false*/ )
{
	if (!m_LoggedIn)
		return RESULT_ERROR;

	CharString md5( strlen( pPW ) > 0 ? MD5( CharString( pPW ) ).checksum() : "" );

	dword job = createJob();
	send( SERVER_CREATE_ROOM ) << job << CharString( pName ) << md5 << a_bStatic << a_bModerated << a_bPrivate;

	return waitJob( job );
}

dword MetaClient::joinRoom( dword roomId, const char * pPW /*= NULL*/ )
{
	if (!m_LoggedIn)
		return 0;

	CharString md5( (pPW != NULL && strlen( pPW ) > 0) ? MD5( CharString( pPW ) ).checksum() : "" );

	dword job = createJob();
	send( SERVER_JOIN_ROOM ) << job << roomId << md5;

	return (dword)waitJob( job );
}

int MetaClient::leaveRoom( dword a_nRoomId /*= 0*/ )
{
	if (!m_LoggedIn)
		return RESULT_ERROR;

	if ( a_nRoomId == 0 )
	{
		AutoLock lock( &m_Lock );
		for( ChatRoomMap::iterator iRoom = m_ChatRoomMap.begin(); 
			iRoom != m_ChatRoomMap.end(); ++iRoom )
		{
			if ( iRoom->first != 0 )
				leaveRoom( iRoom->first );
		}

		return RESULT_OKAY;
	}

	send( SERVER_LEAVE_ROOM ) << a_nRoomId;
	return RESULT_OKAY;
}

//----------------------------------------------------------------------------

/**
 *  Checks/Waits for updates on the list of players in the current room
 *  block: blocking/nonblocking mode
 **/
bool MetaClient::waitPlayers( dword a_nRoomId, bool a_bBlock /*= true*/ )
{
	AutoLock lock( &m_Lock );
	ChatRoomMap::iterator iRoom = m_ChatRoomMap.find( a_nRoomId );
	if ( iRoom == m_ChatRoomMap.end() )
		return true;

	if ( a_bBlock )
	{
		// make a copy  so we can release our lock..
		Event waitEvent = iRoom->second.m_PlayersUpdated;
		lock.release();

		waitEvent.wait();
		return true;
	}

	return iRoom->second.m_PlayersUpdated.signaled();
}

//----------------------------------------------------------------------------


// gets the current time from the server
dword MetaClient::getTime()
{
	dword job = createJob();
	send( SEND_TIME ) << job;

	int result = waitJob( job );
	if ( result <= 0 )
		return 0;		// error or timeout occured

	// successfully received servertime. Use it to sync local chat.
	m_ServerTimeDiff = result - Time::seconds();

	return result;
}

//----------------------------------------------------------------------------

/**
 *  Returns the external IP address of the user
 **/
int MetaClient::getAddress( CharString & address )
{
	if (!m_Connected)
		return false;

	dword job = createJob( &address );
	send( SERVER_SEND_ADDRESS ) << job;

	return waitJob( job );
}


//----------------------------------------------------------------------------

bool MetaClient::findRoom( const char * pString, Room & found )
{
	dword roomId = 0;
	if ( pString[0] == '@' )
		roomId = strtoul( pString + 1, NULL, 10 );

	CharString pattern( pString );
	pattern.lower();

	Array< Room > rooms;
	if ( getRooms( rooms ) > 0 )
	{
		for(int i=0;i<rooms.size();i++)
		{
			if ( roomId != 0 )
			{
				if ( rooms[i].roomId == roomId )
				{
					found = rooms[i];
					return true;
				}
			}
			else
			{
				CharString name( rooms[i].name );
				name.lower();

				if ( strstr<char>( name, pattern ) != NULL )
				{
					found = rooms[i];
					return true;
				}
			}
		}
	}

	// failed to find room
	return false;
}

//----------------------------------------------------------------------------

dword MetaClient::getSessionId( const char * pUser, const char * pPW )
{
	MetaClient client;
	if (! client.open() )
		return 0;
	if (! client.login( pUser, pPW ) )
		return 0;	

	return client.profile().sessionId;
}

dword MetaClient::version()
{
	return s_CurrentVersion;
}

dword MetaClient::LanguageToLCID( Language nLang )
{
	switch( nLang )
	{
	case ENGLISH:
		return LANG_ENGLISH;
	case GERMAN:
		return LANG_GERMAN;
	case FRENCH:
		return LANG_FRENCH;
	case SPANISH:
		return LANG_SPANISH;
	case CZECH:
		return LANG_CZECH;
	case DUTCH:
		return LANG_DUTCH;
	case CHINESE:
		return LANG_CHINESE;
	case KOREAN:
		return LANG_KOREAN;
	case JAPANESE:
		return LANG_JAPANESE;
	}

	return LANG_ENGLISH;
}

MetaClient::Language MetaClient::LCIDToLanguage( dword nLCID )
{
	switch( nLCID )
	{
	case LANG_ENGLISH:
		return ENGLISH;
	case LANG_GERMAN:
		return GERMAN;
	case LANG_FRENCH:
		return FRENCH;
	case LANG_SPANISH:
		return SPANISH;
	case LANG_CZECH:
		return CZECH;
	case LANG_DUTCH:
		return DUTCH;
	case LANG_CHINESE:
		return CHINESE;
	case LANG_KOREAN:
		return KOREAN;
	case LANG_JAPANESE:
		return JAPANESE;
	}

	return ENGLISH;
}

const char * MetaClient::populationText( int nClients, int nMaxClients )
{
	if ( nClients < (nMaxClients / 5) )
		return "Perfect";
	else if ( nClients < (nMaxClients - (nMaxClients / 3)) )
		return "Medium";
	else if ( nClients < nMaxClients )
		return "High";

	return "Full";
}

//----------------------------------------------------------------------------

CharString MetaClient::flagsText( dword flags )
{
	CharString text;
	if ( flags & ADMINISTRATOR )
		text += "|ADMIN|";
	if ( flags & SERVER )
		text += "|SERVER|";
	if ( flags & MODERATOR )
		text += "|MODERATOR|";
	if ( flags & DEMO )
		text += "|DEMO|";
	if ( flags & BETA )
		text += "|BETA|";
	if ( flags & SUBSCRIBED )
		text += "|SUBSCRIBED|";
	if ( flags & NEWS_ADMIN )
		text += "|NEWS|";
	if ( flags & AWAY )
		text += "|AWAY|";
	if ( flags & MUTED )
		text += "|MUTED|";

	return text;
}

CharString MetaClient::profileText( const Array< ShortProfile > & profiles )
{
	CharString text;
	for(int i=0;i<profiles.size();i++)
	{
		const ShortProfile & profile = profiles[ i ];
		text += CharString().format("<b>%s</b> <i>@%u</i> : %s\n", 
			profile.name.cstr(), profile.userId, profile.status.cstr() );
	}
	return text;
}

bool MetaClient::findClan( const char * pPattern, Array< ShortClan > & found )
{
	if ( pPattern[0] == '@' )
	{
		dword clanId = strtol( pPattern + 1, NULL, 10 );

		Clan clan;
		if ( getClan( clanId, clan ) > 0 )
		{
			ShortClan shortClan;
			shortClan.clanId = clan.clanId;
			shortClan.home = clan.home;
			shortClan.name = clan.name;

			found.push( shortClan );
			return true;
		}

		return false;
	}

	if ( getClans( CharString().format("%%%s%%", pPattern) , found ) > 0 )
		return found.size() > 0;

	return false;

}

bool MetaClient::findUser( const char * pPattern, Array< ShortProfile > & found )
{
	if ( pPattern[0] == '@' )
	{
		dword userId = strtoul( pPattern + 1, NULL, 10 );

		Profile profile;
		if ( getProfile( userId, profile ) > 0 )
		{
			ShortProfile shortProfile;
			shortProfile.name = profile.name;
			shortProfile.flags = profile.flags;
			shortProfile.status = profile.status;
			shortProfile.userId = profile.userId;

			found.push( shortProfile );
			return true;
		}

		return false;
	}

	if ( strlen( pPattern ) < 2 )
	{
		sendLocalChat( "/Error, search pattern must be atleast 2 characters..." );
		return false;
	}

	if ( getProfiles( CharString().format("%%%s%%", pPattern) , found ) > 0 )
		return found.size() > 0;

	return false;
}

bool MetaClient::findUserExactFirst( const char * pPattern, Array< ShortProfile > & found )
{
	if ( pPattern[0] == '@' )
		return findUser( pPattern, found );

	if ( getProfiles( pPattern, found ) > 0 )
		if( found.size() == 0 )
			return findUser( pPattern, found );
		else			
			return found.size() > 0;

	return false;
}

bool MetaClient::findRoomUser( const char * pPattern, Array< ShortProfile > & found )
{
	dword userId = 0;
	if ( pPattern[0] == '@' )
		userId = strtoul( pPattern + 1, NULL, 10 );

	CharString pattern( pPattern );
	pattern.lower();

	AutoLock lock( &m_Lock );
	for( ChatRoomMap::iterator iRoom = m_ChatRoomMap.begin(); 
		iRoom != m_ChatRoomMap.end(); ++iRoom )
	{
		ChatRoom & room = iRoom->second;
		for(int i=0;i<room.m_Players.size();i++)
		{
			if ( userId != 0 )
			{
				if ( room.m_Players[i].userId == userId )
					found.push( room.m_Players[i] );
			}
			else
			{
				CharString name( room.m_Players[i].name );
				name.lower();

				if ( strstr<char>( name, pattern ) != NULL )
					found.push( room.m_Players[i] );
			}
		}
	}

	return found.size() > 0;
}

//----------------------------------------------------------------------------

DLL CharString & MetaClient::Profile::operator[]( const char * pName )
{
	Field * pField = findField( pName );
	if ( pField != NULL )
		return pField->value;

	TRACE( CharString().format( "MetaClient::Profile, field '%s' not found!", pName) );

	static CharString empty;
	return empty;
}

DLL const CharString & MetaClient::Profile::operator[]( const char * pName ) const
{
	const Field * pField = findField( pName );
	if ( pField != NULL )
		return pField->value;

	TRACE( CharString().format( "MetaClient::Profile, field '%s' not found!", pName) );

	static CharString empty;
	return empty;
}

//-------------------------------------------------------------------------------
// EOF

