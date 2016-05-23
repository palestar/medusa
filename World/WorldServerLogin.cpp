/*
	WorldServerLogin.cpp
	(c)2005 Palestar Inc, Richard Lyle
*/


#include "Standard/Time.h"
#include "WorldServer.h"

//----------------------------------------------------------------------------

Constant		DEFAULT_FOCUS_AREA( "DEFAULT_FOCUS_AREA", 5000.0f );

//----------------------------------------------------------------------------

// default factory implementation for the Profile object..
bool WorldServer::createProfile( ClientContext & context )
{
	context.pProfile = new Profile();
	context.pProfile->load( context.MetaProfile );

	return true;
}

bool WorldServer::createProxy( ClientContext & context, const WidgetKey & a_nObjectId, int a_nFactionId )
{
	if (! context.bProxy )
		return true;		// not a proxy client

	context.nFactionId = a_nFactionId;

	if ( a_nObjectId != NULL_WIDGET )
	{
		context.pSelf = new Noun();
		context.pSelf->setKey( a_nObjectId );
	}
	else
	{
		context.pSelf = NULL;
	}

	return true;
}

void WorldServer::loginClients()
{
	m_LoginQueue.reset();
	while( m_LoginQueue.valid() )
	{
		LoginJob & job = *m_LoginQueue;
		if ( job.bDone )
		{
			// we now have login data for the client, finish the login process for the client..
			send( job.nClientId, WorldClient::CLIENT_LOGIN ) << loginClient( job );
			m_LoginQueue.pop();
		}
		else
		{
			// next login entry...
			m_LoginQueue.next();
		}
	}
}

