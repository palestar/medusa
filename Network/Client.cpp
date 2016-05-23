/*
	Client.cpp

	This object is used by a client program to connect to a Server object
	(c)2005 Palestar, Richard Lyle
*/

#include "Debug/Trace.h"
#include "Debug/Assert.h"
#include "Debug/Profile.h"

#include "Client.h"

//----------------------------------------------------------------------------

Client::Client() : 
	m_bConnecting( false ),
	m_bConnected( false ),
	m_nPort( 0 ), 
	m_pDictionary( new Dictionary() ),
	m_NextJobId( 1 )
{}

Client::Client( Socket *pSocket, const char * a_pAddress, int a_nPort ) : 
	m_bConnecting( false ),
	m_bConnected( false ),
	m_nPort( 0 ),
	m_pDictionary( new Dictionary() ),
	m_NextJobId( 1 )
{
	if (! open( pSocket, a_pAddress, a_nPort ) )
		throw Failure();
}

Client::~Client()
{
	close();
}

//----------------------------------------------------------------------------

Socket * Client::socket() const
{
	return m_pSocket;
}

//----------------------------------------------------------------------------

bool Client::open( Socket::Ref a_pSocket, const char * a_pAddress, int a_nPort )
{
	AutoLock lock( &m_Lock );
	close();

	if ( !a_pSocket || !a_pAddress || strlen( a_pAddress ) == 0 )
	{
		LOG_ERROR( "Client", "open() called with invalid socket or address!" );
		return false;
	}

	m_pSocket = a_pSocket;
	m_sAddress = a_pAddress;
	m_nPort = a_nPort;

	if (! m_pSocket->connect( a_pAddress, a_nPort ) )
	{
		LOG_DEBUG_MED( "Client", "connect() returned failure." );
		return false;
	}

	m_pFileSocket = new FileSocket( m_pSocket );
	m_bConnecting = true;
	return true;
}

bool Client::update()
{
	AutoLock lock( &m_Lock );
	if (! m_pSocket.valid() )
		return false;

	if ( m_bConnecting )
	{
		switch( m_pSocket->update() )
		{
		case Socket::SS_CONNECTED:
			m_bConnected = true;
			m_bConnecting = false;
			onConnect();
			break;
		case Socket::SS_CONNECTING:
			return true;
		case Socket::SS_DISCONNECTED:
			m_bConnecting = false;
			return false;
		}
	}

	if (! m_bConnected )
		return false;

	if (! m_pSocket->update() )
	{
		close();
		return false;
	}

	// process any incoming messages..
	if ( m_pSocket->receivedBytes() > 0 )
	{
		byte nMessageID = 0;

		// create the input stream
		InStream input( m_pFileSocket, FF_TRANSMIT, SV_CURRENT, m_pDictionary );
		while( m_pSocket->receivedBytes() > 0 )
		{
			try {
				input >> nMessageID;
				onReceive( nMessageID, input );
			}
			catch( FileSocket::FileError )
			{
				LOG_DEBUG_HIGH( "Client", "Socket error on message 0x%.2x", nMessageID );
				break;
			}
		}
	}

	return true;
}

bool Client::reconnect()
{
	return open( socket(), m_sAddress, m_nPort );
}

void Client::close()
{
	AutoLock lock( &m_Lock );
	if ( m_bConnected )
	{
		m_pSocket->close();
		cancelAllJobs();

		m_bConnected = false;
		onDisconnect();
	}
}

void Client::setSocket( Socket * pSocket )
{
	AutoLock lock( &m_Lock );
	m_pSocket = pSocket;
	m_pFileSocket = new FileSocket( m_pSocket );
	m_bConnected = m_pSocket->state() == Socket::SS_CONNECTED;
	m_bConnecting = m_pSocket->state() == Socket::SS_CONNECTING;
}

OutStream Client::send( byte message )
{
	return OutStream( m_pFileSocket, &message, sizeof(byte), FF_TRANSMIT, SV_CURRENT, m_pDictionary );
}

AutoLock Client::autoLock()
{
	return AutoLock( &m_Lock );
}

void Client::lock()
{
	m_Lock.lock();
}

void Client::unlock()
{
	m_Lock.unlock();
}

//----------------------------------------------------------------------------

void Client::onConnect()
{}

void Client::onReceive( byte message, const InStream & input )
{}

void Client::onDisconnect()
{}

//----------------------------------------------------------------------------

dword Client::createJob()
{
	AutoLock lock( &m_Lock );
	dword job = m_NextJobId++;
	m_Jobs[ job ] = Job();

	return job;
}

dword Client::createJob( void * pResult )
{
	AutoLock lock( &m_Lock );
	dword job = m_NextJobId++;
	m_Jobs[ job ] = Job( pResult );

	return job;
}

void * Client::jobResult( dword job )
{
	void * pResult = NULL;

	AutoLock lock( &m_Lock );

	JobMap::iterator iJob = m_Jobs.find( job );
	if ( iJob != m_Jobs.end() )
		pResult = iJob->second.m_pResult;

	return pResult;
}

int Client::checkJob( dword job )
{
	AutoLock lock( &m_Lock );

	JobMap::iterator iJob = m_Jobs.find( job );
	if ( iJob == m_Jobs.end() )
		return -1;		// not a valid job!

	// check the state of the event
	return iJob->second.m_Event.signaled() ? 1 : 0;
}

int Client::waitJob( dword job, int timeout /*= TIMEOUT*/ )
{
	AutoLock lock( &m_Lock );
	JobMap::iterator iJob = m_Jobs.find( job );
	if ( iJob == m_Jobs.end() )
		return -1;		// not a valid job or job gone.

	do {
		if ( iJob->second.m_Event.signaled() )
		{
			int nStatus = iJob->second.m_nStatus;

			m_Jobs.erase(iJob );
			return nStatus;
		}

		if (! update() )
		{
			LOG_DEBUG_LOW( "Client", "connection lost, waitJob() cancelled." );
			break;
		}

		if ( timeout > 0 )
		{
			lock.release();

			Thread::sleep( 10 );
			timeout -= 10;

			lock.set( &m_Lock );
		}

	} while( timeout > 0 );

	m_Jobs.erase( job );
	return -1;
}

void Client::cancelJob( dword job )
{
	// signal the job complete
	signalJob( job, -1 );
	// clean up the job
	waitJob( job, 0 );
}

bool Client::signalJob( dword job, int status )
{
	AutoLock lock( &m_Lock );

	JobMap::iterator iJob = m_Jobs.find( job );
	if ( iJob == m_Jobs.end() )
		return false;

	iJob->second.m_nStatus = status;
	iJob->second.m_Event.signal();
	return true;
}

void Client::cancelAllJobs()
{
	AutoLock lock( &m_Lock );

	m_Jobs.clear();
	m_NextJobId = 1;
}


//----------------------------------------------------------------------------
// EOF


