/*
	WorldServer.cpp

	This object is used to route verbs for one or more clients
	(c)1999 Palestar Development, Richard Lyle
*/


#include "Debug/Assert.h"
#include "Standard/Timer.h"
#include "Standard/SortedArray.h"
#include "Standard/Time.h"
#include "Resource/Resource.h"
#include "Factory/WidgetWrap.h"
#include "Network/FileSocket.h"
#include "File/FileDisk.h"
#include "Constants.h"
#include "WorldServer.h"

//! define to non-zero to enable the class key dictionary..
#define ENABLE_CLASSKEY_DICT			1

//----------------------------------------------------------------------------

const int		PING_CLIENTS_INTERVAL			= 1000 * 30;										// ping all clients how often
const int		PING_CLIENTS_INTERVAL_UDP		= 1000 * 5;										// ping all clients how often
const int		UPDATE_CLIENT_NOUNS_INTERVAL	= 1000 * 1;
const int		SAVE_UNIVERSE_INTERVAL			= 60 * 10 * 1000;						// how often to save the worldContext and storage
const int		UPDATE_LOGIN_INTERVAL			= 500;
const int		LOG_STATS_INTERVAL				= 60 * 1000;

//----------------------------------------------------------------------------

IMPLEMENT_ABSTRACT_FACTORY( WorldServer, Widget );

WorldServer::WorldServer() : 
	m_bActive( false ), 
	m_bSleeping( false ),
	m_bEndGame( true ), 
	m_bStopServer( false ), 
	m_StartTime( 0 ),
	m_nPlayers( 0 ),
	m_nPlayersInQueue( 0 ),
	m_pMetaUpdate( NULL ),
	m_UpdateTimer( TICK_DURATION_MS, true ), 
	m_PingTimer( PING_CLIENTS_INTERVAL, false ),
	m_PingTimerUDP( PING_CLIENTS_INTERVAL_UDP, false ),
	m_SaveTimer( SAVE_UNIVERSE_INTERVAL, false ),
	m_StatsTimer( LOG_STATS_INTERVAL, false ),
	m_nLastStatsLog( 0 ),
	m_nLastSentBytes( 0 ),
	m_nLastRecvBytes( 0 )
{}

WorldServer::~WorldServer()
{
	stop();
}

//----------------------------------------------------------------------------

