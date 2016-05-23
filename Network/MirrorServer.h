/*
	MirrorServer.h

	This is the server object for MirrorClient. 
	(c)2005 Palestar, Richard Lyle
*/

#ifndef MIRROR_SERVER_H
#define MIRROR_SERVER_H

#include "File/FileMemory.h"
#include "File/FileChange.h"
#include "Standard/SortedArray.h"
#include "Server.h"
#include "MirrorClient.h"
#include "NetworkDLL.h"

//----------------------------------------------------------------------------

class DLL MirrorServer : public Server
{
public:
	// Types
	typedef MirrorClient::Item			Item;
	typedef MirrorClient::Catalog		Catalog;
	typedef MirrorClient::CatalogIt		CatalogIt;

	struct Link
	{
		CharString	sCatalog;
		CharString	sAddress;
		int			nPort;
		bool		bUpdate;

		Link &		operator=( const Link & copy );
	};

	struct Context
	{
		CharString	sCatalog;			// file containing our catalog checksums
		CharString	mirror;				// mirror directory
		CharString	logFile;			// log file
		CharString	address;			// my external address
		int			port;				// port to accept connections on
		int			maxClients;			// maximum number of clients
		bool		bVersionControl;	// enable version control
		int			nMinUpdateTime;		// minimum number of seconds between catalog updates

		CharString	sUID;				// user name and password
		CharString	sPW;

		Array< Link >
					links;				// server to server links

		Context &	operator=(const Context & copy );
	};

	// Construction
	MirrorServer();
	MirrorServer( Context & context );
	virtual	~MirrorServer();

	// Server interface
	void				onConnect( dword client );	
	void				onReceive( dword client, byte message, const InStream & );
	void				onDisconnect( dword client );

	// Accessors
	const Context &		context() const;

	// Mutators
	bool				start( Context & context );
	void				stop();

	virtual void		onUpdate();										// called once per second

	// security layer interface 
	virtual bool		login( dword client, 
							const char * pUser, 
							const char * pMD5 );						// returns true if user login is successful
	virtual bool		login( dword client,
							dword nSID );					
	virtual void		logout( dword client );							// releases the userId

	virtual	bool		canDownload( dword nClientId );					// can the user download files
	virtual bool		canUpload( dword nClientId );					// can the user upload files
	virtual bool		canDelete( dword nClientId );					// can the user delete/undelete files

protected:
	// Data
	Context				m_Context;

	bool				m_bCatalogReady;
	Catalog				m_Catalog;		// file catalog
	FileChange			m_FileChange;

	Hash< dword, dword >
						m_ClientVersion;

	//----------------------------------------------------------------------------

	class UpdateThread : public SafeThread	
	{
	public:
		// Construction
					UpdateThread( MirrorServer * pServer );
		// Thread interface
		int			run();
	private:
		MirrorServer *	m_pServer;
	};
	friend class UpdateThread;

	UpdateThread *		m_pUpdateThread;

	void				updateDemon();
	void				updateCatalog();

	dword				clientVersion( dword nClientId );
	bool				receiveFile( dword nClientId, const Item & item, const byte * pBuffer );
	bool				sendFile( dword nClientId, const Item & item );
	bool				deleteFile( dword nClientId, const char * pFile );
};

//----------------------------------------------------------------------------

inline const MirrorServer::Context & MirrorServer::context() const
{
	return m_Context;
}

//----------------------------------------------------------------------------

inline MirrorServer::Link & MirrorServer::Link::operator=( const Link & copy )
{
	sCatalog = copy.sCatalog;
	sAddress = copy.sAddress;
	nPort = copy.nPort;
	bUpdate = copy.bUpdate;
	return *this;
}

inline MirrorServer::Context & MirrorServer::Context::operator=(const Context & copy )
{
	sCatalog = copy.sCatalog;
	mirror = copy.mirror;
	logFile = copy.logFile;
	address = copy.address;
	port = copy.port;
	maxClients = copy.maxClients;
	bVersionControl = copy.bVersionControl;
	nMinUpdateTime = copy.nMinUpdateTime;
	sUID = copy.sUID;
	sPW = copy.sPW;
	links = copy.links;
	return *this;
}

//----------------------------------------------------------------------------



#endif

//----------------------------------------------------------------------------
// EOF
