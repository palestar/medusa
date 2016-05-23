/*
	WorldServerMessage.cpp
	(c)2005 Palestar Inc, Richard Lyle
*/


#include "Debug/Assert.h"
#include "Debug/Profile.h"
#include "Standard/Time.h"
#include "Math/Helpers.h"
#include "WorldServer.h"
#include "NounTarget.h"

//! Define this to > 0 to time how long it takes to process a received message, if message takes longer than that amount in milliseconds
//! then a warning is generated and pushed into the server log...
#define MESSAGE_WARNING_TIME		50

//----------------------------------------------------------------------------

// called by updateClients() normally... server is already expected to be locked
void WorldServer::receiveMessage( ClientContext & context, bool bUDP, byte nMessage, const InStream & input )
{
#if MESSAGE_WARNING_TIME > 0
	dword nStart = Time::milliseconds();
#endif

	dword nClientId = context.nClientId;
	if ( nClientId == 0 )
		throw FileSocket::FileError();			// client has been disconnected, force this to stop..

	switch( nMessage )
	{
	case WorldClient::SERVER_SEND_VERSION:
		{
			TRACE( "SERVER_SEND_VERSION" );

			dword job;
			input >> job;

			// send our version to the client
			send( context.nClientId, WorldClient::CLIENT_RECV_VERSION ) 
				<< job << version() << context.nClientId;
		}
		break;
	case WorldClient::SERVER_LOGIN:
		{
			TRACE( "SERVER_LOGIN" );

			dword nVersion;
			input >> nVersion;

			if ( version() == nVersion )
			{
				LoginJob & job = m_LoginQueue.push();
				job.bDone = false;
				job.nClientId = context.nClientId;

				input >> job.nSessionId;		// client is sending it's key from the MetaServer
				input >> job.bTransfer;
				input >> job.bProxy;

				if ( job.bProxy )
				{
					input >> job.nProxyId;
					input >> job.nProxyFaction;
				}
				
				input >> job.bServer;

				// if we are in tutorial mode or session id is 0 and from the localhost, then run without login information..
				if ( m_Context.bTutorial || (job.nSessionId == 0 
					&& strcmp( clientAddress( context.nClientId ), "127.0.0.1" ) == 0) )
				{
					// initialize "Cadet" account, allowed on tutorial servers only!
					job.bDone = true;
					job.profile.userId = 0xffffffff;
					job.profile.sessionId = job.nSessionId;
					job.profile.clanId = 0;
					job.profile.flags = MetaClient::DEMO|MetaClient::SUBSCRIBED|MetaClient::ADMINISTRATOR;
					job.profile.name = "Cadet";
					job.profile.score = 0.0f;
				}
			}
			else
			{
				LOG_STATUS( "WorldServer", CharString().format( "Client %u invalid version, disconnecting", context.nClientId ) );
				Server::removeClient( context.nClientId );
			}
		}
		break;
	case WorldClient::SERVER_SET_MODERATE:
		{
			TRACE( "SERVER_SET_MODERATE" );

			bool bModerate;
			input >> bModerate;

			if ( validateClient( context, m_Context.nModFlags ) )
				setModerate( context.nClientId, bModerate );
		}
		break;
	case WorldClient::SERVER_SEND_PROFILE:
		{
			TRACE( "SERVER_SEND_PROFILE" );
			if ( validateClient( context, m_Context.nClientFlags ) )
				sendProfile( context.nClientId );
		}
		break;
	case WorldClient::SERVER_SEND_SERVER_STATUS:
		{
			TRACE( "SERVER_SEND_SERVER_STATUS" );
			if ( validateClient( context, m_Context.nClientFlags ) )
				sendServerStatus( context.nClientId );
		}
		break;
	case WorldClient::SERVER_TRANSFER:
		{
			WorldClient::TransferRequest req;
			input >> req;

			LOG_STATUS( "WorldServer", "Client %u initiated transfer to %s:%d!", 
				context.nClientId, req.m_sAddress.cstr(), req.m_nPort );
			if (! transferClient( context.nClientId, req ) )
				LOG_ERROR( "WorldServer", "Transfer failed!" );
		}
		break;
	case WorldClient::SERVER_SET_FOCUS:
		{
			TRACE( "SERVER_SET_FOCUS" );

			WidgetKey nFocus;
			input >> nFocus;

			if ( validateClient( context, m_Context.nClientFlags ) )
				setClientFocus( context.nClientId, context.nClientId, nFocus );
		}
		break;
	case WorldClient::SERVER_SET_TARGET:
		{
			TRACE( "SERVER_SET_TARGET" );

			WidgetKey nTarget;
			input >> nTarget;

			if ( validateClient( context, m_Context.nClientFlags ) )
				setClientTarget( context.nClientId, context.nClientId, nTarget );
		}
		break;
	case WorldClient::SERVER_SET_TEAM:
		{
			int nTeamId;
			input >> nTeamId;

			TRACE( "SERVER_SET_TEAM" );
			if ( validateClient( context, m_Context.nModFlags ) )
				setClientTeam( context.nClientId, context.nClientId, nTeamId );
		}
		break;
	case WorldClient::SERVER_SET_SELF:
		{
			TRACE( "SERVER_SET_SELF" );
			WidgetKey nSelf;
			input >> nSelf;

			if ( validateClient( context, m_Context.nModFlags ) )		// only moderators can set their ship
				setClientSelf( context.nClientId, context.nClientId, nSelf );
		}
		break;
	case WorldClient::SERVER_SELECT_TEAM:
		{
			TRACE( "SERVER_SELECT_TEAM" );

			int nTeamId;
			input >> nTeamId;

			if ( validateClient( context, m_Context.nClientFlags ) )
				selectTeam( context.nClientId, nTeamId );
		}
		break;
	case WorldClient::SERVER_SELECT_SELF:
		{
			TRACE( "SERVER_SELECT_SELF" );

			WidgetKey nSpawn;
			input >> nSpawn;
			WidgetKey nSelf;
			input >> nSelf;
			Array< WidgetKey > items;
			input >> items;

			if ( validateClient( context, m_Context.nClientFlags ) )
			{
				if (! selectSelf( context.nClientId, nSpawn, nSelf, items ) )
				{
					// failed to select players ship, force them back to the team/ship selection screen...
					setClientSelf( 0, context.nClientId, NULL_WIDGET );
				}
			}
		}
		break;
	case WorldClient::SERVER_SELECT_PROXY:
		{
			TRACE( "SERVER_SELECT_SELF" );

			WidgetKey nSelf( NULL_WIDGET );
			input >> nSelf;
			int nFactionId = 0;
			input >> nFactionId;

			if ( validateClient( context, m_Context.nClientFlags ) )
			{
				if (! createProxy( context, nSelf, nFactionId ) )
					LOG_ERROR( "WorldServer", "ERROR: Failed to select proxy!" );
			}
		}
		break;
	case WorldClient::SERVER_REMOVE_SELF:
		{
			TRACE( "SERVER_REMOVE_SELF" );

			if ( validateClient( context, m_Context.nClientFlags ) )
			{
				Noun * pSelf = context.pSelf;
				if ( pSelf != NULL )
				{
					pSelf->setDetach();
					context.pSelf = NULL;
				}
			}
		}
		break;
	case WorldClient::SERVER_REMOVE_STORAGE:
		{
			TRACE( "SERVER_REMOVE_STORAGE" );

			Array< WidgetKey > remove;
			input >> remove;
		
			if ( validateClient( context, m_Context.nClientFlags ) )
			{
				dword nUserId = context.nUserId; 
				for(int i=0;i<remove.size();i++)
					deleteStorage( 0, nUserId, remove[i] );
			}
		}
		break;
	case WorldClient::SERVER_RECV_CHAT:
		{
			TRACE( "SERVER_RECV_CHAT" );

			CharString chat;
			input >> chat;

			if ( validateClient( context, m_Context.nClientFlags ) )
				receiveChat( context, chat );
		}
		break;
	case WorldClient::SERVER_RECV_VERB:
		{
			TRACE( "SERVER_RECV_VERB" );

			WidgetKey nTarget;
			input >> nTarget;
			WidgetWrap verb;
			input >> verb;

			if ( validateClient( context, m_Context.nClientFlags ) )
				addVerb( context, nTarget, verb, 0 );
		}
		break;
	case WorldClient::SERVER_SEND_NOUN:
		{
			WidgetKey nNoun;
			input >> nNoun;

			WidgetReference<Widget> pWidget( nNoun );
			if ( pWidget.valid() )
			{
				Noun * pRequestedNoun = WidgetCast<Noun>( pWidget );
				if ( pRequestedNoun != NULL )
				{
					BaseNode * pParent = pRequestedNoun->parent();
					if ( pParent != NULL )
					{
						WidgetWrap instance;
						instance.wrap( pRequestedNoun, FF_TRANSMIT, m_pDictionary );

						sendAddNoun( context.nClientId, pParent->key(), nNoun, instance );
						context.updateHash[ pRequestedNoun->key() ] = pRequestedNoun->version();

						LOG_DEBUG_LOW( "WorldServer", "Sending requested noun (%llu,%s) to client %u (%u Bytes)", 
							nNoun.m_Id, pRequestedNoun->className(), context.nClientId, instance.size() );
					}
					else
						LOG_DEBUG_LOW( "WorldServer", "Client %u requested node with no parent (%s)", context.nClientId, pRequestedNoun->className() );
				}
				else
					LOG_DEBUG_LOW( "WorldServer", "Client %u requested Invalid Widget type %s", context.nClientId, pWidget->className() );
			}
			else
				LOG_DEBUG_LOW( "WorldServer", "Client %u requested unknown node (%llu)", context.nClientId, nNoun.m_Id );
		}
		break;

	//---------------------------------------------------------------------------------------------------

	case WorldClient::GROUP_INVITE:	// sent by the client to invite a player into a group or accept a group invite.
		{
			Noun::wRef pTarget;
			input >> pTarget;

			if ( validateClient( context, m_Context.nClientFlags ) )
				groupInvite( context, pTarget );
		}
		break;
	case WorldClient::GROUP_DECLINE:
		{
			Noun::wRef pTarget;
			input >> pTarget;

			if ( validateClient( context, m_Context.nClientFlags ) )
				groupDecline( context, pTarget );
		}
		break;
	case WorldClient::GROUP_REMOVE:	// sent by the client to remove another member or themselves from the group.
		{
			Noun::tRef pRemove;
			input >> pRemove;

			if ( validateClient( context, m_Context.nClientFlags ) )
				groupRemove( context, pRemove );
		}
		break;
	case WorldClient::GROUP_PROMOTE:	// sent by the client/server to make another player the leader
		{
			Noun::tRef pPromote;
			input >> pPromote;

			if ( validateClient( context, m_Context.nClientFlags ) )
				groupPromote( context, pPromote );
		}
		break;

	//----------------------------------------------------------------------------

	case WorldClient::CONTEXT_INIT:
		{
			TRACE( "CONTEXT_INIT" );

			WidgetWrap worldContext;
			input >> worldContext;

			if ( validateClient( context, m_Context.nServerFlags ) )
				contextInitialize( context.nClientId, worldContext );
		}
		break;
	case WorldClient::CONTEXT_END:
		{
			TRACE( "CONTEXT_END" );

			CharString sEndMessage;
			input >> sEndMessage;

			if ( validateClient( context, m_Context.nServerFlags ) )
				contextEnd( context.nClientId, sEndMessage );
		}
		break;
	case WorldClient::CONTEXT_SET_NAME:
		{
			TRACE( "CONTEXT_SET_NAME" );

			CharString sName;
			input >> sName;

			if ( validateClient( context, m_Context.nServerFlags ) )
				contextName( context.nClientId, sName );
		}
		break;
	case WorldClient::CONTEXT_SET_DESC:
		{
			TRACE( "CONTEXT_SET_DESC" );

			CharString sDesc;
			input >> sDesc;

			if ( validateClient( context, m_Context.nServerFlags ) )
				contextDescription( context.nClientId, sDesc );
		}
		break;
	case WorldClient::CONTEXT_SET_TYPE:
		{
			TRACE( "CONTEXT_SET_TYPE" );

			int nType;
			input >> nType;

			if ( validateClient( context, m_Context.nServerFlags ) )
				contextType( context.nClientId, nType );
		}
		break;
	case WorldClient::CONTEXT_SET_SCRIPT:
		{
			TRACE( "CONTEXT_SET_SCRIPT" );

			CharString pScript;
			input >> pScript;

			if ( validateClient( context, m_Context.nServerFlags ) )
				contextScript( context.nClientId, pScript );
		}
		break;
	case WorldClient::CONTEXT_ADD_TEAM:
		{
			TRACE( "CONTEXT_ADD_TEAM" );
			Team team;
			input >> team;

			if ( validateClient( context, m_Context.nServerFlags ) )
				addTeam( context.nClientId, team );
		}
		break;
	case WorldClient::CONTEXT_SET_FLEET_FLAGS:
		{
			TRACE( "CONTEXT_SET_FLEET_FLAGS" );

			int nTeamId;
			input >> nTeamId;
			dword nFlags;
			input >> nFlags;

			if ( validateClient( context, m_Context.nServerFlags ) )
				setTeamFlags( context.nClientId, nTeamId, nFlags );
		}
		break;
	case WorldClient::CONTEXT_DEL_FLEET:
		{
			TRACE( "CONTEXT_DEL_FLEET" );

			int nTeamId;
			input >> nTeamId;

			if ( validateClient( context, m_Context.nServerFlags ) )
				deleteTeam( context.nClientId, nTeamId );
		}
		break;
	case WorldClient::CONTEXT_ADD_CONDITION:
		{
			TRACE( "CONTEXT_ADD_CONDITION" );

			Condition condition;
			input >> condition;

			if ( validateClient( context, m_Context.nServerFlags ) )
				addCondition( context.nClientId, condition );
		}
		break;
	case WorldClient::CONTEXT_DEL_CONDITION:
		{
			TRACE( "CONTEXT_DEL_CONDITION" );

			int nConditionId;
			input >> nConditionId;

			if ( validateClient( context, m_Context.nServerFlags ) )
				deleteCondition( context.nClientId, nConditionId );
		}
		break;
	case WorldClient::CONTEXT_SET_TIME_LIMIT:
		{
			TRACE( "CONTEXT_SET_TIME_LIMIT" );

			dword nTimeLimit;
			input >> nTimeLimit;

			if ( validateClient( context, m_Context.nServerFlags ) )
				setTimeLimit( context.nClientId, nTimeLimit );
		}
		break;
	case WorldClient::CONTEXT_ADD_WORLD:
		{
			TRACE( "CONTEXT_ADD_CONTEXT" );

			WidgetWrap world;
			input >> world;

			if ( validateClient( context, m_Context.nServerFlags ) )
				addWorld( context.nClientId, world );
		}
		break;
	case WorldClient::CONTEXT_DEL_WORLD:
		{
			TRACE( "CONTEXT_DEL_CONTEXT" );

			WidgetKey nWorld;
			input >> nWorld;

			if ( validateClient( context, m_Context.nServerFlags ) )
				deleteWorld( context.nClientId, nWorld );
		}
		break;
	case WorldClient::CONTEXT_ADD_ZONE:
		{
			TRACE( "CONTEXT_ADD_ZONE" );

			WidgetWrap zone;
			input >> zone;

			if ( validateClient( context, m_Context.nServerFlags ) )
				addZone( context.nClientId, zone );
		}
		break;
	case WorldClient::CONTEXT_DEL_ZONE:
		{
			TRACE( "CONTEXT_DEL_ZONE" );

			WidgetKey nZone;
			input >> nZone;

			if ( validateClient( context, m_Context.nServerFlags ) )
				deleteZone( context.nClientId, nZone );
		}
		break;
	case WorldClient::CONTEXT_ADD_NOUN:
		{
			TRACE( "CONTEXT_ADD_NOUN" );

			WidgetKey nParent;
			input >> nParent;
			WidgetKey nNoun;
			input >> nNoun;
			WidgetWrap instance;
			input >> instance;

			if ( validateClient( context, m_Context.nServerFlags ) )
				addNoun( context.nClientId, nParent, nNoun, instance );
		}
		break;
	case WorldClient::CONTEXT_DESYNC_NOUN:		// update a nouns current position
		{
			TRACE( "CONTEXT_SET_NOUN_POSITION" );

			WidgetKey nNoun;
			input >> nNoun;

		}
		break;
	case WorldClient::CONTEXT_DEL_NOUN:
		{
			TRACE( "CONTEXT_DEL_NOUN" );

			WidgetKey nNoun;
			input >> nNoun;
			dword nLastTick;
			input >> nLastTick;

			if ( validateClient( context, m_Context.nServerFlags ) )
				deleteNoun( context.nClientId, nNoun, nLastTick );
		}
		break;
	case WorldClient::CONTEXT_ADD_VERB:
		{
			TRACE( "CONTEXT_ADD_VERB" );

			WidgetKey nTarget;
			input >> nTarget;
			WidgetWrap verb;
			input >> verb;

			if ( validateClient( context, m_Context.nServerFlags ) )
				addVerb( context, nTarget, verb, 0 );
		}
		break;
	case WorldClient::CONTEXT_CRON:
		{
			TRACE( "CONTEXT_CRON" );

			dword nTick;
			input >> nTick;

			if ( validateClient( context, m_Context.nServerFlags ) )
			{
				// syncronize the worldContext tick with the master
				worldContext()->cron( nTick );

				// forward current tick onto all clients
				for(int i=0;i<clientCount();i++)
				{
					dword nClientID = client(i);
					if ( nClientID != context.nClientId )
						sendClient( nClientID, WorldClient::CONTEXT_CRON, true, true ) << nTick;
				}
			}
		}
		break;
	case WorldClient::CONTEXT_RUN_SCRIPT:
		{
			TRACE( "CONTEXT_RUN_SCRIPT" );

			UniqueNumber nScriptId;
			input >> nScriptId;
			CharString sName;
			input >> sName;
			CharString sScript;
			input >> sScript;

			if ( validateClient( context, m_Context.nServerFlags ) )
			{
				LOG_STATUS( "WorldServer", "Client %u (%s) running script %s:", nClientId, context.MetaProfile.name.cstr(), sName.cstr() );
				LOG_STATUS( "WorldServer", sScript.cstr() );

				worldContext()->runScript( nScriptId, sName, sScript );
			}
		}
		break;
	case WorldClient::CONTEXT_KILL_SCRIPT:
		{
			TRACE( "CONTEXT_KILL_SCRIPT" );

			UniqueNumber nScriptId;
			input >> nScriptId;

			if ( validateClient( context, m_Context.nServerFlags ) )
			{
				worldContext()->killScript( nScriptId );

				// forward kill onto all other clients
				for(int i=0;i<clientCount();i++)
					if ( client(i) != context.nClientId && isValid( client(i) ) )
						send( client(i), WorldClient::CONTEXT_KILL_SCRIPT ) << nScriptId;
			}
		}
		break;

	//---------------------------------------------------------------------------------------------------

	case WorldClient::UDP_ACK_CONNECT:
		{
			dword nAckId;
			input >> nAckId;

			LOG_STATUS( "WorldServer", CharString().format("Client %u sent UDP_ACK_CONNECT with ID of %u", nClientId, nAckId) );
		}
		break;
	//----------------------------------------------------------------------------

	case WorldClient::PING:
		{
			dword nTime;
			input >> nTime;

			if ( bUDP )
				sendUDP( context.nClientId, WorldClient::PONG ) << nTime;
			else
				send( context.nClientId, WorldClient::PONG ) << nTime;
		}
		break;
	case WorldClient::PONG:
		{
			dword nTime;
			input >> nTime;

			//TRACE( "PONG" );
			receivePong( context, nTime, bUDP );
		}
		break;
	default:
		// pass unknown messages into a virtual function so a derived class may handle those if it chooses..
		onMessage( context, bUDP, nMessage, input );
		break;
	}

#if MESSAGE_WARNING_TIME > 0
	dword nElapsed = Time::milliseconds() - nStart;
	if ( nElapsed > MESSAGE_WARNING_TIME )
	{
		LOG_WARNING( "WorldServer",CharString().format( "WARNING: Message 0x%0.2x from client %u took %u ms, bUDP = %s", 
			nMessage, nClientId, nElapsed, bUDP ? "true" : "false" ) );
	}
#endif
}

