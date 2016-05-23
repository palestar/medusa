/*
	WorldClient.cpp

	This object is used to connect to an WorldServer object over a Socket object
	(c)1999 Palestar, Richard Lyle
*/

#define MEDUSA_TRACE_ON			// force trace messages to stay on
#include "Debug/Assert.h"
#include "Debug/Trace.h"
#include "Debug/Profile.h"
#include "Factory/WidgetWrap.h"
#include "Network/FileSocket.h"
#include "Network/Socket.h"
#include "Standard/Timer.h"
#include "Standard/Time.h"
#include "Standard/Settings.h"
#include "Standard/StringBuffer.h"
#include "Standard/RegExpM.h"
#include "Resource/Resource.h"
#include "NounTarget.h"
#include "Constants.h"
#include "WorldClient.h"

//----------------------------------------------------------------------------

const int		TIMEOUT = (60 * 5) * 1000;
const dword		DEFAULT_BPS = 5 * 1024;

static Constant	DEFAULT_FOCUS_AREA( "DEFAULT_FOCUS_AREA", 5000.0f );

WorldClient::UserWorldClient NULL_USER;

//----------------------------------------------------------------------------

IMPLEMENT_ABSTRACT_FACTORY( WorldClient, Widget );

#pragma warning(disable:4800) // forcing value to bool 'true' or 'false' (performance warning)

WorldClient::WorldClient() : Client(),
	m_pUser( &NULL_USER ), 
	m_Active( true ), 
	m_Connected( false ), 
	m_bEnableUDP( true ),
	m_bEnableFilter( true ),
	m_sFilterFile( "WordFilter.txt" ),
	m_bConnectedUDP( false ), 
	m_LoggedIn( false ), 
	m_bProxy( false ),
	m_nProxyId( NULL_WIDGET ),
	m_nProxyFactionId( 0 ),
	m_bServer( false ),
	m_bBanned( false ),
	m_vContextTransform( Vector3::ZERO ),
	m_nQueuePosition( 0 ),
	m_nQueueSize( 0 ),
	m_ServerVersion( 0 ), 
	m_nLatency( 0 ), 
	m_nLatencyUDP( 0 ), 
	m_BPS( DEFAULT_BPS ), 
	m_SessionId( 0 ), 
	m_pMetaClient( NULL ), 
	m_nDefaultRoomId( 0 ),
	m_pMetaUpdate( NULL ),
	m_nTeamId( 0 ), 
	m_LastChatMessage( 0 ), 
	m_ReconnectTime( 0 ), 
	m_fFocusArea( DEFAULT_FOCUS_AREA ), 
	m_nLastPingTime( 0 ), 
	m_nLastPingTimeUDP( 0 ), 
	m_nLastUDPTime( 0 ),
	m_bServerTransfer( false )
{
	// create a new WorldContext object
	m_pWorldContext = new WorldContext();
	m_pWorldContext->setUser( this );

	// create our storage object
	m_pStorage = new UserStorage();
	// create a empty profile until we receive one..
	m_pProfile = new Profile();
	m_pProfile->setName( "John Doe" );
}

WorldClient::~WorldClient()
{
	// make sure we don't get any crashes..
	if ( m_pWorldContext )
		m_pWorldContext->setUser( NULL );
	
	// stop the threads
	m_Active = false;

	if (m_pMetaUpdate )
	{
		delete m_pMetaUpdate;
		m_pMetaUpdate = NULL;
	}

	// close the connection to the meta server
	if ( m_pMetaClient )
	{
		delete m_pMetaClient;
		m_pMetaClient = NULL;
	}
}

//----------------------------------------------------------------------------

