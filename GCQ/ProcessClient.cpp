/*
	ProcessClient.cpp
	(c)2000 Palestar Inc, Richard Lyle
*/

#include "Standard/MD5.h"
#include "GCQ/ProcessClient.h"

//----------------------------------------------------------------------------

ProcessClient::ProcessClient() : m_Connected( false ), m_LoggedIn( false ), m_nSessionId( 0 )
{}

//----------------------------------------------------------------------------

void ProcessClient::onConnect()
{
	m_Connected = true;
}

void ProcessClient::onReceive( byte message, const InStream & input )
{
	switch( message )
	{
	case CLIENT_RECV_PROCESS_LIST:
		{
			dword job;
			input >> job;
			Array< Process > processList;
			input >> processList;

			AutoLock lock( &m_Lock );
			if ( jobResult( job ) != NULL )
				*((Array< Process > *)jobResult( job )) = processList;
			signalJob( job, true );
		}
		break;
	case CLIENT_RECV_CONFIG:
		{
			dword job;
			input >> job;
			CharString config;
			input >> config;

			AutoLock lock( &m_Lock );
			if ( jobResult( job ) != NULL )
				*((CharString *)jobResult(job)) = config;
			signalJob( job, true );
		}
		break;
	case CLIENT_RECV_LOG:
		{
			dword job;
			input >> job;
			CharString log;
			input >> log;

			AutoLock lock( &m_Lock );
			if ( jobResult( job ) != NULL )
				*((CharString *)jobResult(job)) = log;
			signalJob( job, true );
		}
		break;
	case CLIENT_RECV_STATUS:
		{
			dword job;
			input >> job;
			Status status;
			input >> status;

			AutoLock lock( &m_Lock );
			if ( jobResult( job ) != NULL )
				*((Status *)jobResult(job)) = status;
			signalJob( job, true );
		}
		break;
	case CLIENT_RECV_LOG_ID:
		{
			dword job;
			input >> job;
			dword logId;
			input >> logId;

			signalJob( job, logId );
		}
		break;
	case CLIENT_RECV_LOG_UPDATE:
		{
			dword logId;
			input >> logId;
			CharString line;
			input >> line;

			AutoLock lock( &m_Lock );
			if ( m_Log.find( logId ).valid() )
				m_Log[ logId ].push( line );
		}
		break;
	case CLIENT_RECV_SEARCHRESULT:
		{
			dword job;
			input >> job;
			CharString resultString;
			input >> resultString;

			AutoLock lock( &m_Lock );
			if ( jobResult( job ) != NULL )
				*((CharString *)jobResult(job)) = resultString;
			signalJob( job, true );
		}
		break;
	case CLIENT_JOB_DONE:
		{
			dword job;
			input >> job;
			bool jobDone;
			input >> jobDone;

			signalJob( job, jobDone );
		}
		break;
	case PING:
		send( PONG );
		break;
	case PONG:
		break;
	}
}

void ProcessClient::onDisconnect()
{
	m_Log.release();
	m_Connected = false;
	m_LoggedIn = false;
}

//----------------------------------------------------------------------------

bool ProcessClient::connected() const
{
	return m_Connected;
}

bool ProcessClient::loggedIn() const
{
	return m_LoggedIn;
}

dword ProcessClient::sessionId() const
{
	return m_nSessionId;
}

//----------------------------------------------------------------------------

bool ProcessClient::open( const char * serverAddress, int serverPort )
{
	// open connection to server
	if (! Client::open( new Socket( "ZLIB" ), serverAddress, serverPort ) )
		return false;
	m_Connected = true;

	return true;
}

bool ProcessClient::login( const char * pID, const char * pPW )
{
	if (! m_Connected )
		return false;

	dword job = createJob();
	send( SERVER_LOGIN ) << job << CharString( pID ) << MD5( CharString( pPW ) ).checksum();

	m_LoggedIn = waitJob( job ) > 0;
	return m_LoggedIn;
}

bool ProcessClient::login( dword sessionId )
{
	if (! m_Connected )
		return false;

	m_nSessionId = sessionId;

	dword job = createJob();
	send( SERVER_SESSION_LOGIN ) << job << sessionId;

	m_LoggedIn = waitJob( job ) > 0;
	return m_LoggedIn;
}

bool ProcessClient::getProcessList( Array< Process > & process )
{
	if (! m_LoggedIn )
		return false;

	dword job = createJob( &process );
	send( SERVER_SEND_PROCESS_LIST ) << job;

	return waitJob( job ) > 0;
}

bool ProcessClient::getConfig( dword processId, CharString & config )
{
	if (! m_LoggedIn )
		return false;

	dword job = createJob( &config );
	send( SERVER_SEND_CONFIG ) << job << processId;

	return waitJob( job ) > 0;
}