void WorldServer::onMessage( ClientContext & context, bool bUDP, byte nMessage, const InStream & input )
{
	// unknown data, possible hack into the server
	LOG_ERROR( "WorldServer", CharString().format("Unknown message 0x%x received from client %d, disconnecting", nMessage, context.nClientId) );
	Server::removeClient( context.nClientId );
}

//----------------------------------------------------------------------------

bool WorldServer::receiveChat( ClientContext & context, const char * pChat )
{
	if ( pChat == NULL )
		return false;
	if ( strlen( pChat ) == 0 )
		return false;
	if ( context.nUserId == 0 )
		return false;

	// get the userId of the sender
	dword nUserId = context.nUserId;
	// log the chat message received
	onChat( NULL, CharString().format( "%s: \"%s\"", context.MetaProfile.name.cstr(), pChat) );

	// forward all chat/commands received from clients to any clients in moderator mode
	List<dword>::Iterator iMod = m_Moderators.head();
	if ( iMod.valid() )
	{
		CharString chat;
		chat.format( "> %s : %s", context.MetaProfile.name.cstr(), pChat );

		while( iMod.valid() )
		{
			sendChat( *iMod, chat );
			iMod++;
		}
	}

	// check if this is a command or not, if not then send team chat by default
	return processCommand( context, pChat );
}

