/*
	WorldContextUser.cpp
	(c)2000 Palestar Inc, Richard Lyle
*/

#include "Debug/Trace.h"
#include "WorldContext.h"

//----------------------------------------------------------------------------

bool WorldContext::User::isServer() const
{
	return false;
}

bool WorldContext::User::isSleeping() const
{
	return false;
}

dword WorldContext::User::serverRole() const
{
	return 0xffffffff;
}

dword WorldContext::User::userId() const
{
	return 0xffffffff;
}

int WorldContext::User::maxPlayers() const
{
	return 1;
}

int WorldContext::User::playerCount() const
{
	return 0;
}

int WorldContext::User::spawnedScore( int a_nFactionId )
{
	return 0;
}

int WorldContext::User::maxPlayerScoreAI()
{
	return 0;
}

int WorldContext::User::baseTeamAi() const
{
	return 0;
}

void WorldContext::User::onChat( Noun * pNoun, const char * pMessage )
{}

bool WorldContext::User::canAttachVerb( Noun * pTarget, Verb * pVerb )
{
	return true;
}

void WorldContext::User::onAttachVerb( Verb * pVerb )
{}

void WorldContext::User::onAddCondition( const Condition & condition )
{}

void WorldContext::User::onDeleteCondition( int conditionId )
{}

void WorldContext::User::onCondition( int condition )
{}

void WorldContext::User::onAddTeam( const Team & team )
{}

void WorldContext::User::onDeleteTeam( int nTeamId )
{}

void WorldContext::User::onAddWorld( WorldContext * pWorld )
{}

void WorldContext::User::onDeleteWorld( WorldContext * pWorld )
{}

void WorldContext::User::onAddZone( NodeZone * pZone )
{}

void WorldContext::User::onDeleteZone( NodeZone * pZone )
{}

void WorldContext::User::onAttachNoun( Noun * pNoun )
{}

void WorldContext::User::onUpdateNoun( Noun * pNoun )
{}

void WorldContext::User::onDetachNoun( Noun * pNoun )
{}

void WorldContext::User::onTransferNoun( Noun * pNoun, NodeZone * pNewZone, NodeZone * pOldZone )
{}

void WorldContext::User::onWorldTransform( const Vector3 & transform )
{}

void WorldContext::User::onServerTransfer( Noun * pWho, const char * a_pAddress, int a_nPort, const char * a_pSpawnPoint )
{}

bool WorldContext::User::saveStorage(Noun * a_pSelf )
{
	return false;
}

//----------------------------------------------------------------------------
//EOF