void WorldClient::onConnect()
{
	if (! m_bServerTransfer )
	{
		lock();

		// set the connected flag
		m_Connected = true;
		// notify the user
		user()->onConnected();

		TRACE( CharString().format( "WorldClient::onConnect() - Connected to %s:%d, Interface %s:%d", 
			socket()->peerAddress(), socket()->peerPort(), socket()->address(), socket()->port() ) );

		// Start the UDP Server...
		if (! UDPServer::start( socket()->port() ) )
			TRACE( "Failed to start UDPServer!" );

		m_nLastUDPTime = 0;
		// register our one client..
		UDPServer::registerClient( SERVER_CLIENTID, 
			UDPServer::getAddress( socket()->peerAddress() ), socket()->peerPort() );

		unlock();
	}
	else
	{
		lock();

		m_Connected = true;
		// Start the UDP Server...
		if (! UDPServer::start( socket()->port() ) )
			TRACE( "Failed to start UDPServer!" );

		m_nLastUDPTime = 0;
		// register our one client..
		UDPServer::registerClient( SERVER_CLIENTID, 
			UDPServer::getAddress( socket()->peerAddress() ), socket()->peerPort() );

		// auto-login to the server now
		m_LoginEvent.clear();
		m_LoggedIn = true;

		// update our server variables so that if we lose our connection, we will connect to the correct server instead
		// of the server we transfered from...
		Client::m_sAddress	= m_TransferReq.m_sAddress;
		Client::m_nPort		= m_TransferReq.m_nPort;

		unlock();
	}
}

void WorldClient::onReceive( byte nMessage, const InStream & input )
{
	receiveMessage( false, nMessage, input );
}

void WorldClient::onDisconnect()
{
	lock();

	if ( m_bServerTransfer )
	{
		pushChat( "<color;ffffff>CLIENT: Failed to connect to the new server..." );
		m_bServerTransfer = false;
	}

	// release any waiting threads
	m_LoginEvent.signal();
	m_ContextReceived.signal();
	// notify the user
	user()->onDisconnected();
	// stop the UDPServer
	UDPServer::stop();

	// don't stop the Context if this is a ProxyClient, we want the other maps to keep simulating..
	if (! m_bProxy )
	{
		// stop the world simulation
		WorldContext * pContext = context();
		if ( pContext != NULL )
			m_pWorldContext->stop();	
	}

	// set the connected flag false
	m_Connected = false;
	m_LoggedIn = false;
	
	unlock();
}

//---------------------------------------------------------------------------------------------------

void WorldClient::onUDP( u32 nAddress, u16 nPort, u8 nMessage, UDP * pUDP )
{
	TRACE( CharString().format("Unknown UDP from %u : %d", nAddress, nPort) );
}

void WorldClient::onClientUDP( u32 nClient, u8 nMessage, UDP * pUDP )
{
	if ( nClient == SERVER_CLIENTID )
	{
		try {
			InStream input( pUDP, FF_TRANSMIT, SV_CURRENT, m_pDictionary );
			receiveMessage( true, nMessage, input );
		}
		catch( File::FileError )
		{}

		m_nLastUDPTime = Time::seconds();
	}
}

//----------------------------------------------------------------------------

bool WorldClient::prepWorld()
{
	if ( m_pWorldContext )
		m_pWorldContext->onWorldTransform( m_vContextTransform );

	return true;
}

bool WorldClient::setContext( Context * pContext )
{
	if ( m_pWorldContext != pContext )
	{
		if ( !m_bProxy && m_pWorldContext.valid() )
		{
			m_pWorldContext->setUser( NULL );
			m_pWorldContext->stop();
		}

		m_pWorldContext = pContext;
		m_pWorldContext->setUser( this );

		// prepare the world
		if (! prepWorld() )
		{
			LOG_ERROR( "WorldClient", "setContext() - Failed to prepare world!" );
			return false;
		}
		m_pWorldContext->start();
	}

	return true;
}

void WorldClient::setUser( UserWorldClient * pUser )
{
	m_pUser = pUser != NULL ? pUser : &NULL_USER;
}

void WorldClient::setEnableUDP( bool a_bEnableUDP )
{
	m_bEnableUDP = a_bEnableUDP;
}

void WorldClient::setEnableFilter( bool a_bFilter )
{
	if ( a_bFilter != m_bEnableFilter )
	{
		m_bEnableFilter = a_bFilter;

		if ( m_pMetaClient != NULL )
		{
			if ( m_bEnableFilter )
				m_pMetaClient->loadWordFilter( m_sFilterFile );
			else
				m_pMetaClient->clearWordFilter();
		}
	}
}

