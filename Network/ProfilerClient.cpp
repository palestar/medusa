/*
	ProfilerClient.cpp
	(c)2004 Palestar Inc, Richard Lyle
*/

#include "ProfilerClient.h"

//----------------------------------------------------------------------------

ProfilerClient::ProfilerClient() : m_bConnected( false ), m_nTotalCPU( 0 )
{}

void ProfilerClient::onConnect()
{
	m_bConnected = true;
}

void ProfilerClient::onReceive( byte message, const InStream & input )
{
	switch( message )
	{
	case CLIENT_RECV_STATE:
		lock();
		input >> m_nTotalCPU;
		input >> m_Messages;
		input >> m_Profiles;
		unlock();
		break;
	}
}

void ProfilerClient::onDisconnect()
{
	m_bConnected = false;
}

//----------------------------------------------------------------------------

void ProfilerClient::sendUpdate()
{
	// request a state update from the server
	send( SERVER_REQ_STATE );
}


//----------------------------------------------------------------------------
//EOF
