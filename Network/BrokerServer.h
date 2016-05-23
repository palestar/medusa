/*
	BrokerServer.h

	This server provides access to a broker file over a socket
	(c)2005 Palestar, Richard Lyle
*/

#ifndef BROKER_SERVER_H
#define BROKER_SERVER_H

#include "File/Archive.h"
#include "Server.h"
#include "BrokerClient.h"
#include "NetworkDLL.h"

//----------------------------------------------------------------------------

class DLL BrokerServer : public Server
{
public:
	// Construction
	BrokerServer();
	BrokerServer( const char * archiveFile, Socket * pSocket, int serverPort, int maxClients );
	virtual	~BrokerServer();

	// Server interface
	void				onConnect( dword client );	
	void				onReceive( dword client, byte message, const InStream & );
	// Mutators
	bool				start( const char * archiveFile, Socket * pSocket, int serverPort, int maxClients );
	void				stop();

private:
	// Data
	Archive				m_Archive;
};

//----------------------------------------------------------------------------



#endif

//----------------------------------------------------------------------------
// EOF