bool WorldServer::loginClient( const LoginJob & job )
{
	if (! job.bDone )
		return false;			// job hasn't be flagged as done yet..  should never happen.
	if ( job.profile.userId == 0 )
		return false;			// didn't get a user ID... 
	if (! isClient( job.nClientId ) )
		return false;			// client is no longer connected!

	// initialize the client context
	ClientContext & context = m_ClientContext[ job.nClientId ];
	context.MetaProfile = job.profile;
	context.bProxy = job.bProxy;
	context.bServer = job.bServer;
	// replace any < with [ and > with ] so they don't screw with in-game display ..
	context.MetaProfile.name.replace( '<', '[' );
	context.MetaProfile.name.replace( '>', ']' );
	context.nUserId = context.MetaProfile.userId;
	context.bModerate = false;
	context.nLoginTime = Time::seconds();
	context.nLogoffTime = 0;
	context.nQueuePosition = -1;

	// send the server status
	sendServerStatus( job.nClientId );
	// send the context to the new client
	sendContext( context );

	// if a server connection, then early out since we don't care if the same user account is logged in multiple
	// times for servers. 
	if ( context.bServer )
	{
		if ( (context.MetaProfile.flags & MetaClient::SERVER) == 0 )
		{
			LOG_ERROR( "WorldServer", "Client %u does not have the server flag.", job.nClientId );
			return false;
		}

		LOG_STATUS( "WorldServer", "Client %u server login as '%s' @%u", 
			job.nClientId, context.MetaProfile.name.cstr(), context.nUserId );

		onLogin( context );
		return true;
	}

	bool bNewLogin = true;

	// check if this player is logged in through another client, disconnect the other client if one is found...
	UserClientHash::iterator iClient = m_UserClientHash.find( context.nUserId );
	if ( iClient != m_UserClientHash.end() )
	{
		dword nOldClientId = iClient->second;
		if ( nOldClientId == job.nClientId )
		{
			LOG_STATUS( "WorldServer", "Client %u already logged in as user %u", job.nClientId, context.nUserId );
			return true;
		}

		Server::removeClient( nOldClientId );		// close the old socket

		ClientContext & OldContext = m_ClientContext[ nOldClientId ];
		if (! OldContext.bProxy )
		{
			LOG_STATUS( "WorldServer", "Client %u is already logged in, disconnecting client %u", job.nClientId, nOldClientId );

			context.pProfile = OldContext.pProfile;
			context.pStorage = OldContext.pStorage;
			context.nFactionId = OldContext.nFactionId;
			context.nTeamId = OldContext.nTeamId;
			context.pFocus = OldContext.pFocus;
			context.pTarget = OldContext.pTarget;
			context.pSelf = OldContext.pSelf;		
			context.nLoginTime = OldContext.nLoginTime;
			context.nQueuePosition = OldContext.nQueuePosition;
			context.fFocusArea = OldContext.fFocusArea;
			OldContext.nQueuePosition = -1;		// IMPORTANT that this happens, otherwise deleteClient() might reduce the player or player in queue counts..

			// clean up old ClientContext now... close storage if this is a proxy connection.
			if (! deleteClient( OldContext, false ) )
				LOG_ERROR( "WorldServer", "ERROR: Failed to delete the clients old context!" );

			// send the player his profile now
			send( job.nClientId, WorldClient::CLIENT_SET_PROFILE ) << context.pProfile;
			send( job.nClientId, WorldClient::CLIENT_SET_FOCUS_AREA ) << context.fFocusArea;

			bNewLogin = false;
		}
		else
		{
			// old connection was a proxy connection, clean it up but then treat this as a new connection..
			// note that we close the storage to force the Proxy object to be detached from the world.
			if (! deleteClient( OldContext, true ) )
				LOG_ERROR( "WorldServer", "ERROR: Failed to delete the proxy clients old context!" );
		}
	}

	if ( bNewLogin )
	{
		int nTeamId = 0;

		// check if team is create for the players clan
		if ( m_Context.bClan && context.MetaProfile.clanId != 0 )
		{
			// did we get the clan information, clan.clanId will be 0 if not!
			if ( job.clan.clanId == context.MetaProfile.clanId )
			{
				nTeamId = worldContext()->teamClan( job.clan.clanId, job.clan.faction );
				if ( nTeamId == 0 )
				{
					nTeamId = addTeam( job.clan );
					if ( nTeamId == 0 )
					{
						LOG_ERROR( "WorldServer", CharString().format("ERROR: Failed to create team for clan %u, factionId = %u!", 
							job.clan.clanId, job.clan.faction ) );
					}
				}
			}
		}

		// if client is a moderator, don't force them onto a team.. allow them to always pick a team.
		if ( nTeamId != 0 && isModerator( job.nClientId ) )
			nTeamId = 0;

		if (! createProfile( context ) || !context.pProfile )
		{
			LOG_ERROR( "WorldServer", "ERROR: Failed to create profile!" );
			return false;
		}
		context.nTeamId = nTeamId;
		context.nFactionId = worldContext()->teamFactionId( context.nTeamId );
		context.fFocusArea = DEFAULT_FOCUS_AREA;

		// send the player his profile now
		send( job.nClientId, WorldClient::CLIENT_SET_PROFILE ) << context.pProfile;
		send( job.nClientId, WorldClient::CLIENT_SET_FOCUS_AREA ) << context.fFocusArea;
	}

	// if this is a proxy connection, then create a proxy object so we have something to validate
	// against for the local player..
	if ( context.bProxy && !createProxy( context, job.nProxyId, job.nProxyFaction ) )
	{
		LOG_ERROR( "WorldServer", "ERROR: Failed to create proxy!" );
		return false;
	}

	// send selected team, focus, target, and ship if any to the client
	if ( context.nTeamId != 0 )
		send( job.nClientId, WorldClient::CLIENT_SET_TEAM ) << context.nTeamId;
	if ( context.pFocus.valid() )
		send( job.nClientId, WorldClient::CLIENT_SET_FOCUS ) << Noun::wRef( context.pFocus );
	if ( context.pTarget.valid() )
		send( job.nClientId, WorldClient::CLIENT_SET_TARGET ) << Noun::wRef( context.pTarget );
	if ( context.pSelf.valid() )
		send( job.nClientId, WorldClient::CLIENT_SET_SELF ) << Noun::wRef( context.pSelf );

	// load and send storage to the player..
	if ( !sendStorage( context ) )
	{
		sendChat( job.nClientId, "SERVER: Storage locked by another server, please try again later..." );
		return false;
	}

	if (! checkClientGroup( context ) )
	{
		LOG_ERROR( "WorldServer", "ERROR: Failed to check the client group!" );
		return false;
	}

	// hash the players team & faction if already selected...
	if ( context.nTeamId != 0 )
		m_TeamClientHash[ context.nTeamId ].insert( job.nClientId );
	if ( context.nFactionId != 0 )
		m_FactionClientHash[ context.nFactionId ].insert( job.nClientId );
	// hash the user ID to the client ID
	m_UserClientHash[ context.nUserId ] = job.nClientId;

	// is this is a new connection of an actual player..
	if ( bNewLogin && !context.bProxy )
	{
		// if this is NOT a server transfer, increment the Games PLayed..
		if (! job.bTransfer )
			addPlayerProfile( context.nUserId, "Games Played", 1.0f );

		// since this is a new login, check if the player must wait in the queue or not..
		if ( m_nPlayers >= m_Context.maxPlayers && clientCount() > m_Context.maxPlayers )
		{
			m_nPlayersInQueue += 1;
			context.nQueuePosition = m_nPlayersInQueue;

			LOG_STATUS( "WorldServer", CharString().format( "Client %u entering queue, %d players in queue...", 
				job.nClientId, m_nPlayersInQueue ) );

			// send the client their queue position...
			send( job.nClientId, WorldClient::CLIENT_RECV_QUEUEPOS ) 
				<< context.nQueuePosition << m_nPlayersInQueue;
		}
		else
		{
			m_nPlayers += 1; 
			context.nQueuePosition = 0;
		}
	}

	LOG_STATUS( "WorldServer", "Client %u %s as '%s' @%u", 
		job.nClientId, context.bProxy ? "proxy login" : "login", context.MetaProfile.name.cstr(), context.nUserId );
	LOG_STATUS( "WorldServer", "[STAT] %u Clients, %d Players, %d Players Queued", clientCount(), m_nPlayers, m_nPlayersInQueue );

	onLogin( context );

	return true;
}

