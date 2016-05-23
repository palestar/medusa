/*
	Server.h

	This object is used to create a server for several client objects
	(c)2005 Palestar, Richard Lyle
*/

#include "Debug/Assert.h"
#include "Debug/Trace.h"
#include "File/Stream.h"
#include "Standard/Time.h"
#include "Standard/Atomic.h"

#include "FileSocket.h"
#include "Client.h"
#include "Server.h"

#include <time.h>

//----------------------------------------------------------------------------

Server::Server() : 
	m_bRunning( false ), 
	m_Port( -1 ),
	m_MaxClients( -1 ), 
	m_NextClientId( 1 ),
	m_ServerTime( 0 ), 
	m_pDictionary( new Dictionary() ),
	m_pSocketList( new SocketList() )
{}

Server::Server( Socket *pSocket, int serverPort, int maxClients ) : 
	m_bRunning( false ), 
	m_Port( -1 ), 
	m_MaxClients( -1 ), 
	m_NextClientId( 1 ),
	m_ServerTime( 0 ), 
	m_pDictionary( new Dictionary() ),
	m_pSocketList( new SocketList() )
{
	if (! start( pSocket, serverPort, maxClients ) )
		throw Failure();
}

Server::~Server()
{
	stop();
}

//----------------------------------------------------------------------------

bool Server::start( Socket * pSocket, int serverPort, int maxClients )
{
	stop();

	m_Socket = pSocket;
	m_Port = serverPort;
	m_MaxClients = maxClients;
	m_NextClientId = 1;
	m_ServerTime = Time::seconds();

	if (! m_Socket->listen( m_Port ) )
		return false;
	if (! m_pSocketList->add( m_Socket ) )
		return false;

	m_bRunning = true;

	return true;
}

bool Server::update()
{
	AutoLock lock( &m_Lock );
	// update the sockets, and handle any events..
	dword nUpdateFlags = m_pSocketList->update();

	if ( (nUpdateFlags & SocketList::EVENT_READY) != 0 )
	{
		Socket::Ref pRead = m_pSocketList->dataReady();
		while( pRead.valid() )
		{
			dword nClientId = socketClient( pRead );
			if ( nClientId != 0 )
			{
				InStream input( clientFileSocket( nClientId ), FF_TRANSMIT, SV_CURRENT, m_pDictionary );

				byte nMessageID = 0;
				while( pRead->receivedBytes() > 0 )
				{
					try {
						// get the package type
						input >> nMessageID;
						// handle incoming package
						onReceive( nClientId, nMessageID, input );
					}
					catch( FileSocket::FileError )
					{
						LOG_DEBUG_HIGH( "Server", "Client %u caught error on message 0x%.2x.", nClientId, nMessageID );
						break;
					}
				}
			}

			// next socket with data to read..
			pRead = m_pSocketList->dataReady();
		}
	}

	if ( (nUpdateFlags & SocketList::EVENT_ACCEPTED) != 0 )
	{
		Socket::Ref pAccept = m_pSocketList->accepted();
		while( pAccept.valid() )
		{
			bool bReject = false;

			// new connection
			if ( m_Clients.size() < m_MaxClients )
			{
				LOG_DEBUG_LOW("Server", "listenDemon(), connecting new client");

				dword nClientId = addClient( pAccept );
				if ( nClientId != 0 )
					onConnect( nClientId );
				else
					bReject = true;
			}
			else
			{
				LOG_DEBUG_LOW( "Server", "Maximum number of clients, rejecting new connection." );
				bReject = true;
			}

			if ( bReject )
			{
				// too many clients, must close the connection
				onReject( pAccept );
				pAccept->close();

				LOG_DEBUG_HIGH("Server", "listenDemon(), rejecting new client");
			}

			// next incoming connection..
			pAccept = m_pSocketList->accepted();
		}
	}

	if ( (nUpdateFlags & SocketList::EVENT_CLOSED) != 0 )
	{
		Socket::Ref pClosed = m_pSocketList->closed();
		while( pClosed.valid() )
		{
			dword nClientId = socketClient( pClosed );
			LOG_DEBUG_HIGH("Server", "Client %u attempt disconnect from %s.", nClientId, pClosed->peerAddress() );			
			if ( nClientId != 0 )
				disconnect( nClientId );

			pClosed = m_pSocketList->closed();
		}
	}

	return m_bRunning;
}

void Server::stop()
{
	AutoLock lock( &m_Lock );
	if ( m_bRunning )
	{
		// stop listening for connections
		m_Socket->close();

		// close all the client sockets
		for(int i=0;i<m_Clients.size();i++)
			m_ClientSocket[ m_Clients[i] ]->close();

		m_ClientFileSocket.release();
		m_ClientSocket.release();
		m_pSocketList->release();

		m_bRunning = false;
	}
}

OutStream Server::send( dword nClientId, byte message )
{
	return OutStream( clientFileSocket( nClientId ), &message, sizeof(byte), FF_TRANSMIT, SV_CURRENT, m_pDictionary );
}

AutoLock Server::autoLock()
{
	return AutoLock( &m_Lock );
}

void Server::lock()
{
	m_Lock.lock();
}

void Server::unlock()
{
	m_Lock.unlock();
}

dword Server::connect( Socket * pSocket, const char * pAddress, int port )
{
	if ( pSocket->connect( pAddress, port ) )
		return addClient( pSocket );

	// failed to connect
	return 0;
}

void Server::disconnect( dword client )
{
	// notify any subclass
	onDisconnect( client );
	// remove this client from the server
	removeClient( client );
}

//----------------------------------------------------------------------------

void Server::onConnect( dword nClientId )
{}

void Server::onReceive( dword nClientId, byte message, const InStream & input )
{}

void Server::onDisconnect( dword nClientId )
{}

void Server::onReject( Socket * pConnection )
{}

//---------------------------------------------------------------------------------------------------

dword Server::addClient( Socket * pSocket )
{
	AutoLock lock( &m_Lock );
	if ( m_NextClientId >= 0xffffffff )
	{
		LOG_ERROR( "Server", "ClientId's exhausted." );
		return 0;		// exhausted all client ID's!
	}
	if ( ! m_pSocketList->add( pSocket ) )
	{
		LOG_ERROR( "Server", "SocketList is full." );
		return 0;		// no more space in the socket list!
	}

	dword nClientId = m_NextClientId++;

	m_Clients.push( nClientId );
	m_ClientSocket[ nClientId ] = pSocket;
	m_SocketClient[ pSocket->instanceID() ] = nClientId;
	m_ClientFileSocket[ nClientId ] = new FileSocket( pSocket );

	return nClientId;
}

void Server::removeClient( dword nClientId )
{
	AutoLock lock( &m_Lock );

	Socket::Ref pSocket = clientSocket( nClientId );
	if ( pSocket.valid() )
	{
		m_ClientSocket.remove( nClientId );
		m_SocketClient.remove( pSocket->instanceID() );
		m_ClientFileSocket.remove( nClientId );
		m_Clients.removeSearch( nClientId );
		m_pSocketList->remove( pSocket );

		pSocket->close();
	}
}

//----------------------------------------------------------------------------
// EOF
