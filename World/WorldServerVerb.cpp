/*
	WorldServerVerb.cpp
	(c)2003 Palestar Inc, Richard Lyle
*/

#include "WorldServer.h"

//----------------------------------------------------------------------------

// This routine is called when a verb is received from a client...
bool WorldServer::addVerb( ClientContext & context, const WidgetKey & nTarget, const WidgetWrap & verb, dword nUserId )
{
	// get the sender ClientContext
	if ( context.nUserId == 0 )
	{
		LOG_WARNING( "WorldServer",CharString().format("Client %u sent verb without login!", context.nClientId ) );
		return false;										// verb not from a valid client..
	}
	if ( nUserId == 0 )
		nUserId = context.nUserId;						// no userId provided, use the userId of the client

	Noun::wRef pTarget = nTarget;
	if (! pTarget.valid() || pTarget->context() != worldContext() )		// added check for same WorldContext as some nouns might not get deleted on a map switch
		return false;													// FIX: actually we need to make sure everything gets deleted instead

	// get the targets zone
	NodeZone * pZone = pTarget->zone();
	if ( pZone == NULL )
		return false;												// target noun not attached to a zone!

	// unwrap the verb
	Widget::Ref pUncasted = verb.unwrap();
	// cast the verb
	Verb * pVerb = WidgetCast<Verb>( pUncasted.pointer() );
	if ( pVerb == NULL )
	{
		LOG_ERROR( "WorldServer",CharString().format( "Failed to unwrap verb from client %u", context.nClientId ) );
		return false;												// failed to unwrap the verb
	}
	if ( !pVerb->client() )
	{
		// reduce log spam..
		static dword nLastClientId = 0;
		if (nLastClientId != context.nClientId )
		{
			LOG_ERROR( "WorldServer",CharString().format( "Client %u sent non-client verb %s", 
				context.nClientId, pVerb->factory()->className() ) );
			nLastClientId = context.nClientId;
		}
		return false;												// invalid verb to receive from a client
	}

	pVerb->setClientId( context.nClientId );
	pVerb->setUserId( nUserId );

	// attach the verb to the noun
	if (! pTarget->attachVerb( pVerb ) )
	{
		//LOG_ERROR( "WorldServer",CharString().format( "Failed to attach verb %s from client %u",
		//	pVerb->factory()->className(), context.nClientId ) );
		return false;
	}

	return true;
}

// This routine is called after a verb is attached to a noun, it will forward the verb onto the necessary servers/clients
bool WorldServer::routeVerb( Verb * pVerb )
{
	Noun * pTarget = pVerb->target();
	if (! pTarget )
		return false;		// invalid target for the verb!
	NodeZone * pZone = pTarget->zone();
	if (! pZone )
		return false;		// target is not attached to a zone

	// Route the given verb object to the correct clients based on the verb scope...
	switch( pVerb->scope() )
	{
	case Verb::SERVER:
		break;
	case Verb::GLOBAL:	
		{
			// send verb to all other clients and servers!
			for(int i=0;i<clientCount();i++)
				if ( client(i) != pVerb->clientId() && isValid( client(i) ) )
					sendVerb( client(i), pVerb );
		}
		break;
	case Verb::LOCAL:		// send verb to clients local to target
		{
			// get the root object
			Noun * pRoot = pTarget->zoneParent();
			if ( pRoot == NULL )
				break;

			// send verb onto all local users
			NounClientHash::iterator iClients = m_NounClientHash.find( pRoot->key() );
			if ( iClients == m_NounClientHash.end() )
				break;		// no local clients for this noun

			if (! sendVerb( iClients->second, pVerb ) )
				LOG_ERROR( "WorldServer","WorldServer::routeVerb() - sendVerb failed!" );
		}
		break;
	case Verb::NOT_LOCAL:			// send verb to clients not local to target
		{
			// get the root object
			Noun * pRoot = pTarget->zoneParent();
			if ( pRoot == NULL )
				break;

			// send verb onto all local users
			NounClientHash::iterator iClients = m_NounClientHash.find( pRoot->key() );
			if ( iClients == m_NounClientHash.end() )
				break;		// no local clients for this noun

			ClientArray & clients = iClients->second;
			for(int i=0;i<clientCount();i++)
			{
				dword nSendId = client(i);
				if ( nSendId != pVerb->clientId() && isValid( nSendId ) 
					&& clients.find( nSendId ) == clients.end() )
				{
					sendVerb( nSendId, pVerb );
				}
			}
		}
		break;
	case Verb::TEAM:
		{
			TeamClientHash::iterator iClients = m_TeamClientHash.find( pTarget->teamId() );
			if ( iClients == m_TeamClientHash.end() )
				break;
			if (! sendVerb( iClients->second, pVerb ) )
				LOG_ERROR( "WorldServer","WorldServer::routeVerb() - sendVerb failed!" );
		}
		break;
	case Verb::FACTION:
		{
			FactionClientHash::iterator iClients = m_FactionClientHash.find( pTarget->factionId() );
			if ( iClients == m_FactionClientHash.end() )
				break;
			if (! sendVerb( iClients->second, pVerb ) )
				LOG_ERROR( "WorldServer","WorldServer::routeVerb() - sendVerb failed!" );
		}
		break;
	case Verb::PRIVATE:
		{
			UserClientHash::iterator iClient = m_UserClientHash.find( pTarget->userId() );
			if ( iClient == m_UserClientHash.end() )
				break;
			dword nSendId = iClient->second;
			if ( nSendId == pVerb->clientId() )
				break;
			if (! sendVerb( nSendId, pVerb ) )
				LOG_ERROR( "WorldServer", "WorldServer::routeVerb() - sendVerb failed!" );
		}
		break;
	}

	return true;
}

