/*
	WorldServerContext.cpp
	(c)2000 Palestar Inc, Richard Lyle
*/


#include "Debug/Assert.h"
#include "Debug/Trace.h"
#include "Resource/Resource.h"
#include "WorldServer.h"

//----------------------------------------------------------------------------

bool WorldServer::contextInitialize( dword nClientId, const WidgetWrap & worldContext )
{
	Widget::Ref pUncasted = worldContext.unwrap();
	if (! pUncasted.valid() )
		return false;

	WorldContext * pUniverse = WidgetCast<WorldContext>( pUncasted.pointer() );
	if ( pUniverse == NULL )
		return false;

	// set the new worldContext
	m_pWorldContext = pUniverse;
	m_pWorldContext->setUser( this );

	// prepare the world
	if (! prepWorld() )
	{
		LOG_ERROR( "WorldServer", "Failed to prepare the world!" );
		return false;
	}

	// start the timer
	m_pWorldContext->start();
	// TODO: Add logic to unlock zones with no objects in them..

	// forward context onto all other clients
	for(int i=0;i<clientCount();i++)
		if ( client(i) != nClientId && isValid( client(i) ) )
			send( client(i), WorldClient::CONTEXT_INIT ) << worldContext;

	// clear the end game flag
	m_bEndGame = false;
	// de-reference the object before we unlock
	pUncasted = NULL;

	return true;
}

void WorldServer::contextEnd( dword nClientId, const char * pEndMessage )
{
	// set the end game flag
	m_bEndGame = true;
	// stop the worldContext
	m_pWorldContext->stop();

	// release hash tables
	m_NounClientHash.clear();
	m_FactionClientHash.clear();
	m_TeamClientHash.clear();

	// reset the clients
	for(int i=0;i<clientCount();++i)
	{
		ClientContext & context = m_ClientContext[ client(i) ];
		context.pSelf = NULL;
		context.pTarget = NULL;
		context.pFocus = NULL;
		context.nTeamId = 0;
		context.nFactionId = 0;
		context.localNouns.clear();
		context.updateHash.clear();
	}

	// forward end message onto all other clients
	for(int i=0;i<clientCount();i++)
		if ( client(i) != nClientId && isValid( client(i) ) )
			send( client(i), WorldClient::CONTEXT_END ) << CharString( pEndMessage );
}


void WorldServer::contextName( dword nClientId, const char * pName )
{
	worldContext()->setName( pName );

	// forward the change onto all clients
	for(int i=0;i<clientCount();i++)
		if ( client(i) != nClientId && isValid( client(i) ) )
			send( client(i),WorldClient::CONTEXT_SET_NAME ) << CharString( pName );
}

void WorldServer::contextDescription( dword nClientId, const char * pDesc )
{
	worldContext()->setDescription( pDesc );

	// forward the change onto all clients
	for(int i=0;i<clientCount();i++)
		if ( client(i) != nClientId && isValid( client(i) ) )
			send( client(i),WorldClient::CONTEXT_SET_DESC ) << CharString( pDesc );
}

void WorldServer::contextType( dword nClientId, int nType )
{
	worldContext()->setType( nType );

	// forward the change onto all clients
	for(int i=0;i<clientCount();i++)
		if ( client(i) != nClientId && isValid( client(i) ) )
			send( client(i),WorldClient::CONTEXT_SET_TYPE ) << nType;
}

void WorldServer::contextScript( dword nClientId, const char * pScript )
{
	worldContext()->setScript( pScript );

	// forward the change onto all clients
	for(int i=0;i<clientCount();i++)
		if ( client(i) != nClientId && isValid( client(i) ) )
			send( client(i),WorldClient::CONTEXT_SET_SCRIPT ) << CharString( pScript );
}

//----------------------------------------------------------------------------

bool WorldServer::isServer() const
{
	return true;
}

bool WorldServer::isSleeping() const
{
	return m_bSleeping;
}

dword WorldServer::serverRole() const
{
	return m_Context.nServerRole;
}

int WorldServer::maxPlayers() const
{
	return m_Context.maxPlayers;
}

int WorldServer::playerCount() const
{
	return m_nPlayers;
}

int WorldServer::spawnedScore( int a_nFactionId )
{
	return 0;
}

