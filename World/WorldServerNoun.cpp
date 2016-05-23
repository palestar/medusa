/*
	WorldServerNoun.cpp
	(c)2003 Palestar Inc, Richard Lyle
*/

#include "Debug/Assert.h"
#include "Debug/Profile.h"
#include "WorldServer.h"
#include "Noun.h"

//----------------------------------------------------------------------------

// called when a Noun is received by OnReceive(), this routine handles the attaching of the noun and routing of the noun
// to the other clients and servers
bool WorldServer::addNoun( dword nClientId, const WidgetKey & nParent, const WidgetKey & nNoun, const WidgetWrap & instance )
{
	// find the parent BaseNode object
	BaseNode::wRef pParent = nParent;
	if (! pParent.valid() )
		return false;

	// first, detach any noun we already have with the same key
	Noun::wRef pNoun = nNoun;
	if ( pNoun.valid() )
	{
		// set syncronized to false, so onDetachNoun() is ignored
		pNoun->setSyncronized( false );
		// let the noun detach from parent
		pNoun->detachSelf();
	}

	// unwrap the noun from it's package
	Widget::Ref pUncasted = instance.unwrap();
	if (! pUncasted.valid() )
		return false;

	pNoun = WidgetCast<Noun>( pUncasted );
	if (! pNoun )
		return false;

	// set the noun as syncronized before attaching, so onAttachNoun() is ignored
	pNoun->setSyncronized( true );
	// attach the noun to the parent
	pParent->attachNode( pNoun );
	// send the noun onto other clients
	routeAddNoun( nClientId, pNoun );

	return true;
}

// This function is called to route a noun to the correct servers and clients
bool WorldServer::routeAddNoun( dword nClientId, Noun * pNoun )
{
	if ( pNoun == NULL )
		return false;		// invalid pointer!
	BaseNode * pParent = pNoun->parent();
	if (! pParent )
		return false;		// noun has no parent!
	Noun * pRoot = pNoun->zoneParent();
	if ( pRoot == NULL )
		return false;		// noun is not attached to any zone!

	NounType eType = pNoun->nounType();
	switch( eType )
	{
	case TYPE_LOCAL_OBJECT:
		{
			// zone parent object is an object, send newly attached noun's to local clients only..
			ClientArray * pClients = getNounClients( pRoot->key() );
			if (! pClients )
				break;
			if ( pClients->size() <= 0 )
				break;

			WidgetWrap instance, update;
			for( ClientArray::iterator iClient = pClients->begin(); iClient != pClients->end(); ++iClient )
			{
				dword nSendId = *iClient;
				if (nSendId == nClientId )
					continue;
				ClientContextHash::iterator iContext = m_ClientContext.find( nSendId );
				if ( iContext == m_ClientContext.end() )
					continue;
				ClientContext & context = iContext->second;
				if ( context.nUserId == 0 )
					continue;

				UpdateHash::iterator iUpdate = context.updateHash.find( pNoun->key() );
				if ( iUpdate == context.updateHash.end() || iUpdate->second != pNoun->version() )
				{
					// first time sending this noun or the version has changed, so send the FULL object..
					if ( instance.size() == 0 )
						instance.wrap( pNoun, FF_TRANSMIT, m_pDictionary );
					sendAddNoun( nSendId, pParent->key(), pNoun->key(), instance );
					context.updateHash[ pNoun->key() ] = pNoun->version();
				}
				else
				{
					// send a smaller version of this object which only has properties flagged with PF_UPDATE.
					if ( update.size() == 0 )
						update.wrap( pNoun, FF_TRANSMIT|FF_UPDATE, m_pDictionary );
					sendUpdateNoun( nSendId, pParent->key(), pNoun->key(), update );
				}
			}
		}
		break;
	case TYPE_OBJECT:
	case TYPE_LANDMARK:
		{
			WidgetWrap instance, update;
			for(int i=0;i<clientCount();++i)
			{
				dword nSendId = client(i);
				if (nSendId == nClientId )
					continue;
				ClientContextHash::iterator iContext = m_ClientContext.find( nSendId );
				if ( iContext == m_ClientContext.end() )
					continue;
				ClientContext & context = iContext->second;
				if ( context.nUserId == 0 )
					continue;

				UpdateHash::iterator iUpdate = context.updateHash.find( pNoun->key() );
				if ( iUpdate == context.updateHash.end() || iUpdate->second != pNoun->version() )
				{
					if ( instance.size() == 0 )
						instance.wrap( pNoun, FF_TRANSMIT, m_pDictionary );
					sendAddNoun( nSendId, pParent->key(), pNoun->key(), instance );
					context.updateHash[ pNoun->key() ] = pNoun->version();
				}
				else
				{
					if ( update.size() == 0 )
						update.wrap( pNoun, FF_TRANSMIT|FF_UPDATE, m_pDictionary );
					sendUpdateNoun( nSendId, pParent->key(), pNoun->key(), update );
				}
			}
		}
		break;
	}

	return true;
}

