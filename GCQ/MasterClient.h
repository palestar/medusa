/*
	MasterClient.h
	(c)2000 Palestar Inc, Richard Lyle
*/

#ifndef MASTERCLIENT_H
#define MASTERCLIENT_H

#include "Standard/Event.h"
#include "Network/Client.h"
#include "GCQ/GCQDll.h"



//-------------------------------------------------------------------------------

class DLL MasterClient : public Client
{
public:
	enum Messages
	{
		CLIENT_RECV_PUBLIC	= 0x00,		// Server is sending the public key
		CLIENT_RECV_KEY		= 0x01,		
		SERVER_RECV_KEY		= 0x81,		// Client is requesting authorziation
	};

	// Construction
						MasterClient();
	virtual				~MasterClient();

	// Client interface
	void				onConnect();										// called when connection made
	void				onReceive( byte message, const InStream & );				// called on incoming data
	void				onDisconnect();										// called when connection lost
	
	// Accessors
	bool				connected() const;									// are we connected

	// Mutators
	bool				open();												// connect to the best meta-server
	bool				authorize();										// check if this client is authorized

	// Static
	static dword		VERSION;											// current version of this client

private:
	// Data
	bool				m_Connected;	
	Event				m_PublicKeyEvent;
	CharString			m_PublicKey;
};

//----------------------------------------------------------------------------

inline bool MasterClient::connected() const
{
	return( m_Connected );
}

//----------------------------------------------------------------------------



#endif

//-------------------------------------------------------------------------------
// EOF
