/*
	WorldServerUpdate.cpp
	(c)2000 Palestar Inc, Richard Lyle
*/


#include "Debug/Assert.h"
#include "Debug/Profile.h"
#include "Standard/Timer.h"
#include "Standard/SortedArray.h"
#include "Standard/Time.h"
#include "File/FileDisk.h"
#include "Constants.h"
#include "WorldServer.h"

//----------------------------------------------------------------------------

const int		MAX_PINGS = 1;											// PING_CLIENT_RATE * MAX_PINGS = Total time before dead-client is disconnected
const dword		UDP_DOWN_TIME = 15;										// if no UDP packets received within this time period, UDP is considered down..

//----------------------------------------------------------------------------

bool WorldServer::updateServer()
{
	// throttle the number of times this routine can be called per second... no need to call faster than the current
	// ticks per second of the game world.
	m_UpdateTimer.wait();

	if (! worldContext() )
		return false;
	if (! active() )
		return false;		// server is stopped...

	PROFILE_START("WorldServer::updateServer()");

	// global lock while we update the world and clients...
	lock();

	// check if we should enter sleep mode
	if ( !m_bSleeping && m_DeleteClientList.size() == 0 && clientCount() == 0 )
	{
		LOG_STATUS( "WorldServer", "Entering sleep mode." );
		m_bSleeping = true;
	}

	// send & receive data from clients..
	Server::update();

	// update the world simulation
	worldContext()->update();

	pingClients();				// sends / receives ping messages, disconnects dead clients
	updateClientNouns();		// update noun/client hash tables
	loginClients();				// process pending logins
	deleteClients();			// cleanup disconnected clients
	saveWorld();				// persist the game world
	flushProfileQueue();		// flush out player profile changes
	logStats();

	// we are done, release our global lock...
	unlock();

	PROFILE_END();

	if ( m_bStopServer )
		return false;

	return true;
}

//----------------------------------------------------------------------------

bool WorldServer::pingClients()
{
	if (! worldContext() )
		return false;

	if ( m_PingTimerUDP.signaled() )
	{
		for(int i=0;i<clientCount();++i)
		{
			dword nClientID = client( i );
			ClientContextHash::iterator iContext = m_ClientContext.find( nClientID );
			if ( iContext == m_ClientContext.end() )
				continue;

			// we always try to ping no matter if the connections is up or down... 
			// this is done in order to keep the UDP connection alive...
			try {
				if ( m_Context.bEnableUDP )
					sendUDP( nClientID, WorldClient::PING ) << Time::milliseconds();
			}
			catch( File::FileError )
			{}

			ClientContext & context = iContext->second;
			if ( (context.nLastUDP + UDP_DOWN_TIME) > Time::seconds() )
			{
				// UDP connection is active..
				if (! context.bConnectedUDP )
				{
					LOG_STATUS( "WorldServer", CharString().format("Client %u UDP connection active!", context.nClientId ) );
					context.bConnectedUDP = true;
				}
			}
			else if ( context.bConnectedUDP )
			{
				LOG_STATUS( "WorldServer", CharString().format( "Client %u UDP connection down!", context.nClientId ) );
				context.bConnectedUDP = false;
			}

		}
	}

	if ( m_PingTimer.signaled() )
	{
		// get the current worldContext tick
		dword nTick = worldContext()->tick();

		Queue< dword > dead;
		// ping all clients...
		for(int i=0;i<clientCount();++i)
		{
			dword nClientId = client(i);

			ClientContextHash::iterator iContext = m_ClientContext.find( nClientId );
			if ( iContext == m_ClientContext.end() )
				continue;

			ClientContext & context = iContext->second;
			if ( context.nPings <= MAX_PINGS )
			{
				++context.nPings;
				// measure their latency
				try {
					send( nClientId, WorldClient::PING ) << Time::milliseconds();
					sendClient( nClientId, WorldClient::CONTEXT_CRON, true, true ) << nTick;
				}
				catch( File::FileError )
				{}
			}
			else
			{
				// client not responding to pings, remove them..
				dead.push( nClientId );	
			}
		}

		while( dead.valid() )
		{
			dword nClientId = *dead;
			dead.next();
			
			LOG_STATUS( "WorldServer", CharString().format("Client %u dead, disconnecting...", nClientId ) );

			onDisconnect( nClientId );
			Server::removeClient( nClientId );
		}
	}

	return true;
}

void WorldServer::receivePong( ClientContext & context, dword nTime, bool bUDP )
{
	if (! bUDP )
	{
		context.nLatency = (Time::milliseconds() - nTime) / 2;
		context.nPings = 0;
	}
	else
	{
		context.nLatencyUDP = (Time::milliseconds() - nTime) / 2;
	}
}

void WorldServer::flushProfileQueue(bool bForce /*= false*/)
{
	int nMaxUpdates = m_ProfileUpdateQueue.updateCount();
	int nUpdates = 0;

	// pop any pending profile updates from the ProfileUpdateQueue object and push them to 
	// the meta server for persistent storage...
	ProfileUpdateQueue::ProfileUpdate update;
	while( nUpdates < nMaxUpdates && m_ProfileUpdateQueue.popUpdate( update, bForce ) )
	{
		++nUpdates;

		// try to add the profile data... 
		if (! m_MetaClient.addProfile( update.nUserId, update.nFieldId, update.fAmount ) )
		{
			// failed to send profile update to the server, push the update back into the queue
			// so we will try again later.
			m_ProfileUpdateQueue.pushUpdate( update.nUserId, update.nFieldId, update.fAmount );

			// TODO: Should we log something?
		}
	}
}

void WorldServer::logStats()
{
	if (! m_StatsTimer.signaled() )
		return;

	dword nTime = serverTime();
    dword nElapsed = nTime - m_nLastStatsLog;
	if ( nElapsed < 1 )
		nElapsed = 1;
	m_nLastStatsLog = nTime;

	dword nTotalReceived = bytesReceived() + bytesReceivedUDP();
	dword nReceived = nTotalReceived - m_nLastRecvBytes;
	m_nLastRecvBytes = nTotalReceived;

	dword nTotalSent = bytesSent() + bytesSentUDP();
	dword nSent = nTotalSent - m_nLastSentBytes;
	m_nLastSentBytes = nTotalSent;

	LOG_STATUS( "WorldServer", CharString().format( "Clients: %u / %u, Received: %.1f k : %.1f k/s, Sent: %.1f k : %.1f k/s, Load: %.2f",
		clientCount(), maxClients(),
		float( nReceived  ) / 1024.0f,
		float( nReceived ) / nElapsed / 1024.0f,
		float( nSent ) / 1024.0f,
		float( nSent ) / nElapsed / 1024.0f,
		Profiler::CPUused() ) );
}

//----------------------------------------------------------------------------
//EOF