bool WorldServer::deleteNoun( dword nClientId, const WidgetKey & nNoun, dword nLastTick )
{
	Noun::wRef pNoun = nNoun;
	if (! pNoun.valid() )
		return false;

	routeDeleteNoun( nClientId, pNoun );

	pNoun->setTick( nLastTick, false );
	pNoun->setSyncronized( false );
	pNoun->detachVerbs();
	pNoun->detachSelf();
	pNoun = NULL;

	return true;
}

bool WorldServer::routeDeleteNoun( dword nClientId, Noun * pNoun )
{
	if (! pNoun )
		return false;
	Noun * pRoot = pNoun->zoneParent();
	if ( pRoot == NULL )
		return false;		// noun is not attached to any zone!

	NounType eType = pNoun->nounType();
	switch( eType )
	{
	case TYPE_LOCAL_OBJECT:
		{
			ClientArray * pClients = getNounClients( pRoot->key() );
			if (! pClients )
				break;
			if ( pClients->size() <= 0 )
				break;

			for( ClientArray::iterator iClient = pClients->begin(); iClient != pClients->end(); ++iClient )
			{
				dword nSendId = *iClient;
				if (nSendId == nClientId )
					continue;
				ClientContextHash::iterator iContext = m_ClientContext.find( nSendId );
				if ( iContext == m_ClientContext.end() )
					continue;
				ClientContext & context = iContext->second;
				if ( context.nUserId == 0 )
					continue;

				context.updateHash.erase( pNoun->key() );
				sendDeleteNoun( nSendId, pNoun->key(), pNoun->tick() );
			}
		}
		break;
	case TYPE_OBJECT:
	case TYPE_LANDMARK:
		{
			// delete the noun from all clients
			for(int i=0;i<clientCount();i++)
			{
				dword nSendId = client(i);
				if (nSendId == nClientId )
					continue;
				ClientContextHash::iterator iContext = m_ClientContext.find( nSendId );
				if ( iContext == m_ClientContext.end() )
					continue;
				ClientContext & context = iContext->second;
				if ( context.nUserId == 0 )
					continue;
				
				context.updateHash.erase( pNoun->key() );
				sendDeleteNoun( nSendId, pNoun->key(), pNoun->tick() );
			}
		}
		break;
	}

	return true;
}

bool WorldServer::sendAddNoun( dword nClientId, const WidgetKey & nParent, const WidgetKey & nNoun, const WidgetWrap & instance )
{
	// push the noun to into the data queue for this client
	send( nClientId, WorldClient::CONTEXT_ADD_NOUN ) << nParent << nNoun << instance;
	return true;
}