//---------------------------------------------------------------------------------------------------

void WorldServer::setModerate( dword nClientId, bool bModerate )
{
	LOG_STATUS( "WorldServer", CharString().format("Client %u moderator mode %s", nClientId, bModerate ? "ON" : "OFF") );

	// get the client context
	ClientContext & context = m_ClientContext[ nClientId ];
	if ( context.bModerate != bModerate )
	{
		if ( context.bModerate = bModerate )
			m_Moderators.insert( nClientId );
		else
			m_Moderators.remove( nClientId );
	}
	sendChat( nClientId, CharString().format( "Moderator mode is %s", bModerate ? "ON" : "OFF" ) );
}

void WorldServer::sendProfile( dword nClientId )
{
	ClientContextHash::iterator iClient = m_ClientContext.find( nClientId );
	if ( iClient != m_ClientContext.end() )
		send( nClientId, WorldClient::CLIENT_SET_PROFILE ) << iClient->second.pProfile;
}

void WorldServer::sendServerStatus( dword nClientId )
{
	bool isModerator = (getFlags( nClientId ) & MetaClient::MODERATOR) != 0;

	ServerStatus status;
	status.gameId = m_Context.gameId;
	status.name = m_Context.name;
	status.address = m_Context.address;
	status.port = m_Context.port;
	status.upTime = serverTime();
	status.bytesRecv = bytesReceived();
	status.bytesSent = bytesSent();
	status.maxClients = maxClients();
	status.maxRank = m_Context.maxRank;
	status.bPersistant = m_Context.bPersistant;
	status.bUseShipFlags = m_Context.bUseShipFlags;
	status.sShipFlags = m_Context.sShipFlags;

	// only send client data to moderators..
	if ( isModerator )
	{
		status.clients.allocate( clientCount() );
		for(int i=0;i<clientCount();i++)
		{
			dword nClientId = client( i );

			const ClientContext & context = m_ClientContext[ nClientId ];
			status.clients[ i ].name = context.MetaProfile.name;
			status.clients[ i ].userId = context.MetaProfile.userId;
			status.clients[ i ].latency = context.nLatency;
			status.clients[ i ].address = clientAddress( nClientId );		// don't show IP addresses to non-moderators
		}
	}

	send( nClientId, WorldClient::CLIENT_RECV_SERVER_STATUS ) << status;
}