bool WorldServer::start( const Context & context )
{
	// stop this server first if already running
	WorldServer::stop();

	m_bActive = true;
	m_bEndGame = false;

	m_Context = context;
	if ( m_Context.maxPlayers > m_Context.maxClients )
		m_Context.maxPlayers = m_Context.maxClients;
	if ( m_Context.maxPlayers == 0 )
		m_Context.maxPlayers = m_Context.maxClients;

	m_StartTime = Time::seconds();
	m_Server.clients = 0;
	
	// open the log client
	if ( m_Context.sLogAddress.length() > 0 )
		m_LogClient.open( m_Context.name, m_Context.nSessionId, 
			m_Context.sLogAddress, m_Context.nLogPort );

	// start our profiler server
	m_ProfilerServer.start( new Socket("ZLIB"),
		m_Context.port + PROFILER_SERVER_PORT, m_Context.maxClients );

	// initialize our dictionary..
#if ENABLE_CLASSKEY_DICT
	Factory::initializeClassDictionary( m_pDictionary );
#endif

	// TODO: start the voice server

	// TODO: start the broker server

	// attempt to load worldContext context
	WorldContext::Ref pStaticContext;
	if ( m_Context.nUniverseContext != NULL_WIDGET )
	{
		WorldContext::Link pWorldContext;
		if (! loadResource( m_Context.nUniverseContext, pWorldContext ) )
		{
			LOG_ERROR( "WorldServer", "ERROR: Failed to load context!" );
			return false;
		}
		pStaticContext = pWorldContext;
	}

	// load the world from persistence...
	if ( m_Context.bPersistant )
	{
		if (! loadWorld( false ) )
		{
			LOG_ERROR( "WorldServer", "ERROR: Failed to load world from %s, trying backup...", m_Context.sSaveFile.cstr() );
			if (! loadWorld( true ) )
			{
				LOG_ERROR( "WorldServer", "ERROR: Failed to load world from backup..." );
				return false;
			}
		}
	}

	// merge any changes made in the static data into our persistent data..
	if ( pStaticContext != NULL )
	{
		if ( m_pWorldContext )
			m_pWorldContext->merge( pStaticContext );
		else
			m_pWorldContext = pStaticContext;
	}

	// create a WorldContext object if we have none
	if (! m_pWorldContext.valid() )
		m_pWorldContext = new WorldContext();
	// set the user of the WorldContext as this object
	m_pWorldContext->setUser( this );
	// set the address into the context
	m_pWorldContext->setAddress( m_Context.address, (u16)m_Context.port );

	if (! m_Context.bTutorial )
	{
		// All servers require a connection to the meta server!
		if ( !checkMetaServer() )
		{
			LOG_ERROR( "WorldServer", "Failed to connect to MetaServer!" );
			return false;
		}

		//// check that we are a trusted server
		//if( ( m_MetaClient.profile().flags & MetaClient::SERVER) == 0 )
		//{
		//	// we are not a trusted server, set default rights and disable all scripts
		//	//m_Context.nServerFlags = MetaClient::SERVER;
		//	m_Context.nModFlags = MetaClient::MODERATOR;
		//	m_Context.nClientFlags = 0;
		//	
		//	// only tutorial servers are allowed to run scripts when untrusted
		//	worldContext()->setScript( "" );
		//}

		// start the MetaUpdate thread
		m_pMetaUpdate = new MetaUpdate( this );
		m_pMetaUpdate->resume();
	}
	else
	{
		// tutorial server, set the default rights
		//m_Context.nServerFlags = MetaClient::SERVER;
		m_Context.nModFlags = MetaClient::MODERATOR|MetaClient::ADMINISTRATOR|MetaClient::DEVELOPER;
		m_Context.nClientFlags = 0;
	}
	
	// prep the World
	if (! prepWorld() )
	{
		LOG_ERROR( "WorldServer", "Failed to prepare the world!" );
		return false;
	}

	// start the worldContext
	m_pWorldContext->start();
	// remove any player objects from the worldContext
	cleanWorld();

	// create the listening socket
	Socket::Ref pSocket = new Socket("ZLIB");

	// start this server listening for connections
	if( !Server::start( pSocket, m_Context.port, m_Context.maxClients ) )
	{
		LOG_ERROR( "WorldServer", "Failed to start WorldServer!" );
		WorldServer::stop();

		return false;
	}

	if (! UDPServer::start( m_Context.port ) )
	{
		LOG_ERROR( "WorldServer", "Failed to start UDP server!" );
		WorldServer::stop();

		return false;
	}

	LOG_STATUS( "WorldServer", "Server now running on %s:%d...", m_Context.address.cstr(), m_Context.port);

	// this server is now running!
	return true;
}

bool WorldServer::update()
{
	m_LogClient.update();
	m_ProfilerServer.update();

	return updateServer();
}

bool WorldServer::stop()
{
	if (! m_bActive )
		return false;

	LOG_STATUS( "WorldServer", "Stopping server..." );

	// prevent a crash while deleting all the clients
	m_Lock.lock();
	// set the active flag to false
	m_bActive = false;

	// first, delete all clients which removes their ships from the world, saves their final profile changes, and saves their storage to disk..
	ClientContextHash::iterator iContext = m_ClientContext.begin();
	while( iContext != m_ClientContext.end() )
	{
		ClientContextHash::iterator iRemove = iContext;
		++iContext;

		deleteClient( iRemove->second, true );
	}

	m_Lock.unlock();

	// clean up, stop the threads first
	if ( m_pMetaUpdate != NULL )
	{
		delete m_pMetaUpdate;
		m_pMetaUpdate = NULL;
	}

	// just in case ... this may be removed with little side effect
	m_bEndGame = true;
	// will allow all users information to be updated on the meta-server
	Server::stop();
	UDPServer::stop();
	// remove any player nouns from the worldContext
	cleanWorld();
	// save the worldContext before we release the game data
	saveWorld( true );
	// send all final profile changes
	flushProfileQueue( true );
	// close our connection to the meta server
	m_MetaClient.close();

	m_StartTime = 0;

	m_ClientContext.clear();
	m_UserClientHash.clear();
	m_Moderators.release();

	m_pWorldContext = NULL;
	m_NounClientHash.clear();
	m_FactionClientHash.clear();
	m_TeamClientHash.clear();
	m_ProfileUpdateQueue.release();
	m_LoginQueue.release();

	LOG_STATUS( "WorldServer", "Server stopped..." );

	// stop the profiler server
	m_ProfilerServer.stop();
	// close our connection to the LogServer
	m_LogClient.close();

	return true;
}

