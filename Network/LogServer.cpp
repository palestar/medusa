/*
	LogServer.cpp
	(c)2005 Palestar Inc, Richard Lyle
*/

#include "Debug/Log.h"
#include "LogServer.h"

//----------------------------------------------------------------------------

LogServer::LogServer() : m_nLogLevel( LL_STATUS )
{}

//----------------------------------------------------------------------------

void LogServer::onConnect( dword client )
{
	AutoLock lock( &m_Lock );

	ClientContext & context = m_hClientContext[ client ];
	context.nSessionId = 0;
	context.sName = "Unknown";
	context.bLoggedIn = false;

	LOG_STATUS( "LogServer", CharString().format( "Client %u connected from %s", client, clientAddress( client ) ) );
}

void LogServer::onReceive( dword nClientId, byte nMessage, const InStream & input )
{
	switch( nMessage )
	{
	case LogClient::SERVER_LOGIN:
		{
			dword nSessionId;
			input >> nSessionId;
			CharString sName;
			input >> sName;

			//TRACE( CharString().format( "SERVER_LOGIN, clientId = %u, nSessionId = %u, name = %s", nClientId, nSessionId, sName) );
			
			AutoLock lock( &m_Lock );

			if ( onLogin( nClientId, nSessionId, sName ) )
			{
				LOG_STATUS( "LogServer", CharString().format( "Client %u login, sessionId = %u, name = %s", nClientId, nSessionId, (const char *)sName) );

				ClientContext & context = m_hClientContext[ nClientId ];
				context.nSessionId = nSessionId;
				context.sName = sName;
				context.bLoggedIn = true;

				// login the client, send them the current login level as well
				OutStream output( send( nClientId, LogClient::CLIENT_LOGIN ) );
				output << m_nLogLevel;
				output.flush();
			}
			else
			{
				// login failed
				LOG_STATUS( "LogServer", CharString().format( "Client %u failed to login...", nClientId) );
				// disconnect the client
				removeClient( nClientId );
			}
		}
		break;
	case LogClient::SERVER_RECV_LOG:
		{
			CharString sFile;
			input >> sFile;
			unsigned int nLine = 0;
			input >> nLine;
			unsigned int nLogLevel = 0;
			input >> nLogLevel;
			CharString sSystem;
			input >> sSystem;
			CharString sMessage;
			input >> sMessage;

			//TRACE( CharString().format( "SERVER_RECV_LOG, clientId = %u, log = %s", nClientId, (const char *)sLog) );

			AutoLock lock( &m_Lock );

			ClientContext & context = m_hClientContext[ nClientId ];
			if ( context.bLoggedIn )
				Logging::report( sFile, nLine, nLogLevel, sSystem, context.sName + " | " + sMessage );
		}
		break;
	}
}

void LogServer::onDisconnect( dword nClientId )
{
	AutoLock lock( &m_Lock );

	LOG_STATUS( "LogServer", CharString().format( "Client %u disconnected...", nClientId ) );
	m_hClientContext.remove( nClientId );
}

//----------------------------------------------------------------------------

bool LogServer::start( unsigned int nLogLevel, int nPort, int nMaxClients )
{
	m_nLogLevel = nLogLevel;

	if (! Server::start( new Socket( "ZLIB" ), nPort, nMaxClients ) )
		return false;

	LOG_STATUS( "LogServer", CharString().format( "Log server running, logLevel = %d, port = %u, maxClients = %u", 
		m_nLogLevel, nPort, nMaxClients ) );

	return true;
}

bool LogServer::stop()
{
	Server::stop();
	return true;
}

//----------------------------------------------------------------------------

bool LogServer::onLogin( dword nClientId, dword nSessionId, const char * pName )
{
	return true;
}

//----------------------------------------------------------------------------
//EOF
