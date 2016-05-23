/*
	LogClient.h

	This client object is used to connect to a LogServer object. You use the LogClient/LogServer objects to 
	log server information, stats, events over a network connection instead of logging data to a local data file
	that may not be accessable to admins/moderators in a networked game.

	The LogServer object by default stores the log information into a flat file on the machine it's running, however
	it provides for virtuals to allow logging information to be stored by other means as well.

	(c)2005 Palestar Inc, Richard Lyle
*/

#ifndef LOGCLIENT_H
#define LOGCLIENT_H

#include "Client.h"
#include "Debug/Log.h"

#include "NetworkDLL.h"

//----------------------------------------------------------------------------

class DLL LogClient : public Client, public Logging::Reactor
{
public:
	enum Messages 
	{
		CLIENT_LOGIN,			// send by the server to the client containing login status
		SERVER_LOGIN,			// sent by the client to login
		SERVER_RECV_LOG,		// sent by the client to record a log entry
	};

	// Construction
	LogClient();

	//! Logging::Reactor interface
	virtual void	onReport( const Logging::Report & a_Report );

	//! Client interface
	void			onConnect();									// called when connection made
	void			onReceive( byte message, const InStream & );			// called on incoming data
	void			onDisconnect();									// called when connection lost

	// Accessors
	const char *	name() const;									// our name
	dword			sessionId() const;								// our sessionId
	const char *	address() const;								// address of the log server
	int				port() const;									// port for the log server

	int				logLevel() const;								// get the current log level

	// Mutators
	bool			open( const char * pName, dword sessionId, 
						const char * pAddress, int nPort );
	bool			close();
	
private:
	//! Data
	bool			m_bActive;			// true if client is active
	bool			m_bConnected;		// true if we are connected
	bool			m_bLoggedIn;		// true if we are currently logged in

	CharString		m_sAddress;			// server address
	int				m_nPort;			// server port
	CharString		m_sName;			// our name
	dword			m_nSessionId;		// our sessionId

	unsigned int	m_nLogLevel;		// current log level
};

//----------------------------------------------------------------------------

inline const char * LogClient::name() const
{
	return m_sName;
}

inline dword LogClient::sessionId() const
{
	return m_nSessionId;
}

inline const char * LogClient::address() const
{
	return m_sAddress;
}

inline int LogClient::port() const
{
	return m_nPort;
}

inline int LogClient::logLevel() const
{
	return m_nLogLevel;
}

//----------------------------------------------------------------------------

#endif

//----------------------------------------------------------------------------
//EOF
