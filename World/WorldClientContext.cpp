/*
	WorldClientContext.cpp
	(c)2000 Palestar Inc, Richard Lyle
*/


#include "Debug/Assert.h"
#include "Resource/Resource.h"
#include "WorldClient.h"

#pragma warning(disable:4018)	// warning C4018: '==' : signed/unsigned mismatch

//----------------------------------------------------------------------------

bool WorldClient::isServer() const
{
	return false;
}

dword WorldClient::userId() const
{
	if ( profile() )
		return profile()->userId();
	return 0;		// no profile, so not logged in yet...
}

void WorldClient::onChat( Noun * pNoun, const char * pMessage )
{
	pushChat( pMessage, pNoun != NULL ? pNoun->userId() : 0 );
}

bool WorldClient::canAttachVerb( Noun * pTarget, Verb * pVerb )
{
	if ( pVerb->clientId() == 0 )
	{
		// make sure the verb tick has been set, if not use the current context tick plus 250 ms
		if ( pVerb->tick() == 0 )
			pVerb->setTick( tick() );
		
		// send verb onto the server
		sendVerb( pTarget, pVerb );
		return true;
	}

	// verb is from server, let it be attached
	return true;
}

void WorldClient::onAttachVerb( Verb * pVerb )
{}

void WorldClient::onAddTeam( const Team & team )
{}

void WorldClient::onDeleteTeam( int nTeamId )
{}

void WorldClient::onAddCondition( const Condition & condition )
{}

void WorldClient::onDeleteCondition( int conditionId )
{}

void WorldClient::onAddZone( NodeZone * pZone )
{}

void WorldClient::onDeleteZone( NodeZone * pZone )
{}

void WorldClient::onAttachNoun( Noun * pNoun )
{}

void WorldClient::onDetachNoun( Noun * pNoun )
{}

void WorldClient::onTransferNoun( Noun * pNoun, NodeZone * pNewZone, NodeZone * pOldZone )
{}

void WorldClient::onWorldTransform( const Vector3 & transform )
{
	// inform our user that the worldContext is being transformed
	user()->onWorldTransform( transform );
}

void WorldClient::onCondition( int conditionId )
{}

void WorldClient::onServerTransfer( Noun * pWho, const char * a_pAddress, int a_nPort, const char * a_pSpawnPoint )
{
	// just set the flag to true ... the server will send us a message to do the actual transfer!
	// setting this flag will prevent the ship selection screen from getting shown...
	m_bServerTransfer = true;

	// notify our user that we are waiting for the server transfer message..
	TransferRequest req;
	req.m_sAddress = a_pAddress;
	req.m_nPort = a_nPort;
	req.m_nSelf = pWho != NULL ? pWho->key() : NULL_WIDGET; 
	req.m_sSpawnPoint = a_pSpawnPoint;

	user()->onServerTransfer( req );
}

//---------------------------------------------------------------------------------------------------
//EOF