void WorldClient::setFilterFile( const char * a_pFile )
{
	if ( a_pFile != m_sFilterFile )
	{
		m_sFilterFile = a_pFile;

		if ( m_pMetaClient != NULL && m_bEnableFilter )
			m_pMetaClient->loadWordFilter( m_sFilterFile );
	}
}

bool WorldClient::connect( const char * a_pAddress, int a_nPort )
{
	close();

	// clear this event that was set when we disconnected from any previous server..
	m_ContextReceived.clear();

	if (! Client::open( new Socket("ZLIB"), a_pAddress, a_nPort ) )
		return false;

	// request version information
	dword job = createJob();
	send( SERVER_SEND_VERSION ) << job;

	m_ServerVersion = waitJob( job );
	if ( m_ServerVersion != version() )
		return false;

	return true;
}

bool WorldClient::login( dword a_nSessionID )
{
	if ( m_LoggedIn )
		return false;

	lock();

	m_LoginEvent.clear();
	m_SessionId = a_nSessionID;
	m_LoggedIn = true;
	m_bProxy = false;
	m_bServer = false;

	send( SERVER_LOGIN ) << version() << m_SessionId 
		<< false		// transfer
		<< false		// proxy
		<< false;		// server

	if (! m_pMetaClient )
	{
		// create the meta client object
		m_pMetaClient = new MetaClient;

		if ( m_bEnableFilter )
			m_pMetaClient->loadWordFilter( m_sFilterFile );
		
		// start the meta update thread
		m_pMetaUpdate = new MetaUpdate( this );
		m_pMetaUpdate->resume();
	}

	unlock();

	return true;
}

bool WorldClient::loginAsProxy( dword a_nSessionID, const WidgetKey & a_nObjectId,
						int a_nFactionId )
{
	if ( m_LoggedIn )
		return false;

	lock();

	m_LoginEvent.clear();
	m_SessionId = a_nSessionID;
	m_LoggedIn = true;
	m_bProxy = true;
	m_nProxyId = a_nObjectId;
	m_nProxyFactionId = a_nFactionId;
	m_bServer = false;

	send( SERVER_LOGIN ) << version() << m_SessionId 
		<< false		// transfer
		<< true << a_nObjectId << a_nFactionId		// proxy
		<< false;		// server
	unlock();

	return true;
}

bool WorldClient::loginAsServer( dword a_nSessionID )
{
	if ( m_LoggedIn )
		return false;

	lock();

	m_LoginEvent.clear();
	m_SessionId = a_nSessionID;
	m_LoggedIn = true;
	m_bProxy = false;
	m_bServer = true;

	send( SERVER_LOGIN ) << version() << m_SessionId 
		<< false		// transfer
		<< false		// proxy
		<< true;		// server
	unlock();

	return true;
}


bool WorldClient::transfer( const TransferRequest & a_Request )
{
	if ( m_bProxy )
	{
		LOG_ERROR( "WorldClient", "transfer invalid for proxy client." );
		return false;
	}

	LOG_STATUS("WorldClient", "Starting transfer to %s:%d, Self = %llu, Spawn = %s, Items = %u", 
		a_Request.m_sAddress.cstr(), a_Request.m_nPort,
		a_Request.m_nSelf.m_Id, a_Request.m_sSpawnPoint.cstr(), 
		a_Request.m_Items.size() );

	UDPServer::stop();

	m_bServerTransfer = true;		// this flag is cleared once we receive the context from the server..
	m_TransferReq = a_Request;

	socket()->close();

	if (! socket()->connect( m_TransferReq.m_sAddress, m_TransferReq.m_nPort ) )
	{
		onDisconnect();
		return false;
	}

	send( SERVER_LOGIN ) << version() << m_SessionId 
		<< true				// transfer
		<< false			// proxy
		<< false;			// server
	return true;
}

bool WorldClient::reconnect()
{
	if ( m_bProxy )
		return false;
	if (! Client::reconnect() )
		return false;		
	
	lock();

	m_LoginEvent.clear();
	m_LoggedIn = true;

	send( SERVER_LOGIN ) << version() << m_SessionId 
		<< true				// transfer
		<< m_bProxy			// proxy
		<< m_bServer;		// transfer

	unlock();

	return true;
}

bool WorldClient::waitForContext()
{
	m_ContextReceived.wait();
	return connected();
}

