/*
	LogServer.h
	(c)2005 Palestar Inc, Richard Lyle
*/

#ifndef LOGSERVER_H
#define LOGSERVER_H

#include "LogClient.h"
#include "Server.h"
#include "NetworkDLL.h"

//----------------------------------------------------------------------------

class DLL LogServer : public Server
{
public:
	// Construction
	LogServer();

	// Server2 interface
	void			onConnect( dword client );							// called when connection made
	void			onReceive( dword client, byte message, const InStream & );	// called when receiving from client
	void			onDisconnect( dword client );						// called when connection lost

	// Mutators
	bool			start( unsigned int nLogLevel, 
						int nPort, int nMaxClients );					// start this server
	bool			stop();												// stop the server

	virtual bool	onLogin( dword nClientId, dword nSessionId, 
						const char * pName );							// called when client is logging in, return true to allow

private:
	// Types
	struct ClientContext
	{
		dword		nSessionId;
		CharString	sName;
		bool		bLoggedIn;

		ClientContext & operator=( const ClientContext & copy );
	};
	typedef Hash< dword, ClientContext >		ClientContextHash;

	// Data
	CharString			m_sLogFile;
	unsigned int		m_nLogLevel;
	ClientContextHash	m_hClientContext;
};

//----------------------------------------------------------------------------

inline LogServer::ClientContext & LogServer::ClientContext::operator=( const ClientContext & copy )
{
	nSessionId = copy.nSessionId;
	sName = copy.sName;
	bLoggedIn = copy.bLoggedIn;
	return *this;
}

//----------------------------------------------------------------------------




#endif

//----------------------------------------------------------------------------
//EOF