bool ProcessClient::putConfig( dword processId, const CharString & config )
{
	if (! m_LoggedIn )
		return false;

	dword job = createJob();
	send( SERVER_RECV_CONFIG ) << job << processId << config;

	return waitJob( job ) > 0;

}

bool ProcessClient::getLog( dword processId, CharString & log )
{
	if (! m_LoggedIn )
		return false;

	dword job = createJob( &log );
	send( SERVER_SEND_LOG ) << job << processId;

	return waitJob( job ) > 0;
}

bool ProcessClient::addProcess( const Process & process )
{
	if (! m_LoggedIn )
		return false;

	dword job = createJob();
	send( SERVER_ADD_PROCESS ) << job << process;

	return waitJob( job ) > 0;
}

bool ProcessClient::setProcess( const Process & process )
{
	if (! m_LoggedIn )
		return false;

	dword job = createJob();
	send( SERVER_SET_PROCESS ) << job << process;

	return waitJob( job ) > 0;
}

bool ProcessClient::deleteProcess( dword processId )
{
	if (! m_LoggedIn )
		return false;

	dword job = createJob();
	send( SERVER_DEL_PROCESS ) << job << processId;

	return waitJob( job ) > 0;
}

bool ProcessClient::stopProcess( dword processId )
{
	if (! m_LoggedIn )
		return false;

	dword job = createJob();
	send( SERVER_STOP_PROCESS ) << job << processId;

	return waitJob( job ) > 0;
}

bool ProcessClient::startProcess( dword processId )
{
	if (! m_LoggedIn )
		return false;

	dword job = createJob();
	send( SERVER_START_PROCESS ) << job << processId;

	return waitJob( job ) > 0;
}

bool ProcessClient::restartProcess( dword processId )
{
	if (! m_LoggedIn )
		return false;

	dword job = createJob();
	send( SERVER_RESTART_PROCESS ) << job << processId;

	return waitJob( job ) > 0;
}

bool ProcessClient::getStatus( Status & status )
{
	if (!m_LoggedIn )
		return false;

	dword job = createJob( &status );
	send( SERVER_SEND_STATUS ) << job;

	return waitJob( job ) > 0;
}

bool ProcessClient::stopAll()
{
	if (!m_LoggedIn )
		return false;

	dword job = createJob();
	send( SERVER_STOP_ALL ) << job;

	return waitJob( job ) > 0;
}

bool ProcessClient::startAll()
{
	if (!m_LoggedIn )
		return false;

	dword job = createJob();
	send( SERVER_START_ALL ) << job;

	return waitJob( job ) > 0;
}

bool ProcessClient::restartAll()
{
	if (!m_LoggedIn )
		return false;

	dword job = createJob();
	send( SERVER_RESTART_ALL ) << job;

	return waitJob( job ) > 0;
}

bool ProcessClient::reboot()
{
	if (!m_LoggedIn )
		return false;

	dword job = createJob();
	send( SERVER_REBOOT ) << job;

	return waitJob( job ) > 0;
}

bool ProcessClient::exit()
{
	if (!m_LoggedIn )
		return false;

	dword job = createJob();
	send( SERVER_EXIT ) << job;

	return waitJob( job ) > 0;
}

bool ProcessClient::terminateProcess( dword processId )
{
	if (! m_LoggedIn )
		return false;

	dword job = createJob();
	send( SERVER_TERMINATE_PROCESS ) << job << processId;

	return waitJob( job ) > 0;
}

dword ProcessClient::openLog( dword processId )
{
	if (! m_LoggedIn )
		return 0;

	dword job = createJob();
	send( SERVER_OPEN_LOG ) << job << processId;

	int logId = waitJob( job );

	AutoLock lock( &m_Lock );
	if ( logId > 0 )
		m_Log[ logId ].reset();		// create hash entry

	return logId;
}

bool ProcessClient::isLogValid( dword logId )
{
	return m_Log.find( logId ).valid();
}

void ProcessClient::closeLog( dword logId )
{
	if (! m_LoggedIn )
		return;

	AutoLock lock( &m_Lock );
	
	// remove from hash table
	m_Log.remove( logId );
	// send command to stop log updates
	send( SERVER_CLOSE_LOG ) << logId;
}

bool ProcessClient::popLog( dword logId, CharString & line )
{
	AutoLock lock( &m_Lock );
	if ( m_Log.find( logId ).valid() )
	{
		Queue< CharString > & lines = m_Log[ logId ];
		lines.reset();

		if ( lines.valid() )
		{
			line = *lines;
			lines.pop();

			return true;
		}
	}

	return false;
}

bool ProcessClient::searchLogs( SearchLogRequest req , CharString & result)
{
	if (!m_LoggedIn )
		return false;

	dword job = createJob( &result );
	send( SERVER_SEARCH_LOGS ) << job << req;

	return waitJob( job ) > 0;
}


//----------------------------------------------------------------------------
//EOF