bool WorldClient::setModerate( bool moderate )
{
	if (! m_LoggedIn )
		return false;
	if (! canModerate() )
		return false;

	send( SERVER_SET_MODERATE ) << moderate;
	return true;
}

bool WorldClient::sendProfile()
{
	if (! m_LoggedIn )
		return false;

	send( SERVER_SEND_PROFILE );
	return true;
}

bool WorldClient::sendServerStatus()
{
	if (! m_LoggedIn )
		return false;

	send( SERVER_SEND_SERVER_STATUS );
	return true;
}

bool WorldClient::selectTeam( int nTeamId )
{
	if (! m_LoggedIn )
		return false;

	send( SERVER_SELECT_TEAM ) << nTeamId;
	return true;
}

bool WorldClient::requestTransfer( const WorldClient::TransferRequest & a_Req )
{
	if (! m_LoggedIn )
		return false;

	LOG_STATUS("WorldClient", "Requesting transfer to %s:%d, Self = %llu, Spawn = %s, Items = %u", 
		a_Req.m_sAddress.cstr(), a_Req.m_nPort,
		a_Req.m_nSelf.m_Id, a_Req.m_sSpawnPoint.cstr(), 
		a_Req.m_Items.size() );

	// just set the flag to true ... the server will send us a message to do the actual transfer!
	// setting this flag will prevent the ship selection screen from getting shown...
	m_bServerTransfer = true;
	// notify our user that we are waiting for the server transfer message..
	user()->onServerTransfer( a_Req );

	send( SERVER_TRANSFER ) << a_Req;
	return true;
}

bool WorldClient::selectSelf( const WidgetKey & a_nSpawn, const WidgetKey & a_nSelf, const WidgetKeys & items )
{
	if (! m_LoggedIn || m_bProxy )
		return false;		// not connected or logged in

	send( SERVER_SELECT_SELF ) << a_nSpawn << a_nSelf << items;
	return true;
}

void WorldClient::onSetSelf( Noun * pSelf )
{}

bool WorldClient::selectProxy( const WidgetKey & a_nProxyId, int a_nFactionId )
{
	if (! m_LoggedIn || !m_bProxy )
		return false;		// not connected or logged in

	m_nProxyId = a_nProxyId;
	m_nProxyFactionId = a_nFactionId;

	send( SERVER_SELECT_PROXY ) << a_nProxyId << a_nFactionId;
	return true;
}

bool WorldClient::removeSelf()
{
	if (! m_LoggedIn )
		return false;

	send( SERVER_REMOVE_SELF );
	return true;
}

bool WorldClient::removeStorage( const WidgetKeys & remove )
{
	if (! m_LoggedIn )
		return false;
	if ( remove.size() == 0 )
		return false;

	send( SERVER_REMOVE_STORAGE ) << remove;
	for(int i=0;i<remove.size();i++)
	{
		user()->onDeleteStorage( remove[i] );
		m_pStorage->remove( remove[i] );
	}

	return true;
}

bool WorldClient::canGroupInvite( Noun * pTarget )
{
	if ( pTarget->userId() == 0 )
		return false;		// can't invite non players to group
	if ( pTarget == m_pSelf || pTarget->zoneParent() == m_pSelf )
		return false;		// can't invite self
	if( !isGroupLeader() )
		return false;		// not the group leader 
	if ( isGroupMember( pTarget ) )
		return false;		// already a member of this group
	if ( pTarget->factionId() != factionId() )
		return false;		// can't invite enemies

	return true;
}

bool WorldClient::canGroupRemove( Noun * pTarget )
{
	Group * pGroup = group();
	if (! pGroup )
		return false;		// not in a group 
	if ( pTarget == m_pSelf )
		return true;		// you can always remove yourself from the group.
	if (! isGroupLeader() )
		return false;		// only leaders can remove people from the group
	if (! isGroupMember( pTarget ) )
		return false;		// target isn't in our group
	if ( pGroup->isLeader( pTarget->userId() ) )
		return false;		// can't remove another leader

	return true;
}

