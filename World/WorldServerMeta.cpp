/*
	WorldServerMeta.cpp
	(c)2000 Palestar Inc, Richard Lyle
*/


#include "Debug/Assert.h"
#include "Standard/Time.h"
#include "Standard/StringBuffer.h"
#include "WorldServer.h"

Constant UPDATE_PROFILE_TIME( "UPDATE_PROFILE_TIME", 60.0f );			// how long to wait before sending updated profile information
Constant MAX_PROFILE_UPDATES( "MAX_PROFILE_UPDATES", 8.0f );			// maximum number of profiles to update in 1 second

//----------------------------------------------------------------------------

bool WorldServer::checkMetaServer()
{
	// check if we are logged in
	if (! m_MetaClient.loggedIn() )
	{
		// attempt to reconnect to the meta-server
		if (! m_MetaClient.open( m_Context.sMetaAddress, m_Context.nMetaPort ) )
		{
			LOG_ERROR( "WorldServer", "<color;ffffff>CLIENT: Failed to re-connect to meta-server!" );
			return false;
		}

		if ( m_MetaClient.login( m_Context.user, m_Context.password ) < 0 )
		{
			// try to login using our sessionid then
			if ( m_MetaClient.login( m_Context.nSessionId ) < 0 )
			{
				LOG_ERROR( "WorldServer", "<color;ffffff>CLIENT: Failed to login to meta-server, closing connection!" );
				m_MetaClient.close();
				return false;
			}
		}

		// save our sessionId on successful user/password login
		m_Context.nSessionId = m_MetaClient.profile().sessionId;

		// get our external ip address if none is provided
		// select the correct game
		if ( m_MetaClient.selectGame( m_Context.gameId ) < 0 )
		{
			LOG_ERROR( "WorldServer", "<color;ffffff>CLIENT: Failed to select game!" );
			m_MetaClient.close();
			return false;
		}

		// update our address
		if ( m_Context.address.length() == 0 )
			m_MetaClient.getAddress( m_Context.address );
		m_Server.address = m_Context.address;

		LOG_STATUS( "WorldServer", "<color;ffffff>CLIENT: Connected to the meta-server..." );

		// publish this server immediately
		publishServer();
	}

	return true;
}

bool WorldServer::publishServer()
{
	// Register this server with the meta server.. 
	return false;
}

bool WorldServer::updateLoginQueue()
{
	lock();

	m_LoginQueue.reset();
	while( m_LoginQueue.valid() )
	{
		if ( (*m_LoginQueue).bDone )
		{
			m_LoginQueue.next();		// skip, already processed
			continue;
		}

		LoginJob & job = *m_LoginQueue;
		unlock();

		job.profile.userId = 0;
		job.profile.clanId = 0;

		if ( m_MetaClient.loginByProxy( job.nSessionId, job.profile ) < 0 )
			LOG_ERROR( "WorldServer", CharString().format("Failed to login client %u with session id %u", job.nClientId, job.nSessionId) );

		// get the players clan information as well...
		if ( job.profile.userId != 0 && job.profile.clanId != 0 )
		{
			// get the clan information from the metaserver
			if ( m_MetaClient.getClan( job.profile.clanId, job.clan ) < 0 )
				job.clan.clanId = 0;
		}

		lock();
		job.bDone = true;
	}

	unlock();

	return true;
}

//----------------------------------------------------------------------------

const int UPDATE_META_RATE = 60 * 5;		// update the meta-server every 5 minutes

int WorldServer::metaUpdateDemon()
{
	dword nPublishTime = 0;

	// main loop for our thread...
	while( active() )
	{
		Thread::sleep( 100 );
		
		m_MetaClient.update();

		// make sure we are connected with the MetaServer
		if ( checkMetaServer() )
		{
			if ( m_Context.bPublish && (Time::seconds() - nPublishTime) > UPDATE_META_RATE )
			{
				if ( publishServer() )
					nPublishTime = Time::seconds();
			}

			updateLoginQueue();
		}
		else
		{
			LOG_STATUS( "WorldServer", "Connection to the MetaServer lost..." );
			Thread::sleep( 5 * 1000 );
		}

	}

	// close out our connection to the metasever, otherwise our server will remain..
	if ( m_MetaClient.loggedIn() )
	{
		if ( m_Context.bPublish )
			m_MetaClient.removeServer();
	}


	return 0;
}

//----------------------------------------------------------------------------
//EOF