int WorldServer::maxPlayerScoreAI()
{
	return 0;
}

int WorldServer::baseTeamAi() const
{
	return 0;
}

void WorldServer::onChat( Noun * pNoun, const char * pMessage )
{
	CharString sStatus;
	if ( pNoun != NULL )
		sStatus.format("> %s : %s", pNoun->name(), pMessage );
	else
		sStatus.format("> %s", pMessage );
	sStatus.replace( "%", "%%" );		// escape any % symbols..

	LOG_STATUS( "WorldServer", sStatus );
}

bool WorldServer::canAttachVerb( Noun * pTarget, Verb * pVerb )
{
	return true;
}

void WorldServer::onAttachVerb( Verb * pVerb )
{
	// route the verb to the correct clients/players and servers
	routeVerb( pVerb );
}

void WorldServer::onAddTeam( const Team & team )
{}

void WorldServer::onDeleteTeam( int nTeamId )
{}

void WorldServer::onAddCondition( const Condition & condition )
{}

void WorldServer::onDeleteCondition( int conditionId )
{}

void WorldServer::onAddWorld( WorldContext * pWorld )
{}

void WorldServer::onDeleteWorld( WorldContext * pWorld )
{}

void WorldServer::onAddZone( NodeZone * pZone )
{}

void WorldServer::onDeleteZone( NodeZone * pZone )
{}

void WorldServer::onAttachNoun( Noun * pNoun )
{
	ASSERT( pNoun != NULL );

	//TRACE( CharString().format("WorldServer::onAttachNoun, name = %s, key = %s", pNoun->name(), pNoun->key().string()) );

	// if noun is not syncronized, syncronize it with the other servers/clients
	if (! pNoun->syncronized() )
	{
		// set the noun as syncronized now
		pNoun->setSyncronized( true );

		// route the noun to other servers and clients, do this last as this may actually
		// detach the object and send it off to another server
		routeAddNoun( 0, pNoun );
	}
}

void WorldServer::onUpdateNoun( Noun * pNoun )
{
	ASSERT( pNoun != NULL );
	ASSERT( pNoun->parent() != NULL );

	// only allow the server who has the zone locked for the noun to send 
	routeAddNoun( 0, pNoun );
}


void WorldServer::onDetachNoun( Noun * pNoun )
{
	ASSERT( pNoun != NULL );
	ASSERT( pNoun->parent() != NULL );

	//TRACE( CharString().format("WorldServer::onDetachNoun, name = %s, key = %s", pNoun->name(), pNoun->key().string()) );

	// check if the noun is syncronized and currently in a zone locked by this server
	if ( pNoun->syncronized() )		// removed inLockedZone() check because any server should be able to detach a noun
	{
		// set the noun as syncronized now
		pNoun->setSyncronized( false );
		// route the delete command out to other servers and clients
		routeDeleteNoun( 0, pNoun );
	}

	// clean up our hash tables at this point
	releaseNoun( pNoun );
}

void WorldServer::onTransferNoun( Noun * pNoun, NodeZone * pNewZone, NodeZone * pOldZone )
{
	// NOT sure why I was doing this, but commented it out since it looks like this should really never happen on the server (12-26-2008)
	//if ( pOldZone->locked() && !pNewZone->locked())
	//	routeAddNoun( 0, pNoun );
}

void WorldServer::onWorldTransform( const Vector3 & transform )
{}

void WorldServer::onCondition( int nConditionId )
{}

void WorldServer::onServerTransfer( Noun * a_pWho, const char * a_pAddress, 
							int a_nPort, const char * a_pSpawnPoint )
{
	dword nClientId = getClientId( a_pWho->userId() );
	if ( nClientId != 0 )
	{
		WorldClient::TransferRequest req;
		req.m_sAddress = a_pAddress;
		req.m_nPort = a_nPort;
		req.m_nSelf = a_pWho->key();
		req.m_sSpawnPoint = a_pSpawnPoint;

		if ( !transferClient( nClientId, req ) )
			LOG_ERROR( "WorldServer", CharString().format("ERROR: Failed to transfer client %u!", nClientId ) );
	}
}

//----------------------------------------------------------------------------
//EOF