bool WorldClient::canGroupPromote( Noun * pTarget )
{
	Group * pGroup = group();
	if (! pGroup )
		return false;		// not in a group 
	if (! isGroupLeader() )
		return false;		// only leaders can promote
	if (! isGroupMember( pTarget ) )
		return false;		// target isn't in our group
	if ( pGroup->isLeader( pTarget->userId() ) )
		return false;		// already a leader

	return true;
}

bool WorldClient::groupInvite( Noun * pTarget )
{
	send( GROUP_INVITE ) << Noun::wRef( pTarget );
	return true;
}

bool WorldClient::groupDecline( Noun * pTarget )
{
	send( GROUP_DECLINE ) << Noun::wRef( pTarget );
	return true;
}

bool WorldClient::groupPromote( Noun * pTarget )
{
	send( GROUP_PROMOTE ) << Noun::tRef( pTarget );
	return true;
}

bool WorldClient::groupRemove( Noun * pTarget )
{
	send( GROUP_REMOVE ) << Noun::tRef( pTarget );
	return true;
}

bool WorldClient::groupLeave()
{
	return groupRemove( self() );
}

bool WorldClient::sendVerb( Noun * pNoun, Verb * pVerb )
{
	if (! m_LoggedIn )
		return false;
	if ( pNoun == NULL || pVerb == NULL )
		return false;

	if ( pVerb->useUDP() && m_bEnableUDP && m_bConnectedUDP )
		sendUDP( SERVER_CLIENTID, SERVER_RECV_VERB ) << pNoun->key() << WidgetWrap( pVerb, FF_TRANSMIT, m_pDictionary );
	else
		send( SERVER_RECV_VERB ) << pNoun->key() << WidgetWrap( pVerb, FF_TRANSMIT, m_pDictionary );
	return true;
}

void WorldClient::setFocus( Noun * pFocus, bool bForce /*= false*/ )
{
	lock();

	// if focus is NULL, then default to the players ship
	if ( pFocus == NULL )
		pFocus = self();

	if ( bForce || m_pFocus != pFocus )
	{
		// set the new focus
		m_pFocus = pFocus;
		// inform the server of our focus change
		send( SERVER_SET_FOCUS ) << m_pFocus;
	}

	unlock();
}

void WorldClient::setTarget( Noun * pTarget, bool bForce /*= false*/ )
{
	lock();

	if ( bForce || m_pTarget != pTarget )
	{
		m_pTarget = pTarget;
		send( SERVER_SET_TARGET ) << m_pTarget;
	}

	unlock();
}

