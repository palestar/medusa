/**
	@file WorldServerGroup.cpp
	Grouping code for the WorldServer.

	(c)2009 Palestar Inc
	@author Richard Lyle @date 7/23/2010 9:57:53 PM
*/

#include "WorldServer.h"

//---------------------------------------------------------------------------------------------------

bool WorldServer::checkClientGroup( ClientContext & context )
{
	if ( context.bProxy )
		return true;
	if (! context.pStorage )
		return false;

	// on login, validate any group the player might belong too... disband the group 
	// if it's no longer valid.
	Group::Ref pGroup = context.pStorage->group();
	if ( pGroup.valid() )
	{
		if ( pGroup->memberCount() == 0 || pGroup->findMember( context.nUserId ) == NULL )
		{
			send( context.nClientId, WorldClient::GROUP_DEL );

			// remove invalid groups from a user..
			context.pStorage->setGroup( NULL );
			context.pStorage->save( false );

			return true;
		}

		// first off, check if this group is already instanced on this server, if true then we 
		// want to shared the same Group object between all clients.
		GroupMap::iterator iGroup = m_GroupMap.find( pGroup->key() );
		if ( iGroup != m_GroupMap.end() )
		{
			// group instance is already on this server, share it with this new client..
			pGroup = iGroup->second;
			context.pStorage->setGroup( pGroup );
		}
		else
		{
			// first member of this group on the server, add it to the map..
			m_GroupMap[ pGroup->key() ] = pGroup;
		}

		// check the group always..
		validateGroup( pGroup );

		// disband the group if the number of members is 1 or lower..
		if ( pGroup->memberCount() <= 1 )
		{
			disbandGroup( pGroup );
			return true;
		}

		// remove this client from the group if they are no longer a member or their faction has changed.
		GroupMember * pMember = pGroup->findMember( context.nUserId );
		if ( pMember == NULL || (pGroup->factionId() != context.nFactionId && context.nFactionId != 0) )
		{
			if ( pMember != NULL )
			{
				pGroup->remove( pMember );
				if ( pGroup->memberCount() <= 1 )
					disbandGroup( pGroup );
				else
					sendGroupUpdate( pGroup );		
			}

			send( context.nClientId, WorldClient::GROUP_DEL );

			// no longer a member of the group, so remove it from our storage..
			context.pStorage->setGroup( NULL );
			context.pStorage->save( false );
		}
	}

	return true;
}

bool WorldServer::groupInvite( ClientContext & context, Noun * pTarget )
{
	if ( !context.pStorage || !context.pSelf || !pTarget )
		return false;		// invalid target
	dword nTargetUserId = pTarget->userId();
	if ( nTargetUserId == 0 )
		return false;		// can't invite AI
	if ( nTargetUserId == context.nUserId )
		return false;		// can't invite self into group
	dword nTargetClientId = getClientId( nTargetUserId );
	if ( nTargetClientId == 0 )
		return false;		// no client ID, user isn't connected anymore..
	ClientContextHash::iterator iContext = m_ClientContext.find( nTargetClientId );
	if ( iContext == m_ClientContext.end() )
		return false;		// no context for the user
	ClientContext & tcontext = iContext->second;
	if (! tcontext.pStorage )
		return false;
	if ( context.nFactionId != tcontext.nFactionId )
		return false;		// can't invite enemies to group

	Group * pGroup = context.pStorage->group();
	Group * pTargetGroup = tcontext.pStorage->group();

	if ( pTargetGroup != NULL )
	{
		if ( pGroup == NULL && pTargetGroup->isPending( context.nUserId ) )
		{
			// user is acepting an invite..
			context.pStorage->setGroup( pTargetGroup );
			pTargetGroup->clearPending( context.nUserId );

			sendChat( nTargetClientId, CharString().format( "SERVER: '%s' has accepted your invite.", context.MetaProfile.name.cstr() ) );
			sendChat( context.nClientId, CharString().format( "SERVER: You are now grouped with '%s'.", pTarget->name() ) );

			// send updated group to all members
			return sendGroupUpdate( pTargetGroup );
		}

		sendChat( context.nClientId, CharString().format( "SERVER: '%s' is already in a group.", pTarget->name() ) );
		return false;
	}

	if ( pGroup == NULL )
	{
		// create new group, invite other client into our group...
		pGroup = new Group();
		pGroup->setFactionId( context.nFactionId );

		m_GroupMap[ pGroup->key() ] = pGroup;

		pGroup->add( new GroupMember( false, true, context.nUserId, 
			context.MetaProfile.name, context.pSelf.key(), getNounType( context.pSelf ) ) );
		pGroup->add( new GroupMember( true, false, tcontext.nUserId, 
			tcontext.MetaProfile.name, pTarget->key(), getNounType( pTarget ) ) );

		context.pStorage->setGroup( pGroup );
		send( nTargetClientId, WorldClient::GROUP_INVITE ) << context.pSelf;
		sendChat( context.nClientId, CharString().format( "SERVER: You have invited '%s' into your group.", pTarget->name() ) );

		return sendGroupUpdate( pGroup );
	}
	
	validateGroup( pGroup );

	if ( pGroup->memberCount() >= Group::sm_nMaxGroupMembers )
	{
		sendChat( context.nClientId, "SERVER: Group is full!" );
		return false;
	}
	if (! pGroup->isLeader( context.nUserId ) )
	{
		sendChat( context.nClientId, "SERVER: You are not the leader - you can't invite into the group." );
		return false;
	}

	pGroup->add( new GroupMember( true, false, tcontext.nUserId,
		tcontext.MetaProfile.name, pTarget->key(), getNounType( pTarget ) ) );
	send( nTargetClientId, WorldClient::GROUP_INVITE ) << context.pSelf;

	return sendGroupUpdate( pGroup );
}