bool WorldServer::sendChat( const char * pChat, dword nFromId /*= 0*/)
{
	AutoLock lock( &m_Lock );

	onChat( NULL, pChat);
	for(int i=0;i<clientCount();i++)
		if ( isClient( client(i) ) )
			sendChat( client(i), pChat, nFromId );

	return true;
}

//----------------------------------------------------------------------------

static CharString GetIP( u32 nAddress )
{
	return CharString().format( "%d.%d.%d.%d", 
		(nAddress >> 24) & 0xff,
		(nAddress >> 16) & 0xff, 
		(nAddress >> 8) & 0xff,
		nAddress & 0xff );
}

void WorldServer::onUDP( u32 nAddress, u16 nPort, u8 nMessage, UDP * pUDP )
{
	switch( nMessage )
	{
	case WorldClient::UDP_ACK_CONNECT:
		{
			dword nClient = 0;

			try {
				InStream input( pUDP );
				input >> nClient;
			}
			catch( File::FileError )
			{}

			AutoLock lock( &m_Lock );
			ClientContextHash::iterator iContext = m_ClientContext.find( nClient );
			if ( iContext == m_ClientContext.end() )
			{
				LOG_STATUS( "WorldServer", "%s:%d sent UDP_ACK_CONNECT with invalid client ID of %u", 
					GetIP( nAddress ).cstr(), nPort, nClient );
				break;
			}

			ClientContext & context = iContext->second;
			if (! context.pSocket.valid() )
			{
				LOG_STATUS( "WorldServer","%s:%d sent UDP_ACK_CONNECT, client %u has no socket!", 
					GetIP( nAddress ).cstr(), nPort, nClient );
				break;
			}

			// validate the UDP packet source address against our TCP address, they must be the same..
			if ( nAddress != UDPServer::getAddress( context.pSocket->peerAddress() ) &&
				UDPServer::isClientRegistered( nClient ) )
			{
				LOG_STATUS( "WorldServer", "%s:%d sent UDP_ACK_CONNECT for client %u with IP of %s; IP mismatch", 
					GetIP( nAddress ).cstr(), nPort, nClient, context.pSocket->peerAddress() );
				break;
			}

			context.nLastUDP = Time::seconds();
			lock.release();

			// UDP packet from our client, map the address and port for the provided client ID...
			if (! UDPServer::registerClient( nClient, nAddress, nPort ) )
			{
				LOG_STATUS( "WorldServer", "%s:%d failed to register client %u ", GetIP( nAddress ).cstr(), nPort, nClient );
				break;
			}

			//LOG_STATUS( "WorldServer", CharString().format( "Received UDP_ACK_CONNECT for client %u from %s:%d", nClient, GetIP( nAddress ), nPort ) );
		}
		break;
	default:
		//LOG_STATUS( "WorldServer", CharString().format( "Unknown UDP packet from %s:%d, message 0x%x", GetIP( nAddress ), nPort, nMessage ) );
		break;
	}
}

void WorldServer::onClientUDP( u32 nClient, u8 nMessage, UDP * pUDP )
{
	AutoLock lock( &m_Lock );

	ClientContextHash::iterator iContext = m_ClientContext.find( nClient );
	if ( iContext != m_ClientContext.end() )
	{
		ClientContext & context = iContext->second;
		context.nLastUDP = Time::seconds();

		bool bError = false;

		try {
			InStream input( pUDP, FF_TRANSMIT, SV_CURRENT, m_pDictionary );
			receiveMessage( context, true, nMessage, input );
		}
		catch( File::FileError )
		{
			bError = true;
		}

		if ( bError )
		{
			LOG_STATUS( "WorldServer", CharString().format("Client %u UDP error, removing client..", nClient ) );
			Server::removeClient( nClient );
		}
	}
	else
	{
		LOG_STATUS( "WorldServer", CharString().format( "Client %u sent UDP packet, context not found", nClient ) );
	}
}