void WorldServer::onLogin( ClientContext & client )
{}

bool WorldServer::logoffClient( dword nClientId )
{
	if ( nClientId == 0 )
		return false;
	ClientContextHash::iterator iContext = m_ClientContext.find( nClientId );
	if ( iContext == m_ClientContext.end() )
		return false;

	ClientContext & context = iContext->second;
	if ( context.nUserId != 0 )
		LOG_STATUS( "WorldServer", "Client %u logoff '%s' @%u", nClientId, context.MetaProfile.name.cstr(), context.nUserId );
	else
		LOG_STATUS( "WorldServer", "Client %u logoff - no login", nClientId );

	// Set the logoff time, this flags this client connection to be deleted by deleteClients()... 
	// If the user login's again before the logoff timer expires, then their old client data is deleted
	// immediately...
	if ( context.nLogoffTime == 0 )
		context.nLogoffTime = Time::seconds();

	// clean up our hash tables now, so we don't try to send verbs & nouns to this removed client..
	NounArray & nouns = context.localNouns;
	for(NounArray::iterator iNoun = nouns.begin(); iNoun != nouns.end(); ++iNoun )
		m_NounClientHash[ (*iNoun)->key() ].erase( nClientId );
	nouns.clear();

	if ( context.nFactionId != 0 )
		m_FactionClientHash[ context.nFactionId ].erase( nClientId );
	if ( context.nTeamId != 0 )
		m_TeamClientHash[ context.nTeamId ].erase( nClientId );
	if ( context.bModerate )
		m_Moderators.remove( nClientId );

	// IMPORTANT part... we must push this client into the removal list, this will remove their NOun in the world
	// after the disconnect time has expired. This is done to prevent people from logging out to cheat death, etc..
	if (! m_DeleteClientList.find( nClientId ).valid() )
		m_DeleteClientList.insert( nClientId );

	return true;
}