bool WorldServer::groupDecline( ClientContext & context, Noun * pTarget )
{
	if (! pTarget )
		return false;
	dword nTargetClientId = getClientId( pTarget->userId() );
	if ( nTargetClientId == 0 )
		return false;		// no client ID, user isn't connected anymore..
	ClientContextHash::iterator iContext = m_ClientContext.find( nTargetClientId );
	if ( iContext == m_ClientContext.end() )
		return false;		// no context for the user
	ClientContext & tcontext = iContext->second;
	if (! tcontext.pStorage )
		return false;
	Group * pGroup = tcontext.pStorage->group();
	if (! pGroup )
		return false;
	GroupMember * pMember = pGroup->findMember( context.nUserId );
	if (! pMember )
		return false;
	if (! pMember->isPending() )
		return false;

	sendChat( nTargetClientId, CharString().format( "SERVER: '%s' has decline your invite.", context.MetaProfile.name.cstr() ) );

	pGroup->remove( pMember );
	validateGroup( pGroup, false );

	// disband the group when it falls to only one person..
	if ( pGroup->memberCount() <= 1 )
		return disbandGroup( pGroup );
	else
		return sendGroupUpdate( pGroup );
}

bool WorldServer::groupRemove( ClientContext & context, Noun * pTarget )
{
	if ( !context.pStorage || !context.pSelf || !pTarget )
		return false;		// invalid target
	Group::Ref  pGroup = context.pStorage->group();
	if (! pGroup.valid() )
		return false;		// not even in a group..

	// in order to remove a member - you either need to be the leader or removing yourself..
	if (! pGroup->isLeader( context.nUserId ) && context.pSelf != pTarget )
	{
		sendChat( context.nClientId, "SERVER: You are not the leader - you can't remove group members." );
		return false;
	}

	GroupMember * pMember = pGroup->findMember( pTarget->userId() );
	if (! pMember )
		return false;
	if ( context.pSelf != pTarget && pMember->isLeader() )
	{
		sendChat( context.nClientId, "SERVER: You can't remove another group leader!" );
		return false;
	}

	// client may not be connected, but allow the removal anyway if this is the case..
	dword nRemoveClientId = getClientId( pMember->userId() );
	if ( nRemoveClientId != 0 )
	{
		ClientContextHash::iterator iContext = m_ClientContext.find( nRemoveClientId );
		if ( iContext == m_ClientContext.end() )
			return false;		// no context for the user
		ClientContext & tcontext = iContext->second;
		if (! tcontext.pStorage )
			return false;

		if ( !pMember->isPending() )
		{
			tcontext.pStorage->setGroup( NULL );
			tcontext.pStorage->save( false );

			// inform the client they are no longer in the group..
			if ( context.pSelf != pTarget )
				sendChat( nRemoveClientId, CharString().format( "SERVER: '%s' has removed you from the group.", context.MetaProfile.name.cstr() ) );
			else
				sendChat( nRemoveClientId, "SERVER: You have left the group." );

			// send message to delete the group info...
			send( nRemoveClientId, WorldClient::GROUP_DEL );
		}
	}

	// remove the member from the group...
	pGroup->remove( pMember );
	// if group has no leader, auto-pick the player with the highest prestige..
	validateGroup( pGroup, false );
	// disband the group when it falls to only one person..
	if ( pGroup->memberCount() <= 1 )
		return disbandGroup( pGroup );
	else
		return sendGroupUpdate( pGroup );
}

bool WorldServer::groupPromote( ClientContext & context, Noun * pTarget)
{
	if ( !context.pStorage || !context.pSelf || !pTarget )
		return false;		// invalid target
	Group * pGroup = context.pStorage->group();
	if (! pGroup )
		return false;		// not even in a group..

	// in order to remove a member - you either need to be the leader or removing yourself..
	if (! pGroup->isLeader( context.nUserId ) )
	{
		sendChat( context.nClientId, "SERVER: You are not the leader - you can't promote group members." );
		return false;
	}
	GroupMember * pMember = pGroup->findMember( pTarget->userId() );
	if (! pMember )
		return false;

	// set target to leader status
	pMember->setIsLeader( true );

	return sendGroupUpdate( pGroup );
}