void WorldServer::onConnect( dword nClientId )
{
	// get the client Socket object
	AutoLock lock( &m_Lock );
	Socket::Ref pSocket = clientSocket( nClientId );
	if ( m_bActive && pSocket.valid() )
	{
		if ( m_bSleeping )
		{
			LOG_STATUS( "WorldServer", "Leaving sleep mode." );
			m_bSleeping = false;
		}

		ClientContext & newContext = m_ClientContext[ nClientId ];
		newContext.nClientId = nClientId;
		newContext.pSocket = pSocket;
		newContext.nLoginTime = Time::seconds();
		newContext.nLastTime = Time::milliseconds();
		newContext.bConnectedUDP = false;
		newContext.nLastUDP = 0; 

		CharString sIP = pSocket->peerAddress();
		int nPort =  pSocket->peerPort();
		
		LOG_STATUS( "WorldServer", "Client %u connected from %s:%d on %s:%d", nClientId, sIP.cstr(), nPort,
			pSocket->address(), pSocket->port() );

		// update the client count in the register server
		m_Server.clients = clientCount();

		// send the dictionaries to this client first thing..
		Dictionary::TypedStorage<ClassKey> * pClassKey = m_pDictionary->getStorage<ClassKey>();
		if ( pClassKey != NULL )
			send( nClientId, WorldClient::CLIENT_RECV_CK_DICT ) << pClassKey;

		// start the UDP connection for this client
		if ( m_Context.bEnableUDP )
			send( nClientId, WorldClient::UDP_START_CONNECT ) << nClientId;
	}
	else
	{
		// we are shutting down, remove any new clients!
		Server::removeClient( nClientId );		
	}
}

void WorldServer::onReceive( dword nClientId, byte message, const InStream & input )
{
	AutoLock lock( &m_Lock );

	ClientContextHash::iterator iContext = m_ClientContext.find( nClientId );
	if ( iContext != m_ClientContext.end() )
		receiveMessage( iContext->second, false, message, input );
}

void WorldServer::onDisconnect( dword nClientId )
{
	LOG_STATUS( "WorldServer","Client %u Disconnected", nClientId );

	// Remove the UDP connection first
	UDPServer::removeClient( nClientId );

	AutoLock lock( &m_Lock );
	// start the logoff process for the client
	logoffClient( nClientId );
}

//----------------------------------------------------------------------------

bool WorldServer::validateClient( ClientContext & context, dword nFlags /*= 0*/ )
{
	if ( context.nUserId != 0 && (context.MetaProfile.flags & nFlags) == nFlags )
		return true;

	//LOG_STATUS( "WorldServer", CharString().format("Client %u failed validation...", context.nClientId) );
	//Server::removeClient( context.nClientId );

	return false;
}

bool WorldServer::isValid( dword nClientId )
{
	ClientContextHash::iterator iClient = m_ClientContext.find( nClientId );
	return ( iClient != m_ClientContext.end() && iClient->second.nUserId != 0 );
}

bool WorldServer::isClient( dword nClientId )
{
	return m_ClientContext.find( nClientId ) != m_ClientContext.end();
}

bool WorldServer::isPlayer( dword nClientId )
{
	ClientContextHash::iterator iClient = m_ClientContext.find( nClientId );
	return iClient != m_ClientContext.end() && iClient->second.nUserId != 0;
}

bool WorldServer::isModerator( dword nClientId )
{
	ClientContextHash::iterator iClient = m_ClientContext.find( nClientId );
	return iClient != m_ClientContext.end() && iClient->second.nUserId != 0 
		&& (iClient->second.MetaProfile.flags & m_Context.nModFlags) != 0;
}

//----------------------------------------------------------------------------

dword WorldServer::getClientId( dword nUserId )
{
	UserClientHash::iterator iClient = m_UserClientHash.find( nUserId );
	if ( iClient == m_UserClientHash.end() )
		return 0;
	return iClient->second;
}