bool WorldServer::sendUpdateNoun( dword nClientId, const WidgetKey & nParent, const WidgetKey & nNoun, const WidgetWrap & update )
{
	// push the noun to into the data queue for this client
	send( nClientId, WorldClient::CONTEXT_UPDATE_NOUN ) << nParent << nNoun << update;
	return true;
}

bool WorldServer::sendDesyncNoun( dword nClientId, const WidgetKey & nNoun )
{
	send( nClientId, WorldClient::CONTEXT_DESYNC_NOUN ) << nNoun;
	return true;
}

bool WorldServer::sendDeleteNoun( dword nClientId, const WidgetKey & nNoun, dword nLastTick )
{
	send( nClientId, WorldClient::CONTEXT_DEL_NOUN ) << nNoun << nLastTick;
	return true;
}

//---------------------------------------------------------------------------------------------------

WorldServer::ClientArray * WorldServer::getNounClients( Noun * pNoun )
{
	if (! pNoun )
		return NULL;
	// resolve noun to it's zone parent
	Noun * pRoot = pNoun->zoneParent();
	if (! pRoot )
		return NULL;

	return getNounClients( pRoot->key() );
}

WorldServer::ClientArray * WorldServer::getNounClients( const WidgetKey & nNoun )
{
	NounClientHash::iterator iClients = m_NounClientHash.find( nNoun );
	if ( iClients == m_NounClientHash.end() )
		return NULL;

	return &(iClients->second);
}

WorldServer::NounArray * WorldServer::getClientNouns( dword nClientId )
{
	ClientContextHash::iterator iContext = m_ClientContext.find( nClientId );
	if ( iContext == m_ClientContext.end() )
		return NULL;

	ClientContext & context = iContext->second;
	return &context.localNouns;
}

// updateClientNouns() handles the movement of the players focus, target, and ship. It checks the local area
// of the player and sends updates for any nouns that have entered the players local area. Some noun types
// are deleted by this routine as well when they leave the local area of the player to conserve client memory.

// It also handles nouns leaving the players local area, only nouns of TYPE_LOCAL_OBJECT are removed
// from the players client. All other noun objects are left attached as proxy objects.

bool WorldServer::updateClientNouns()
{
	PROFILE_START( "WorldServer::updateClientNouns()" );

	// update all clients
	for(int i=0;i<clientCount();++i)
		updateClientNouns( client(i) );

	PROFILE_END();
	return true;
}

static bool InsertRootNoun( WorldServer::NounArray & nouns, Noun * pNoun )
{
	if (! pNoun )
		return false;
	Noun * pRoot = pNoun->zoneParent();
	if (! pRoot )
		return false;
	nouns.insert( pRoot );
	return true;
}