bool WorldServer::deleteClients()
{
	List< dword >::Iterator iClient = m_DeleteClientList.head();
	while( iClient.valid() )
	{
		ClientContextHash::iterator iContext = m_ClientContext.find( *iClient );
		if ( iContext != m_ClientContext.end() )
		{
			ClientContext & context = iContext->second;

			// moderators get removed immediately, so they can switch sides quickly... no time delay for them.
			bool isModerator = (context.MetaProfile.flags & MetaClient::MODERATOR) != 0;
			
			if ( context.nUserId == 0 
				|| context.bProxy
				|| (context.nLogoffTime != 0 && (Time::seconds() - context.nLogoffTime) > m_Context.nDisconnectTime) 
				|| isModerator )
			{
				// check for players that should be deleted
				if (! deleteClient( context, true ) )
					LOG_ERROR( "WorldServer", CharString().format("ERROR: Failed to delete client %u...", context.nClientId) );

				// client was already deleted, this could happen if the player login's again before their old ClientContext is purged..
				m_DeleteClientList.remove( iClient );
				continue;
			}
		}
		else
		{
			// no context found, remove the client from the list then..
			m_DeleteClientList.remove( iClient );
			continue;
		}

		iClient.next();
	}

	return true;
}

bool WorldServer::deleteClient( ClientContext & client, bool bCloseStorage )
{
	dword nClientId = client.nClientId;
	if (! logoffClient( nClientId ) )
		return false;			// force the client to be logged off... this may happen if the same users connects with a 2nd machine on the same account..

	// get the client's group if any..
	Group::Ref pGroup;
	if ( client.pStorage )
		pGroup = client.pStorage->group();

	dword nUserId = client.nUserId;
	if ( nUserId != 0 )
	{
		LOG_STATUS( "WorldServer", "Deleting client %u - %s @%u...", nClientId, client.MetaProfile.name.cstr(), client.nUserId );

		onDeleteClient( client );
	
		if ( bCloseStorage )
		{
			// detach the players ship from the worldContext, the ship will get stored by onDetachNoun() callback
			if ( client.pSelf.valid() )
				client.pSelf->detachSelf();
			// lastly, save the users storage onto disk
			if (! closeStorage( nUserId ) )
				LOG_ERROR( "WorldServer", CharString().format("ERROR: Failed to close user %u storage!", nUserId ) );
		}

		// make sure the UserClientHash maps our userId to this ClientId before we remove the mapping..
		UserClientHash::iterator iClient = m_UserClientHash.find( nUserId );
		if ( iClient != m_UserClientHash.end() && iClient->second == client.nClientId )
			m_UserClientHash.erase( iClient );
	}
	else
	{
		LOG_STATUS( "WorldServer", CharString().format( "Deleting client %u - no login", nClientId ) );
	}

	m_ClientContext.erase( nClientId );

	// remove the group from the map if this was the last connected client.
	if ( bCloseStorage && pGroup.valid() && !groupConnected( pGroup ) )
		m_GroupMap.erase( pGroup->key() );

	// All data for the client should now be gone... 
	return true;
}