dword WorldServer::getUserId( dword nClientId )
{
	ClientContextHash::iterator iClient = m_ClientContext.find( nClientId );
	if ( iClient == m_ClientContext.end() )
		return 0;
	return iClient->second.nUserId;
}

dword WorldServer::getFlags( dword nClientId )
{
	ClientContextHash::iterator iClient = m_ClientContext.find( nClientId );
	if ( iClient == m_ClientContext.end() )
		return 0;
	return iClient->second.MetaProfile.flags;
}

//----------------------------------------------------------------------------

int WorldServer::addTeam( const MetaClient::Clan & clan )
{
	if ( clan.clanId == 0 )
		return 0;

	// make sure there is not already a team for this clan and make sure faction does exist
	int nTeamId = worldContext()->teamClan( clan.clanId, clan.faction );
	if ( nTeamId == 0 )
	{
		int nCopyTeamId = worldContext()->factionTeam( clan.faction );
		if ( nCopyTeamId == 0 )
		{
			LOG_STATUS( "WorldServer", "Failed to find team for faction %d in world context!", clan.faction );
			return 0;
		}

		const WorldContext::Team & copy = worldContext()->team( nCopyTeamId );
		ASSERT( copy.clanId == 0 );

		// create a new team
		WorldContext::Team team;
		team.nTeamId = 0;
		team.clanId = clan.clanId;
		team.factionId = copy.factionId;
		team.name = CharString().format("%s:%s", copy.name.cstr(), clan.name.cstr() );
		team.longName = clan.longName;
		team.about = copy.about;
		team.pTemplates = copy.pTemplates;

		if ( worldContext()->addTeam( team ) )
		{
			nTeamId = team.nTeamId;

			// send new team out to all other clients
			for(int i=0;i<clientCount();i++)
				if ( isValid( client(i) ) )
					send( client(i), WorldClient::CONTEXT_ADD_TEAM ) << team;

			LOG_STATUS( "WorldServer", "Team %u '%s' created for clan %u", team.nTeamId, team.name.cstr(), team.clanId );
		}
		else
		{
			LOG_STATUS( "WorldServer", "Failed to add team to world context!" );
		}
	}

	return nTeamId;
}

void WorldServer::sendContext( ClientContext & context )
{
	// if this is a proxy client, then we don't send anything!
	if (! context.bProxy )
	{
		// send the context to the client first
		send( context.nClientId, WorldClient::CONTEXT_INIT ) 
			<< WidgetWrap( worldContext(), FF_TRANSMIT|WorldContext::FF_CLIENT, m_pDictionary );
	}

	// initialize the UpdateHash for this client...
	for(int i=0;i<worldContext()->zoneCount();++i)
	{
		NodeZone * pZone = worldContext()->zone( i );
		if (! pZone )
			continue;
		for(int k=0;k<pZone->childCount();++k)
		{
			Noun * pNoun = WidgetCast<Noun>( pZone->child(k) );
			if (! pNoun )
				continue;

			if ( context.bProxy )
			{
				// proxy client, send any TYPE_OBJECT noun's to the client, so they can see ships on other server.
				if ( pNoun->nounType() == TYPE_OBJECT )
				{
					sendAddNoun( context.nClientId, pNoun->parent()->key(), pNoun->key(), 
						WidgetWrap( pNoun, FF_TRANSMIT, m_pDictionary ) );
					context.updateHash[ pNoun->key() ] = pNoun->version();
				}
				else
				{
					// assume client has version 0
					context.updateHash[ pNoun->key() ] = 0;			
				}
			}
			else
				context.updateHash[ pNoun->key() ] = pNoun->version();
		}
	}
}

bool WorldServer::sendStorage( ClientContext & context )
{
	if ( context.bProxy )
		return true;		// client is a proxy, no storage is allowed..
	if (! loadStorage( context ) )
		return false;

	context.pStorage = getStorage( context.nUserId );
	if (! context.pStorage )
		return false;

	send( context.nClientId, WorldClient::CLIENT_RECV_STORAGE ) 
		<< WidgetWrap( context.pStorage, FF_TRANSMIT, m_pDictionary );
	return true;
}

//----------------------------------------------------------------------------