// check a clients current list of local nouns and update it based on the clients current target, focus location and area.
bool WorldServer::updateClientNouns( dword nClientId )
{
	ClientContextHash::iterator iContext = m_ClientContext.find( nClientId );
	if ( iContext == m_ClientContext.end() )
		return false;
	ClientContext & context = iContext->second;
	if ( context.nUserId == 0 || context.nLogoffTime != 0 )
		return false;

	NounArray nouns;
	InsertRootNoun( nouns, context.pSelf );
	InsertRootNoun( nouns, context.pTarget );

	if ( context.pFocus.valid() )
	{
		// get all nouns within the area of our focus, this typically will include our ship
		Array< WorldContext::NounCollision > local;
		worldContext()->proximityCheckLocked( context.pFocus->worldPosition(), 
			context.fFocusArea, local );

		for(int i=0;i<local.size();i++)
		{
			Noun * pNoun = local[i].pNoun;
			if ( pNoun->isRootNoun() )
				nouns.insert( pNoun );
		}
	}

	// compare against our current local nouns, and send add/delete messages to the client...
	NounArray & current = context.localNouns; 

	// scan the current list of local nouns for this client, remove nouns that are no longer local...
	for( NounArray::iterator iNoun=current.begin();iNoun != current.end(); )
	{
		Noun * pNoun = *iNoun;
		if ( nouns.find( pNoun ) != nouns.end() )
		{
			++iNoun;
			continue;		// noun is still local, check next noun...
		}

		// noun is no longer local, do we need to send a delete message to the client?
		if ( pNoun->nounType() == TYPE_LOCAL_OBJECT )
		{
			sendDeleteNoun( nClientId, pNoun->key(), pNoun->tick() );
			context.updateHash.erase( pNoun->key() );		// remove from updateHash as well, if we re-send local object we must re-send the entire object
		}
		else
			sendDesyncNoun( nClientId, pNoun->key() );

		// disassociate the noun and the client...
		m_NounClientHash[ pNoun->key() ].erase( nClientId );
		current.erase( iNoun++ );
	}

	// scan for nouns newly local to our client...
	for( NounArray::iterator iNoun=nouns.begin();iNoun != nouns.end(); ++iNoun )
	{
		Noun * pNoun = *iNoun;

		if ( current.find( pNoun ) != current.end() )
			continue;								// already local to this client...
		BaseNode * pParent = pNoun->parent();		
		if (! pParent )
			continue;								// should NEVER happen...

		// send object to client..
		if ( pNoun->nounType() >= TYPE_LOCAL_OBJECT )// || pNoun->nounType() == TYPE_OBJECT || pNoun->nounType() == TYPE_LANDMARK )
		{
			UpdateHash::iterator iUpdate = context.updateHash.find( pNoun->key() );
			if ( iUpdate == context.updateHash.end() || iUpdate->second != pNoun->version() )
			{
				// send the full object to this client..
				sendAddNoun( nClientId, pParent->key(), pNoun->key(), WidgetWrap( pNoun, FF_TRANSMIT, m_pDictionary ) );
				context.updateHash[ pNoun->key() ] = pNoun->version();
			}
			else
			{
				// send an incremental update of this object to the client instead..
				sendUpdateNoun( nClientId, pParent->key(), pNoun->key(), 
					WidgetWrap( pNoun, FF_TRANSMIT|FF_UPDATE, m_pDictionary) );
			}
		}

		// associate the noun and the client...
		current.insert( pNoun );
		m_NounClientHash[ pNoun->key() ].insert( nClientId );
	}

	return true;
}

bool WorldServer::setNounPosition( dword nClientId, const WidgetKey & nNoun, const Vector3 & vPosition )
{
	Noun::wRef pNoun = nNoun;
	if (! pNoun.valid() )
		return false;

	if ( nClientId != 0 )
	{
		pNoun->setPosition( vPosition );
		pNoun->setLastPosition( vPosition );
	}

	return true;
}

// when a noun is detached and removed from the Universe, this function releases other hash allocations
// made for that noun. This is normally called by onDetachNoun()
bool WorldServer::releaseNoun( Noun * pNoun )
{
	//TRACE( CharString().format( "WorldServer::releaseNoun, key = %s", pNoun->key().string()) );
	NounClientHash::iterator iClients = m_NounClientHash.find( pNoun->key() );
	if ( iClients == m_NounClientHash.end() )
		return false;

	ClientArray & clients = iClients->second;
	for(ClientArray::iterator iClient = clients.begin(); iClient != clients.end(); ++iClient )
	{
		ClientContextHash::iterator iContext = m_ClientContext.find( *iClient );
		if ( iContext != m_ClientContext.end() )
			iContext->second.localNouns.erase( pNoun );
	}

	//for(int i=0;i<clientCount();i++)
	//{
	//	dword nClientId = client(i);
	//	ClientContextHash::Iterator iContext = m_ClientContext.find( nClientId );
	//	if (! iContext.valid() )
	//		continue;
	//	ClientContext & context = *iContext;
	//	context.updateHash.erase( pNoun->key() );
	//}

	
	m_NounClientHash.erase( iClients );
	return true;
}

//----------------------------------------------------------------------------
//EOF