void WorldClient::update()
{
	Client::update();

	if (! m_bProxy )
	{
		// we assume lock() has already been called when this function is invoked..
		WorldContext * pContext = context();
		if ( pContext != NULL )
		{
			// lock all zones intersecting with our focus area or containing our ship or target
			for(int j=-1;j<pContext->worldCount();++j)
			{
				WorldContext * pCheck = j >= 0 ? pContext->world(j) : pContext;

				BoxHull hFocusArea( m_pFocus.valid() ? BoxHull( m_fFocusArea, m_pFocus->worldPosition() ) : false );
				for(int i=0;i<pCheck->zoneCount();i++)
				{
					NodeZone * pZone = pCheck->zone( i );
					if ( pZone->locked() )
					{
						// if the world hull of the zone intersects with our focus area, lock the zone for updating
						if ( hFocusArea.intersect( pZone->worldHull() ) 
							|| pZone->inZone( m_pSelf ) 
							|| pZone->inZone( m_pTarget ) )
						{
							continue;		// keep zone locked
						}
						// unlock the zone
						pCheck->unlockZone( pZone );
					}
					else if (! pZone->locked() )
					{
						if ( !hFocusArea.intersect( pZone->worldHull() ) 
							&& !pZone->inZone( m_pSelf ) 
							&& !pZone->inZone( m_pTarget ) )
						{
							continue;		// keep zone unlocked
						}
						// lock the zone
						pCheck->lockZone( pZone, true );
					}
				}
			}

			// update the worldContext
			pContext->update();
		}
	}

	// validate our current target and focus
	setTarget( validateNoun( target() ) );
	setFocus( validateNoun( focus() ) );

	dword nNow = Time::seconds();

	// UDP is consider ready so long as we have received a UDP packet within the time period specified by CLIENT_UDP_DOWN_TIME
	if ( (m_nLastUDPTime + CLIENT_UDP_DOWN_TIME) > nNow )
	{
		if (! m_bConnectedUDP )
		{
			TRACE( "UDP connection up!" );
			m_bConnectedUDP = true;
		}
	}
	else if ( m_bConnectedUDP )
	{
		m_bConnectedUDP = false;
		TRACE( "UDP connection down!" );
	}

	// update the latency every once in a while
	if ( nNow >= (m_nLastPingTime + CLIENT_PING_TIME) )
	{
		m_nLastPingTime = nNow;
		try {
			send( PING ) << Time::milliseconds();
		}
		catch( File::FileError )
		{}
	}

	if ( m_bEnableUDP && nNow >= (m_nLastPingTimeUDP + CLIENT_UDP_PING_TIME) )
	{
		m_nLastPingTimeUDP = Time::seconds();
		try {
			sendUDP( SERVER_CLIENTID, PING ) << Time::milliseconds();
		}
		catch( File::FileError )
		{}
	}

	if (! m_bProxy )
	{
		// display the current latency
		Profiler::message( 1, CharString().format( "Latency: %u ms, Tick: %u, UDP: %u ms", m_nLatency, m_pWorldContext->tick(), m_nLatencyUDP ) );

		// display network stats
		Socket * pSocket = socket();
		if ( pSocket != NULL )
		{
			static dword nLastUpdated = 0;
			static dword nLast[ 2 ];
			static CharString sSent, sRecv;

			dword nElapsed = Time::milliseconds() - nLastUpdated;
			if ( nElapsed > 1000 )
			{
				
				dword nSent = pSocket->bytesSent();
				dword nRecv = pSocket->bytesReceived();

				sSent.format( "Sent: %u / %u bps",
					nSent, ((nSent - nLast[0]) * nElapsed) / 1000 );
				sRecv.format( "Recv: %u / %u bps",
					nRecv, ((nRecv - nLast[1]) * nElapsed) / 1000 );

				nLast[0] = nSent;
				nLast[1] = nRecv;
				nLastUpdated += nElapsed;
			}
			
			Profiler::message( 2, sSent );
			Profiler::message( 3, sRecv );
		}
	}

	// let subclasses update as well..
	onUpdate();
}

void WorldClient::onUpdate()
{}

//----------------------------------------------------------------------------

Noun * WorldClient::validateTarget( Noun * pTarget )
{
	return validateNoun( pTarget );
}

Noun * WorldClient::validateNoun( Noun * pTarget )
{
	if ( pTarget == NULL )
		return NULL;		
	if ( WidgetCast<NounTarget>( pTarget ) )
		return pTarget;		// NounTarget objects are always valid
	if ( pTarget->zone() == NULL )
		return NULL;		// object detached from the world

	return pTarget;
}

int WorldClient::metaUpdateDemon()
{
	TRACE("WorldClient::metaUpdateDemon() begin");

	while( m_Active )
	{
		Thread::sleep( 100 );

		updateMeta();
	}

	TRACE("WorldClient::metaUpdateDemon() end");
	return 0;
}

//----------------------------------------------------------------------------