bool WorldServer::setClientFocus( dword nSenderId, dword nClientId, const WidgetKey & nFocus )
{
	ClientContextHash::iterator iContext = m_ClientContext.find( nClientId );
	if ( iContext == m_ClientContext.end() )
		return false;

	ClientContext & context = iContext->second;
	context.pFocus = nFocus;
	if ( nSenderId != nClientId )
		send( nClientId, WorldClient::CLIENT_SET_FOCUS ) << nFocus;

	return true;
}

bool WorldServer::setClientFocusArea( dword nSenderId, dword nClientId, float fFocusArea )
{
	ClientContextHash::iterator iContext = m_ClientContext.find( nClientId );
	if ( iContext == m_ClientContext.end() )
		return false;

	ClientContext & context = iContext->second;
	context.fFocusArea = fFocusArea;
	if ( nSenderId != nClientId )
		send( nClientId, WorldClient::CLIENT_SET_FOCUS_AREA ) << fFocusArea;

	return true;
}

bool WorldServer::setClientTarget( dword nSenderId, dword nClientId, const WidgetKey & nTarget )
{
	ClientContextHash::iterator iContext = m_ClientContext.find( nClientId );
	if ( iContext == m_ClientContext.end() )
		return false;

	ClientContext & context = iContext->second;
	context.pTarget = nTarget;
	if ( nSenderId != nClientId )
		send( nClientId, WorldClient::CLIENT_SET_TARGET ) << nTarget;

	return true;
}

