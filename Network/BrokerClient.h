/*
	BrokerClient.h

	This object is used to connect to a broker server
	(c)2005 Palestar, Richard Lyle
*/

#ifndef BROKER_CLIENT_H
#define BROKER_CLIENT_H

#include "Standard/Event.h"
#include "File/Archive.h"
#include "Factory/Broker.h"
#include "Factory/BrokerArchive.h"
#include "Factory/Factory.h"
#include "Client.h"
#include "NetworkDLL.h"

//-------------------------------------------------------------------------------

const byte PACKAGE_TYPE_REQUEST = 0x1;

//----------------------------------------------------------------------------

class DLL BrokerClient : public Broker, public Client
{
public:
	// Types
	struct Context
	{
		Socket *		socket;
		const char *	serverAddress;
		int				serverPort;
		const char *	localArchive;
	};
	struct File
	{
		WidgetKey		key;		// record key
		dword			version;	// version of record
		dword			size;		// size of record
		bool			autoLoad;	// autoload object
	};

	// Exceptions
	class Failure {};

	// Construction
	BrokerClient();
	BrokerClient( Context & context );
	virtual	~BrokerClient();

	// Broker interface
	Widget *		load( const WidgetKey & key );
	bool			store( Widget * pWidget, dword version, bool autoLoad );
	
	// Mutators
	bool			open( Context & context );
	void			close();

private:
	// Client interface
	void			onConnect();												// called when connection made
	void			onReceive( byte message, const InStream & );						// called when widget received from server
	void			onDisconnect();												// called when connection lost

	// Types
	enum Messages
	{
		CLIENT_RECEIVE_FILES,		// Server is sending the file list
		CLIENT_RECEIVE_WIDGET,		// Server has sent the widget
		SERVER_REQUEST_WIDGET,		// Client requests the server send a widget
	};

	typedef Hash< WidgetKey , int >		FileHash;
	typedef FileHash::Iterator			FileHashIt;

	// Data
	Array< File >	m_Files;			// array of all files on the BrokerServer
	FileHash		m_FileHash;			// file hash
	Archive			m_Archive;			// local archive
	Event			m_Received;			// received widget event

	// Mutators
	void			registerFiles();
	void			unregisterFiles();

	// Friends
	friend class BrokerServer;
};

IMPLEMENT_RAW_STREAMING( BrokerClient::File );

//----------------------------------------------------------------------------



#endif

//----------------------------------------------------------------------------
// EOF