void WorldClient::updateMeta()
{
	ASSERT( m_pMetaClient != NULL );

	// process messages..
	m_pMetaClient->update();

	if ( m_SessionId != 0 )
	{
		if (! m_pMetaClient->connected() && m_LoggedIn )
		{
			if ( m_ReconnectTime <= Time::seconds() )
			{
				m_ReconnectTime = Time::seconds() + 30;

				// no connection to the meta server, attempt to reconnect
				if (! m_pMetaClient->open() )
					pushChat( "<color;ffffff>CLIENT: Failed to connect to the meta-server..." );
				else
					pushChat( "<color;ffffff>CLIENT: Connected to the meta-server..." );
			}
		}

		if ( m_pMetaClient->connected() )
		{
			if ( !m_pMetaClient->loggedIn() )
			{
				switch( m_pMetaClient->login( m_SessionId ) )
				{
				case MetaClient::LOGIN_OKAY:
					{
						m_bBanned = false;
#ifdef _DEBUG
						Settings settings( "ClientD" );
#else
						Settings settings( "Client" );
#endif
						m_pMetaClient->selectGame(settings.get( "gameId", 1 ) );

						if ( m_pUser )
							m_pUser->onMetaLogin();
					}
					break;
				case MetaClient::LOGIN_BANNED:
					{
						m_bBanned = true;
						pushChat( "<color;ffffff>SERVER: This account is banned..." );
						m_pMetaClient->close();
						// close connection to the game server, so player will get disconnected screen
						close();
					}
					break;
				default:
					{
						m_bBanned = false;
						pushChat( "<color;ffffff>CLIENT: Failed to login to the meta-server..." );
						m_pMetaClient->close();
						m_ReconnectTime = Time::seconds() + 300;
					}
					break;
				}
			}

			m_pMetaClient->lock();
			if ( m_LastChatMessage > m_pMetaClient->chatCount() )
				m_LastChatMessage = 0;

			// avoid deadlock by pushing messages into a std::list, then unlocking and pushing into the WorldClient..
			std::list< MetaClient::Chat > push;
			for(;m_LastChatMessage<m_pMetaClient->chatCount();++m_LastChatMessage)
				push.push_back( m_pMetaClient->chat( m_LastChatMessage ) );
			m_pMetaClient->unlock();

			for( std::list<MetaClient::Chat>::iterator iPush = push.begin(); 
				iPush != push.end(); ++iPush )
			{
				pushMetaChat( *iPush );
			}
		}
	}
}

//----------------------------------------------------------------------------

CharString WorldClient::filterMetaChat( const char * pText )
{
	CharString filtered( pText );
	
	// strip BBCode
	RegExpM::regSearchReplace( filtered, "\\[(/)?(b|B|i|I)\\]", "" );
	RegExpM::regSearchReplace( filtered, "\\[(color=[0-9a-f][0-9a-f][0-9a-f][0-9a-f][0-9a-f][0-9a-f])\\]", "" );
	filtered.replace( "[/color]", "" );
	RegExpM::regSearchReplace( filtered, "\\[url\\](ht|f)(tp://[^ \\[\"\']+)\\[/url\\]", "\\1\\2" );
	RegExpM::regSearchReplace( filtered, "\\[url\\](https://[^ \\[\"\']+)\\[/url\\]", "\\1" );
	RegExpM::regSearchReplace( filtered, "\\[url\\](www[^ \\[\"\']+)\\[/url\\]", "http://\\1" );
	RegExpM::regSearchReplace( filtered, "\\[url\\](ftp[^ \\[\"\']+)\\[/url\\]", "ftp://\\1" );
	RegExpM::regSearchReplace( filtered, "\\[url=(ht|f)(tp://[^ \\[\"\']+)\\]([^\\[<]+)\\[/url\\]", "\\3 \\1\\2" );
	
	RegExpM::regSearchReplace( filtered, "\\[url=(www\\.[^ /\\[\"\'\\.]+\\.[^<\\[\"\']+)\\]([^\\[<]+)\\[/url\\]", "\\2 http://\\1" );
	RegExpM::regSearchReplace( filtered, "\\[url=(ftp\\.[^ /\\[\"\'\\.]+\\.[^<\\[\"\']+)\\]([^\\[<]+)\\[/url\\]", "\\2 ftp://\\1" );

	// strip italics tags
	RegExpM::regSearchReplace( filtered, "<(/)?(i|I)>", "" );
	// escape all opening brackets <
	RegExpM::regSearchReplace( filtered, "<", "<<>" );
	
	// reconvert escaped brackets for allowed tags
	filtered.replace( "<<>b>", "<color;00ffff>" );
	filtered.replace( "<<>/b>", "<color;00aaaa>" );
	filtered.replace( "<<>/font>", "</font>" );
	RegExpM::regSearchReplace( filtered, "<<>font (color=[0-9a-f][0-9a-f][0-9a-f][0-9a-f][0-9a-f][0-9a-f])>", "<font color=\\1>" );

	if( filtered.find( "<color;" ) )		// if there were colortags used
		filtered += "</color>";					// make sure to switch back to default color at the end of line

	
	return filtered;

}

//----------------------------------------------------------------------------
// EOF