bool WorldServer::setClientTeam( dword nSenderId, dword nClientId, int nTeamId )
{
	ClientContextHash::iterator iContext = m_ClientContext.find( nClientId );
	if ( iContext == m_ClientContext.end() )
		return false;

	ClientContext & context = iContext->second;

	// firstly, remove the hash of the previous team and faction if any...
	if ( context.nFactionId != 0 )
		m_FactionClientHash[ context.nFactionId ].erase( nClientId );
	if ( context.nTeamId != 0 )
		m_TeamClientHash[ context.nTeamId ].erase( nClientId );

	context.nTeamId = nTeamId;
	context.nFactionId = worldContext()->teamFaction( nTeamId );

	// now, rehash the team & faction for this client...
	if ( context.nFactionId != 0 )
		m_FactionClientHash[ context.nFactionId ].insert( nClientId );
	if ( context.nTeamId != 0 )
		m_TeamClientHash[ context.nTeamId ].insert( nClientId );

	if ( nSenderId != nClientId )
		send( nClientId, WorldClient::CLIENT_SET_TEAM ) << nTeamId;

	onJoinTeam( context, nTeamId );
	return true;
}

bool WorldServer::setClientSelf( dword nSenderId, dword nClientId, const WidgetKey & nSelf )
{
	ClientContextHash::iterator iContext = m_ClientContext.find( nClientId );
	if ( iContext == m_ClientContext.end() )
		return false;

	ClientContext & context = iContext->second;
	// NOTE: Commented out the following lines because on destruction, the player wasn't receiving any
	// kamakaze points because their user id was getting cleared from their ship by this function.
	//if ( context.pSelf.valid() )
	//	context.pSelf->setUserId( 0 );
	context.pSelf = nSelf;
	if ( context.pSelf.valid() )
		context.pSelf->setUserId( context.nUserId );

	if ( nSenderId != nClientId )
		send( nClientId, WorldClient::CLIENT_SET_SELF ) << nSelf;

	return true;
}