bool WorldServer::addPlayerProfile( dword nUserId, const char * pFieldName, float fAmount )
{
	if( ( m_MetaClient.profile().flags & MetaClient::SERVER) == 0 )
		return false;		// only trusted servers can push profile updates...
	UserClientHash::iterator iClient = m_UserClientHash.find( nUserId );
	if ( iClient == m_UserClientHash.end() )
		return false;
	ClientContextHash::iterator iContext = m_ClientContext.find( iClient->second );
	if ( iContext == m_ClientContext.end() )
		return false;
	ClientContext & context = iContext->second;

	MetaClient::Field * pField = context.MetaProfile.findField( pFieldName );
	if (! pField )
	{
		LOG_STATUS( "WorldServer", CharString().format( "WorldServer::addPlayerProfile() - Field %s not found in MetaProfile!", pFieldName ) );
		return false;
	}

	// accumulate stat changes in a queue, which will hold them for a few minutes before pushing 
	// them off to be stored in the database!
	m_ProfileUpdateQueue.pushUpdate( nUserId, pField->id, fAmount );

	return true;
}

//----------------------------------------------------------------------------

bool WorldServer::loadWorld( bool a_bUseBackup )
{
	if (! m_Context.bPersistant )
		return false;		// not persistent

	CharString sWorldFile = m_Context.sSaveFile;
	if ( a_bUseBackup )
		sWorldFile += ".bak";

	if (! FileDisk::fileExists( sWorldFile ) )
		return true;		// nothing to load so success!

	bool bLoaded = false;
	try {
		InStream input( new FileDisk( sWorldFile ) );

		int nVersion;
		input >> nVersion;

		// validate the version number...
		switch( nVersion )
		{
		case SV_20090104:
			input.setVersion( nVersion );
			break;
		default:
			// file that is prior to the stream versions...
			input.setVersion( 0 );
			break;
		}

		input >> m_pWorldContext;
		bLoaded = true;
	}
	catch( FileDisk::FileError )
	{}

	if ( bLoaded )
	{
		// reset the worldContext tick
		if ( m_pWorldContext.valid() )
			m_pWorldContext->setTick( 0 );

		// move any player store from the hash into file storage, this will only happen
		// if the server crashed instead of shutting down correctly!
		if (! flushStorage( true ) )
			LOG_STATUS( "WorldServer", "Failed to flush user storage to disk!" );
	}

	return bLoaded;
}

bool WorldServer::saveWorld( bool bForce /*= false*/ )
{
	if (! m_Context.bPersistant )
		return false;		// not persistent
	if (! m_pWorldContext.valid() )
		return false;		// no worldContext to save
	if (! m_SaveTimer.signaled() && !bForce )
		return false;		// not time to save

	// flush any unsaved user storage to disk first..
	flushStorage( bForce );

	CharString sBackupFile = m_Context.sSaveFile + ".bak";
	if ( FileDisk::fileExists( m_Context.sSaveFile ) )
	{
		LOG_STATUS( "WorldServer", "Backing up worldContext file..." );
		FileDisk::deleteFile( sBackupFile );
		FileDisk::renameFile( m_Context.sSaveFile, sBackupFile );
	}

	LOG_STATUS( "WorldServer", "Saving worldContext..." );

	dword nSaveTime = Time::milliseconds();
	bool bSaved = false;
	try {
		int nVersion = SV_CURRENT;

		// save the worldContext to disk
		OutStream output( new FileDisk( m_Context.sSaveFile, FileDisk::WRITE ) );
		output << nVersion << m_pWorldContext;

		bSaved = true;
	}
	catch( FileDisk::FileError )
	{
		FileDisk::deleteFile( m_Context.sSaveFile );
	}

	if ( bSaved )
		LOG_STATUS( "WorldServer", "Universe saved to %s...%d ms", m_Context.sSaveFile.cstr(), Time::milliseconds() - nSaveTime );
	else
		LOG_ERROR( "WorldServer", "Failed to save worldContext to %s...", m_Context.sSaveFile.cstr() );

	return bSaved;
}

bool WorldServer::prepWorld()
{
	return true;
}

bool WorldServer::cleanWorld()
{
	// Applications should override this and provide a method to detach any player related objects from the world.. this
	// function is called before the server is shutdown and the world persisted.
	return true;
}

//----------------------------------------------------------------------------
// EOF

