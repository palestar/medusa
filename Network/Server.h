/*
	Server.h

	This object is used to create a server for several client objects
	(c)2005 Palestar, Richard Lyle
*/

#ifndef SERVER_H
#define SERVER_H

#include "Debug/SafeThread.h"
#include "Standard/Types.h"
#include "Standard/Mutex.h"
#include "Standard/Event.h"
#include "Standard/Time.h"
#include "File/Dictionary.h"
#include "File/Stream.h"
#include "Factory/Factory.h"

#include "Socket.h"
#include "SocketList.h"
#include "FileSocket.h"

#include "NetworkDLL.h"		// include last always

//----------------------------------------------------------------------------

/*
	Notes:

	* The server starts on creation, and is stopped when destroyed. 
	* The client should create a class which inherits from this Server class and implement the server
	functionality. 
*/

class DLL Server
{
public:
	// Exceptions
	class Failure {};

	// Construction
	Server();
	Server( Socket *pSocket, int serverPort, int maxClients );
	virtual	~Server();

	// Accessors
	bool				running() const;									// is server running

	Socket *			socket() const;
	int					port() const;
	int					maxClients() const;
	Dictionary *		dictionary() const;

	int					clientCount() const;								// number of connected clients
	dword				client( int n ) const;								// client id

	Socket::Ref			clientSocket( dword client ) const;					// get client network device
	dword				socketClient( Socket * pSocket ) const;				// get a client ID for the given socket
	FileSocket::Ref		clientFileSocket( dword nClientId ) const;			// get the clients FileSocket object
	const char *		clientAddress( dword client ) const;
	int					clientPort( dword client ) const;

	dword				bytesReceived() const;								// Statistics
	dword				bytesSent() const;
	dword				serverTime() const;									// how long in seconds has the sever been running

	// Mutators
	bool				start( Socket * pSocket, int serverPort, int maxClients );
	bool				update();											// accept connections & process incoming client messages
	void				stop();

	OutStream			send( dword client, byte message );					// begin sending data to client

	AutoLock			autoLock();	
	void				lock();												// lock the server
	void				unlock();											// unlock the server

	dword				connect( Socket * pSocket, 
							const char * pAddress, int port );
	void				disconnect( dword client );

	virtual void		onConnect( dword client );							// called when connection made
	virtual void		onReceive( dword client, byte message, const InStream & );	// called when receiving from client
	virtual void		onDisconnect( dword client );						// called when connection lost
	virtual void		onReject( Socket * pConnection );					// called before rejected connection is closed

protected:

	dword				addClient( Socket * pSocket );
	void				removeClient( dword client );						// remove a client

	//----------------------------------------------------------------------------

	typedef Hash< dword, Socket::Ref >		SocketHash;
	typedef Hash< dword, FileSocket::Ref >	FileSocketHash;
	typedef Hash< dword, dword >			SocketClientHash;

	//----------------------------------------------------------------------------

	// Data
	bool				m_bRunning;
	Socket::Ref			m_Socket;
	int					m_Port;
	int					m_MaxClients;
	Dictionary::Ref		m_pDictionary;
	SocketList::Ref		m_pSocketList;

	Array< dword >		m_Clients;
	SocketHash			m_ClientSocket;
	FileSocketHash		m_ClientFileSocket;
	SocketClientHash	m_SocketClient;

	Mutex				m_Lock;

	dword				m_NextClientId;
	dword				m_ServerTime;
};

//----------------------------------------------------------------------------

inline bool Server::running() const
{
	return( m_bRunning );
}

inline Socket * Server::socket() const
{
	return( m_Socket );
}

inline int Server::port() const
{
	return( m_Port );
}

inline int Server::maxClients() const
{
	return( m_MaxClients );
}

inline Dictionary * Server::dictionary() const
{
	return m_pDictionary;
}

inline int	Server::clientCount() const
{
	return( m_Clients.size() );
}

inline dword Server::client( int n ) const
{
	return( m_Clients[n] );
}

inline Socket::Ref Server::clientSocket( dword nClientId ) const
{
	SocketHash::Iterator it = m_ClientSocket.find( nClientId );
	if ( it.valid() )
		return *it;

	return NULL;
}

inline dword Server::socketClient( Socket * pSocket ) const
{
	SocketClientHash::Iterator iClient = m_SocketClient.find( pSocket->instanceID() );
	if ( iClient.valid() )
		return *iClient;

	return 0;
}

inline FileSocket::Ref Server::clientFileSocket( dword nClientId ) const
{
	FileSocketHash::Iterator it = m_ClientFileSocket.find( nClientId );
	if ( it.valid() )
		return *it;

	return NULL;

}

inline const char * Server::clientAddress( dword nClientId ) const
{
	if ( m_ClientSocket.find( nClientId ).valid() )
		return( clientSocket( nClientId )->peerAddress() );
	return "?";
}

inline int Server::clientPort( dword nClientId ) const
{
	if ( m_ClientSocket.find( nClientId ).valid() )
		return( clientSocket( nClientId )->peerPort() );
	return 0;
}

inline dword Server::bytesReceived() const
{
	return m_pSocketList->bytesReceived();
}

inline dword Server::bytesSent() const
{
	return m_pSocketList->bytesSent();
}

inline dword Server::serverTime() const
{
	return Time::seconds() - m_ServerTime;
}

//----------------------------------------------------------------------------

#endif

//----------------------------------------------------------------------------
// EOF