bool WorldServer::selectTeam( dword nClientId, int nTeamId )
{
	ClientContextHash::iterator iContext = m_ClientContext.find( nClientId );
	if ( iContext == m_ClientContext.end() )
		return false;

	ClientContext & context = iContext->second;
	if ( context.nUserId == 0 || !context.pProfile )
		return false;

	if (! worldContext()->isTeamValid( nTeamId ) )
	{
		if ( context.nTeamId != 0 && (context.pProfile->isModerator() || context.pProfile->isDeveloper() || context.pProfile->isAdmin()) )
		{
			sendChat( nClientId, "<color;ffffff>SERVER: Leaving team." );
			setClientTeam( 0, nClientId, 0 );
			return true;
		}

		sendChat( nClientId, CharString( "<color;ffffff>SERVER: Team is invalid!" ) );
		return false;
	}

	if ( !(context.pProfile->isModerator() || context.pProfile->isAdmin() || context.pProfile->isDeveloper()) )
	{
		if ( !worldContext()->canJoin( nTeamId ))
		{
			sendChat( nClientId, CharString( "<color;ffffff>SERVER: Team is locked!" ) );
			return false;
		}
		if ( !canJoinTeam( context, nTeamId ) )
		{
			sendChat( nClientId, "<color;ffffff>SERVER: Team is not available or is full!" );
			return false;
		}
	}

	const WorldContext::Team & team = worldContext()->team( nTeamId );
	sendChat( nClientId, CharString().format( "<color;ffffff>SERVER: Joined team '%s'", team.name.cstr() ) );

	setClientTeam( 0, nClientId, nTeamId );
	LOG_STATUS( "WorldServer", "Client %u joined team %u (%s)", nClientId, nTeamId, team.name.cstr() );

	if (! checkClientGroup( context ) )
		LOG_ERROR( "WorldServer", "ERROR: Failed to check client group!" );

	return true;
}

bool WorldServer::canJoinTeam( ClientContext & client, int nTeamId )
{
	const WorldContext::Team & team = worldContext()->team( nTeamId );
	if ( team.clanId != 0 && client.MetaProfile.clanId != team.clanId )
		return false;		// don't let player's join the teams of other clans..

	return true;
}

void WorldServer::onJoinTeam( ClientContext & client, int nTeamId )
{}