bool WorldServer::sendGroupUpdate( Group * pGroup )
{
	WidgetWrap group( pGroup, FF_TRANSMIT, m_pDictionary );
	for(int i=0;i<pGroup->memberCount();++i)
	{
		GroupMember * pMember = pGroup->member(i);
		if ( pMember->isPending() )
			continue;		// don't send to pending members!

		dword nClientId = getClientId( pMember->userId() );
		if ( nClientId != 0 )
		{
			send( nClientId, WorldClient::GROUP_UPDATE ) << group;

			// save the storage to disk on group changes..
			ClientContextHash::iterator iContext = m_ClientContext.find( nClientId );
			if ( iContext == m_ClientContext.end() )
				continue;
			ClientContext & context = iContext->second;
			if (! context.pStorage )
				continue;

			context.pStorage->save( false );
		}
	}

	return true;
}

bool WorldServer::disbandGroup( Group * pGroup )
{
	Group::Ref rGroup( pGroup );
	for(int i=0;i<pGroup->memberCount();++i)
	{
		GroupMember * pMember = pGroup->member(i);
		if ( pMember->isPending() )
			continue;		// don't send to pending members!

		dword nClientId = getClientId( pMember->userId() );
		if ( nClientId == 0 )
			continue;		// user isn't connected

		ClientContextHash::iterator iContext = m_ClientContext.find( nClientId );
		if ( iContext == m_ClientContext.end() )
			continue;
		ClientContext & context = iContext->second;
		if (! context.pStorage )
			continue;

		context.pStorage->setGroup( NULL );
		context.pStorage->save( false );

		send( nClientId, WorldClient::GROUP_DEL );
	}

	// lastly, remove from the map..
	m_GroupMap.erase( pGroup->key() );

	return true;
}

void WorldServer::validateGroup( Group * pGroup, bool bAllowUpdate /*= true*/ )
{
	bool bLeaderFound = false;
	bool bSendUpdate = false;

	// check group members.. 
	// 1. remove any pending members who have disconnected..
	// 2. remove any current members who are members of a different group.
	int i = 0;
	while( i < pGroup->memberCount() )
	{
		GroupMember * pMember = pGroup->member(i);
		bLeaderFound |= pMember->isLeader();

		dword nClientId = getClientId( pMember->userId() );
		if ( pMember->isPending() && nClientId == 0 )
		{
			pGroup->remove( pMember );
			bSendUpdate = true;
			continue;
		}

		if ( nClientId != 0 )
		{
			ClientContextHash::iterator iContext = m_ClientContext.find( nClientId );
			if ( iContext != m_ClientContext.end() )
			{
				ClientContext & context = iContext->second;
				if ( context.pStorage && context.pStorage->group() != pGroup )
				{
					pGroup->remove( pMember );		// client is member of a diff group now
					bSendUpdate = true;
					continue;
				}
			}
		}

		// member is valid, next member..
		i += 1;	
	}

	if (! bLeaderFound )
	{
		GroupMember * pBestLeader = NULL;
		float fBestScore = 0.0f;

		// no leader, find player with highest prestige and set them as the leader..
		for(int i=0;i<pGroup->memberCount();++i)
		{
			GroupMember * pMember = pGroup->member(i);
			dword nClientId = getClientId( pMember->userId() );
			if ( nClientId == 0 )
				continue;
			ClientContextHash::iterator iContext = m_ClientContext.find( nClientId );
			if ( iContext == m_ClientContext.end() )
				continue;

			ClientContext & context = iContext->second;
			if ( pBestLeader == NULL || context.MetaProfile.score > fBestScore )
			{
				pBestLeader = pMember;
				fBestScore = context.MetaProfile.score;
			}
		}

		if ( pBestLeader != NULL )
		{
			pBestLeader->setIsLeader( true );
			bSendUpdate = true;
		}
	}

	if ( bSendUpdate && bAllowUpdate )
		sendGroupUpdate( pGroup );
}

bool WorldServer::groupConnected( Group * pGroup )
{
	for(int i=0;i<pGroup->memberCount();++i)
	{
		GroupMember * pMember = pGroup->member(i);
		if ( getClientId( pMember->userId() ) != 0 )
			return true;
	}
	return false;
}

Group * WorldServer::getGroup( dword nUserId )
{
	UserStorage * pStorage = getStorage( nUserId );
	if ( pStorage != NULL )
		return pStorage->group();
	return NULL;
}

int WorldServer::getNounType( Noun * pNoun )
{
	return 0;
}

//---------------------------------------------------------------------------------------------------
//EOF