bool WorldServer::transferClient( dword nClientId, const WorldClient::TransferRequest & a_Req )
{
	if (! logoffClient( nClientId ) )
		return false;			// force the client to be logged off... this may happen if the same users connects with a 2nd machine on the same account..

	ClientContextHash::iterator iContext = m_ClientContext.find( nClientId );
	if ( iContext == m_ClientContext.end() )
	{
		LOG_ERROR( "WorldServer", "ERROR: Failed to find client context!" );
		return false;
	}

	ClientContext & client = iContext->second;
	if ( client.nUserId != 0 )
	{
		LOG_STATUS( "WorldServer", "Transferring client %u - %s @%u to %s:%d...", 
			nClientId, client.MetaProfile.name.cstr(), client.nUserId, a_Req.m_sAddress.cstr(), a_Req.m_nPort );
		client.bServerTransfer = true;

		// save the players ship to storage now...
		if (! onTransferClient( client, a_Req ) )
		{
			LOG_ERROR( "WorldServer", "ERROR: Failed to transfer client!" );
			return false;
		}

		// pre-delete client steps...
		onDeleteClient( client );

		// close out the storage now, this prevents this server trying to save the ship again (with the wrong address & port) 
		// below when it gets detached from the world.
		if (! closeStorage( client.nUserId ) )
			LOG_ERROR( "WorldServer", CharString().format("ERROR: Failed to close storage for user %u!", client.nUserId) );

		// send transfer message to client... 
		send( nClientId, WorldClient::CLIENT_SERVER_TRANSFER ) << a_Req;

		// detach the player's object AFTER we have sent the transfer message...
		if ( client.pSelf.valid() )
			client.pSelf->setDetach();

		m_UserClientHash.erase( client.nUserId );
	}
	else
	{
		LOG_STATUS( "WorldServer", "Transferring client %u to %s:%d... no login", nClientId, a_Req.m_sAddress.cstr(), a_Req.m_nPort );
		send( nClientId, WorldClient::CLIENT_SERVER_TRANSFER ) << a_Req;
	}

	m_ClientContext.erase( iContext );
	m_DeleteClientList.remove( nClientId );

	// All data for the client should now be gone... 
	return true;
}

void WorldServer::onDeleteClient( ClientContext & client )
{
	// if player was NOT in the queue, then update the player count...
	if ( client.nQueuePosition == 0 )
	{
		if (! client.bProxy )
			m_nPlayers -= 1;

		// update the queue, let the next player in the queue into the game..
		if ( m_nPlayers < m_Context.maxPlayers )
		{
			for(int i=0;i<clientCount();++i)
			{
				dword nClientId = Server::client(i);
				if ( client.nClientId == nClientId )
					continue;	
				ClientContextHash::iterator iContext = m_ClientContext.find( nClientId );
				if ( iContext == m_ClientContext.end() )
					continue;
				ClientContext & context = iContext->second;

				// if player was above this one in the queue, adjust their position...
				if ( context.nQueuePosition > 0 )
				{
					context.nQueuePosition -= 1;
					if ( context.nQueuePosition == 0 )
					{
						m_nPlayersInQueue -= 1;
						m_nPlayers += 1;

						LOG_STATUS( "WorldServer", CharString().format( "Client %u leaving queue, %d players in queue...", 
							nClientId, m_nPlayersInQueue ) );
					}

					// send an updated queue position to the player..
					send( nClientId, WorldClient::CLIENT_RECV_QUEUEPOS ) 
						<< context.nQueuePosition << m_nPlayersInQueue;
				}
			}
		}

		client.nQueuePosition = -1;
	}
	else if ( client.nQueuePosition > 0 )
	{
		m_nPlayersInQueue -= 1;

		for(int i=0;i<clientCount();++i)
		{
			dword nClientId = Server::client(i);
			if ( client.nClientId == nClientId )
				continue;	
			ClientContextHash::iterator iContext = m_ClientContext.find( nClientId );
			if ( iContext == m_ClientContext.end() )
				continue;
			ClientContext & context = iContext->second;
			if ( context.nQueuePosition == 0 )
				continue;		// player is not in queue

			// if player was above this one in the queue, adjust their position...
			if ( context.nQueuePosition > client.nQueuePosition )
				context.nQueuePosition -= 1;

			// send an updated queue position to the player..
			send( nClientId, WorldClient::CLIENT_RECV_QUEUEPOS ) 
				<< context.nQueuePosition << m_nPlayersInQueue;
		}

		client.nQueuePosition = -1;
	}
}

bool WorldServer::onTransferClient( ClientContext & client, 
						const WorldClient::TransferRequest & a_Req )
{
	return true;
}

//----------------------------------------------------------------------------
//EOF