bool WorldServer::selectSelf( dword nClientId, 
							 const WidgetKey & nSpawn, 
							 const WidgetKey & nSelf, 
							 const Array< WidgetKey > & items )
{
	bool bNewSpawn = true;

	ClientContextHash::iterator iContext = m_ClientContext.find( nClientId );
	if ( iContext == m_ClientContext.end() )
		return false;

	ClientContext & context = iContext->second;
	if ( context.nUserId == 0 )
		return false;		// user not logged in...

	// don't spawn if we already have an object in game..
	if ( context.pSelf.valid() && 
		context.pSelf->parent() && 
		context.pSelf->teamId() == context.nTeamId )
	{
		setClientSelf( 0, nClientId, context.pSelf->key() );
		setClientFocus( 0, nClientId, context.pSelf->key() );
		return true;
	}

	// first, find the spawn position
	Noun::wRef pSpawnPoint = nSpawn;
	if (! pSpawnPoint.valid() )
		return false;
	if (! pSpawnPoint->zone() )
		return false;
	
	// unwrap any storage that is in the items list
	Array< Noun::Ref > Storage;

	UserStorage * pUserStorage = context.pStorage;
	if (! pUserStorage )
		return false;

	if ( pUserStorage->isValid( nSelf ) )
	{
		dword nFactionId = getStorageFactionId( context.nUserId, nSelf );

		if ( context.nTeamId == 0 )
		{
			// if the player has no team selected, then auto select a team based on the faction ID of the item we're using from storage...
			int nBestTeamID = 0;
			if ( m_Context.bClan && context.MetaProfile.clanId != 0 )
				nBestTeamID = worldContext()->teamClan( context.MetaProfile.clanId, nFactionId );
			if ( nBestTeamID == 0 )
				nBestTeamID = worldContext()->factionTeam( nFactionId );

			if (! selectTeam( nClientId, nBestTeamID ) )
			{
				LOG_ERROR( "WorldServer", CharString().format("ERROR: Failed to select team %d for client %u", nBestTeamID, context.nClientId ) );
				return false;
			}
		}
		else if ( context.nFactionId != nFactionId )
		{
			// object player is trying to use is not of the correct faction!
			TRACE( "Stored ship is not the correct faction!" );
			return false;
		}

		bNewSpawn = false;
		// NOTE: The player's self must always be index 0 because below we attach all these items to the spawned object
		// obviously, we don't want to attach the selected ship onto the copied ship..
		Storage.push( getStorageNoun( context.nUserId, nSelf ) );
	}

	if ( !worldContext()->isTeamValid( context.nTeamId ) )
		return false;

	// now add any items to pull from storage that will be attached to the avatar
	for(int i=0;i<items.size();i++)
		Storage.push( getStorageNoun( context.nUserId, items[i] ) );

	// find the ship to spawn, the object is either in the noun list of the faction or out of the players storage
	Noun::Ref pSpawn = Noun::wRef( nSelf ).pointer();
	if (! pSpawn )
		return false;

	// call application virtual function to see if we can spawn the ship...
	if (! canSpawn( context, pSpawnPoint, pSpawn, Storage ) )
		return false;

	// allow for application specific code before we attach the noun to the zone..
	if (! onSpawn( context, pSpawnPoint, pSpawn, bNewSpawn, Storage ) )
		return false;

	return true;
}

bool WorldServer::canSpawn( ClientContext & context, Noun * pSpawnPoint, Noun * pSpawn, 
						   Array< Noun::Ref > & Storage )
{
	// DO NOT LET ADMINS/DEVS BYPASS THE QUEUE BECAUSE THE CODE IS NOT SETUP TO ALLOW A PLAYER
	// TO SKIP AHEAD IN THE QUEUE YET..
	if ( context.nQueuePosition > 0 )
	{
		sendChat( context.nClientId, CharString().format("<color;ffffff>SERVER: Server is full, you are %d of %d player(s) in the queue...",
			context.nQueuePosition, m_nPlayersInQueue ) );
		return false;		
	}

	return true;
}

bool WorldServer::onSpawn( ClientContext & context, Noun * pSpawnPoint, Noun * pSpawn, bool bNewSpawn, Array< Noun::Ref > & Storage )
{
	if (! pSpawnPoint->zone() )
		return false;

	// now finally, attach the ship to the zone
	pSpawnPoint->zone()->attachNoun( pSpawn );
	// set the players ship and focus
	setClientSelf( 0, context.nClientId, pSpawn->key() );
	setClientFocus( 0, context.nClientId, pSpawn->key() );

	// update the players group if any...
	Group * pGroup = context.pStorage->group();
	if ( pGroup != NULL )
	{
		GroupMember * pMember = pGroup->findMember( context.nUserId );
		if ( pMember != NULL )
		{
			pMember->setType( getNounType( pSpawn ) );
			pMember->setObjectId( pSpawn->key() );

			// update all group members with the new data..
			sendGroupUpdate( pGroup );
		}
	}

	return true;
}

//----------------------------------------------------------------------------

bool WorldServer::addStorage( dword nClientId, dword nUserId, const WidgetKey & nKey, const Storage & storage )
{
	UserStorage * pStorage = getStorage( nUserId );
	if (! pStorage )
		return false;
	if (! pStorage->add( nKey, storage ) )
		return false;

	// send storage item to the client...
	dword nSendId = getClientId( nUserId );
	if ( nSendId != 0 && nSendId != nClientId )
		send( nSendId, WorldClient::CLIENT_ADD_STORAGE ) << nKey << storage;

	return true;
}