OutStream WorldServer::sendClient( dword nClientId, u8 nMessage, bool bUDP, bool bValidUser )
{
	ClientContextHash::iterator iContext = m_ClientContext.find( nClientId );
	if ( iContext != m_ClientContext.end() )
	{
		ClientContext & context = iContext->second;
		
		if (! bValidUser || context.nUserId != 0 )
		{
			if ( bUDP && m_Context.bEnableUDP && context.bConnectedUDP )
				return sendUDP( nClientId, nMessage );
			else
				return send( nClientId, nMessage );
		}
	}

	// return NULL OutStream()...
	return OutStream( NULL );
}

bool WorldServer::sendVerb( const ClientArray & clients, dword nSenderId, const WidgetKey & nNoun, 
						   const WidgetWrap & verb, bool bUDP, bool bCheckNoun )
{
	for( ClientArray::const_iterator iClient = clients.begin(); iClient != clients.end(); ++iClient )
	{
		dword nClientId = *iClient;
		if ( nClientId == nSenderId )
			continue;
		sendVerb( *iClient, nNoun, verb, bUDP, bCheckNoun );
	}

	return true;
}

bool WorldServer::sendVerb( const ClientArray & clients, Verb * pVerb )
{
	if ( pVerb == NULL )
		return false;
	Noun * pTarget = pVerb->target();
	if ( pTarget == NULL )
		return false;
	if ( clients.size() == 0 )
		return true;
	return sendVerb( clients, pVerb->clientId(), pTarget->key(), 
		WidgetWrap( pVerb, FF_TRANSMIT, m_pDictionary ), pVerb->useUDP(), pVerb->checkNoun() );
}

bool WorldServer::sendVerb( dword nClientId, const WidgetKey & nNoun, const WidgetWrap & verb, bool bUDP, bool bCheckNoun )
{
	if ( bCheckNoun )
	{
		// don't send verb's for objects the client doesn't have..
		ClientContextHash::iterator iClient = m_ClientContext.find( nClientId );
		if ( iClient == m_ClientContext.end() )
			return false;
		if ( iClient->second.updateHash.find( nNoun ) == iClient->second.updateHash.end() )
			return false;
	}

	try {
		sendClient( nClientId, WorldClient::CONTEXT_ADD_VERB, bUDP, false ) << nNoun << verb;
	}
	catch( File::FileError )
	{}

	return true;
}

bool WorldServer::sendVerb( dword nClientId, Verb * pVerb )
{
	if ( pVerb == NULL )
		return false;
	Noun * pTarget = pVerb->target();
	if ( pTarget == NULL )
		return false;

	return sendVerb( nClientId, pTarget->key(), WidgetWrap( pVerb, FF_TRANSMIT, m_pDictionary ), 
		pVerb->useUDP(), pVerb->checkNoun() );
}


//----------------------------------------------------------------------------
//EOF
