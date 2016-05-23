/*
	WorldClientUser.cpp
	(c)2000 Palestar Inc, Richard Lyle
*/

#include "WorldClient.h"

//----------------------------------------------------------------------------

void WorldClient::UserWorldClient::onConnected()
{}

void WorldClient::UserWorldClient::onLogin( bool bSuccess )
{}

void WorldClient::UserWorldClient::onUpdateQueuePosition()
{}

void WorldClient::UserWorldClient::onMetaLogin()
{}

void WorldClient::UserWorldClient::onContextReceived()
{}

void WorldClient::UserWorldClient::onContextEnd( const char * pEndMessage )
{}

void WorldClient::UserWorldClient::onDisconnected()
{}

//----------------------------------------------------------------------------

void WorldClient::UserWorldClient::onWorldTransform( const Vector3 & transform )
{}

void WorldClient::UserWorldClient::onServerTransfer( const WorldClient::TransferRequest & a_Req )
{}

void WorldClient::UserWorldClient::onProfile( Profile * pProfile )
{}

void WorldClient::UserWorldClient::onServerStatus( ServerStatus & status)
{}

void WorldClient::UserWorldClient::onChat( const char * pMessage, dword nFromId )
{}

void WorldClient::UserWorldClient::onFlushChat()
{}

void WorldClient::UserWorldClient::onAttachVerb( Verb * pVerb )
{}

//----------------------------------------------------------------------------

void WorldClient::UserWorldClient::onSetFocus( Noun * pFocus )
{}

void WorldClient::UserWorldClient::onSetFocusArea( float fFocusArea )
{}

void WorldClient::UserWorldClient::onSetTarget( Noun * pTarget )
{}

void WorldClient::UserWorldClient::onSetSelf( Noun * pSelf )
{}

void WorldClient::UserWorldClient::onSetTeam( int nTeamId )
{}

void WorldClient::UserWorldClient::onReceiveStorage( UserStorage * a_pStorage )
{}

void WorldClient::UserWorldClient::onAddStorage(const WidgetKey & nKey)
{}

void WorldClient::UserWorldClient::onDeleteStorage(const WidgetKey & nKey)
{}

void WorldClient::UserWorldClient::onGroupInvite( Noun * a_pInvitee )
{}

void WorldClient::UserWorldClient::onGroupUpdate( Group * a_pGroup )
{}

void WorldClient::UserWorldClient::onGroupLeave()
{}

//----------------------------------------------------------------------------
//EOF