bool WorldServer::deleteStorage( dword nClientId, dword nUserId, const WidgetKey & nKey )
{
	UserStorage * pStorage = getStorage( nUserId );
	if (! pStorage )
		return false;
	if (! pStorage->remove( nKey ) )
		return false;		// item not in storage!

	dword nSendId = getClientId( nUserId );
	if ( nSendId != 0 && nSendId != nClientId )
		send( nSendId, WorldClient::CLIENT_DEL_STORAGE ) << nKey;

	return true;
}

//----------------------------------------------------------------------------

void WorldServer::addTeam( dword nClientId, Team & team )
{
	// add the new team to our worldContext
	worldContext()->addTeam( team );

	// forward the change onto all clients
	for(int i=0;i<clientCount();i++)
		if ( client(i) != nClientId && isValid( client(i) ) )
			send( client(i),WorldClient::CONTEXT_ADD_TEAM ) << team;
}

void WorldServer::setTeamFlags( dword nClientId, int nTeamId, dword nFlags )
{
	worldContext()->setTeamFlags( nTeamId, nFlags );
	// forward the change onto all clients
	for(int i=0;i<clientCount();i++)
		if ( client(i) != nClientId && isValid( client(i) ) )
			send( client(i),WorldClient::CONTEXT_SET_FLEET_FLAGS ) << nTeamId << nFlags;
}

void WorldServer::deleteTeam( dword nClientId, int nTeamId )
{
	worldContext()->deleteTeam( nTeamId );
	for(int i=0;i<clientCount();i++)
		if ( client(i) != nClientId && isValid( client(i) ) )
			send( client(i), WorldClient::CONTEXT_DEL_FLEET ) << nTeamId;
}

void WorldServer::addCondition( dword nClientId, Condition & condition )
{
	worldContext()->addCondition( condition );
	for(int i=0;i<clientCount();i++)
		if ( client(i) != nClientId && isValid( client(i) ) )
			send( client(i), WorldClient::CONTEXT_ADD_CONDITION ) << condition;
}

void WorldServer::deleteCondition( dword nClientId, int nConditionId )
{
	worldContext()->deleteCondition( nConditionId );
	for(int i=0;i<clientCount();i++)
		if ( client(i) != nClientId && isValid( client(i) ) )
			send( client(i), WorldClient::CONTEXT_DEL_CONDITION ) << nConditionId;
}

void WorldServer::setTimeLimit( dword nClientId, dword nTimeLimit )
{
	worldContext()->setTimeLimit( nTimeLimit );
	for(int i=0;i<clientCount();i++)
		if ( client(i) != nClientId && isValid( client(i) ) )
			send( client(i), WorldClient::CONTEXT_SET_TIME_LIMIT ) << nTimeLimit;
}

void WorldServer::addWorld( dword nClientId, WidgetWrap & world )
{
	Widget::Ref pUncasted = world.unwrap();
	if ( pUncasted.valid() )
	{
		WorldContext * pWorld = WidgetCast<WorldContext>( pUncasted.pointer() );
		if ( pWorld != NULL )
		{
			worldContext()->addWorld( pWorld );

			// forward world onto our clients now
			for(int i=0;i<clientCount();i++)
				if ( client(i) != nClientId && isValid( client(i) ) )
					send( client(i), WorldClient::CONTEXT_ADD_WORLD ) << world;
		}

		pUncasted = NULL;
	}
}

void WorldServer::deleteWorld( dword nClientId, const WidgetKey & nWorld )
{
	WorldContext::wRef pWorld( nWorld );
	if ( pWorld.valid() )
	{
		worldContext()->deleteWorld( pWorld );
		for(int i=0;i<clientCount();i++)
			if ( client(i) != nClientId && isValid( client(i) ) )
				send( client(i), WorldClient::CONTEXT_DEL_WORLD ) << nWorld;

		pWorld = NULL;
	}
}

void WorldServer::addZone( dword nClientId, WidgetWrap & zone )
{
	Widget::Ref pUncasted = zone.unwrap();
	if ( pUncasted.valid() )
	{
		NodeZone * pZone = WidgetCast<NodeZone>( pUncasted.pointer() );
		if ( pZone != NULL )
		{
			worldContext()->addZone( pZone );

			// forward new zone onto our clients now
			for(int i=0;i<clientCount();i++)
				if ( client(i) != nClientId && isValid( client(i) ) )
					send( client(i), WorldClient::CONTEXT_ADD_ZONE ) << zone;
		}

		pUncasted = NULL;
	}
}

void WorldServer::deleteZone( dword nClientId, const WidgetKey & nZone )
{
	NodeZone::wRef pZone = nZone;
	if ( pZone.valid() )
	{
		worldContext()->deleteZone( pZone );
		for(int i=0;i<clientCount();i++)
			if ( client(i) != nClientId && isValid( client(i) ) )
				send( client(i), WorldClient::CONTEXT_DEL_ZONE ) << nZone;
		pZone = NULL;
	}
}

//----------------------------------------------------------------------------
//EOF
