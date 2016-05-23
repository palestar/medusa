/*
	MirrorClient.h

	This client object is used to connect to an MirrorServer
	(c)2005 Palestar, Richard Lyle
*/

#ifndef MIRROR_CLIENT2_H
#define MIRROR_CLIENT2_H

#include "Client.h"
#include "Standard/Event.h"
#include "Standard/StringHash.h"
#include "NetworkDLL.h"

//----------------------------------------------------------------------------

class DLL MirrorClient : public Client
{
public:
	// Types
	enum Versions
	{
		VERSION_20040618		= 20040618,
		VERSION_20050508		= 20050508,
		VERSION_20051227		= 20051227,
	};
	enum FilterFlags
	{
		FF_DATE					= 0x1,
	};
	enum ConflictAction
	{
		USE_LOCAL				= 0,
		USE_REMOTE				= 1,
		ABORT					= 2,
	};
	enum Messages
	{
		CLIENT_HELLO			= 0x00,		// SERVER_HELLO
		CLIENT_LOGIN			= 0x01,
		CLIENT_RECV_CRC			= 0x02,		// Server is sending the catalog crc
		CLIENT_RECV_CATALOG		= 0x03,		// Server is sending catalog
		CLIENT_RECV_FILE		= 0x04,		// server is send a file
		CLIENT_FILE_RECEIVED	= 0x05,		// Server has received the file	
		CLIENT_FILE_DELETED		= 0x0b,		// Server has deleted the file
		CLIENT_JOB_DONE			= 0x09,		// Server is finished with a job

		SERVER_HELLO			= 0xfe,		// Client is saying hello
		SERVER_LOGIN			= 0x81,		// Client login
		SERVER_LOGIN_SID		= 0x8b,		// Login with session ID
		SERVER_SEND_CRC			= 0x82,		// Client is requesting the catalog crc
		SERVER_SEND_CATALOG		= 0x83,		// Client requesting catalog
		SERVER_SEND_FILE		= 0x84,		// client is requesting a file
		SERVER_SEND_FILES		= 0x85,		// client is requesting files
		SERVER_RECV_FILE		= 0x86,		// Client is sending a file
		SERVER_DELETE_FILE		= 0x8a,		// delete a file

		HANDSHAKE				= 0xff,
	};
	struct Item
	{
		CharString	name;			// file name
		dword		date;			// last modification date
		dword		crc;			// file time
		dword		size;			// size of file in bytes

		Item &		operator=( const Item & copy );
	};

	typedef Tree< CharString, Item >		Catalog;
	typedef Catalog::Iterator				CatalogIt;

	// Construction
	MirrorClient();
	virtual ~MirrorClient();

	// Client interface
	virtual void	onConnect();												// called when connection made
	virtual void	onReceive( byte message, const InStream & );						// called when widget received from server
	virtual void	onDisconnect();												// called when connection lost

	// Accessors
	dword			serverVersion() const;										// version of the server
	bool			connected() const;											// true if we are connected
	bool			loggedIn() const;											// true if we are logged in

	// Mutators
	bool			open( const char * pAddress, int nPort, 
						const char * pMirrorInto, const char * pCatalog = NULL,
						bool bSelfUpdating = true );							// open connection with server
	bool			login( const char * pUID, const char * pPW );				// login with username and password (optional)
	bool			login( dword nSID );					
	void			close();													// close connection

	dword			syncronize( bool bUpload = false );							// syncronize with remote server, optionally upload local changes
																				// returns the jobId of the last operation

	dword			getCRC();													// get the catalog crc, returns 0 if not available
	dword			getLocalCRC();												// get the crc of our local catalog
	bool			getCatalog();												// get the current file catalog from the server

	dword			getFile( const char * pFile );								// requests specified file from server, returns the job ID
	dword			getFiles( const Array< CharString > & files );				// requests file set from the server, returns the job ID
	dword			putFile( const char * pFile );								// upload the specified file
	dword			putFiles( const Array< CharString > & files );				// upload the specified files, returns the job ID
	dword			deleteFile( const char * pFile );							// delete specified file, returns the job ID
	dword			deleteFiles( const Array< CharString > & files );			// delete specified files
	void			deleteLocalFiles( const Array< CharString > & files );

	//----------------------------------------------------------------------------

	void			setLocalCatalog( const char * pFile );						// set the filename for the local catalog (default: catalog.crc)
	void			loadLocalCatalog();
	void			saveLocalCatalog();

	// Interface
	virtual void	onStatus( const char * pStatus );
	virtual void	onError( const char * pError );
	virtual void	onProgress( dword nTotalBytes, dword nBytes );
	virtual void	onFileProgress( const char * pFile, 
						dword nTotalBytes, dword nBytes );

	virtual int		onFileConflict( const char * pFile );						// see ConflictAction enum
	virtual bool	onFileDownload( const char * pFile );						// false to skip download
	virtual bool	onFileUpload( const char * pFile );							// false to skip upload
	virtual bool	onFileDelete( const char * pFile );							// false to abort local delete
	virtual bool	onFileDeleteRemote( const char * pFile );					// false to abort remote delte

	// Static
	static dword	version();													// get the current client version
	static bool		loadCatalog( const char * pFile, Catalog & catalog );
	static bool		saveCatalog( const char * pFile, const Catalog & catalog );
	static void		buildCatalog( const char * pRoot, 
						const char * pMask, Catalog & catalog,
						bool bUploading );										// searchs for files, updates files in the catalog (add's then if bUploading is true)
	static void		checkCatalog( const char * pRoot,	
						Catalog & catalog,
						bool bUploading );										// checks the catalog, adds (if bUploading is true), updates, and removes files from the catalog 
	static dword	checksum( const void * pData, dword size );
	static dword	checksum( const char * pFile );
	static void		normalizeKey( CharString & a_sKey );

protected:
	// Data
	dword			m_nServerVersion;		// version of the server
	bool			m_bConnected;
	bool			m_bLoggedIn;
	CharString		m_MirrorInto;			// mirror directory
	bool			m_bSelfUpdating;		// true if the client is self-updating code, 
											// this will force all received files to be saved with the .UPD extention

	bool			m_bCatalogValid;		// is the remote catalog valid
	Catalog			m_Catalog;				// catalog from server
	bool			m_bLocalCatalog;		// true if the local catalog is loaded
	bool			m_bLocalDirty;			// true if local catalog has been modified
	Catalog			m_LocalCatalog;			// our local catalog

	volatile dword	m_nTotalBytes;			// total bytes to receive/send
	volatile dword	m_nBytes;				// current bytes received/sent
	dword			m_nGetFilesJob;			// last job created for getFiles()

	CharString		m_sLocalCatalogFile;	// our local catalog filename
};

//----------------------------------------------------------------------------

inline dword MirrorClient::serverVersion() const
{
	return m_nServerVersion;
}

inline bool MirrorClient::connected() const
{
	return m_bConnected;
}

inline bool MirrorClient::loggedIn() const
{
	return m_bLoggedIn;
}

//----------------------------------------------------------------------------

inline const InStream & operator>>(const InStream &input, MirrorClient::Item & item )
{
	input >> item.name;
	if ( (input.filter() & MirrorClient::FF_DATE) == 0 )
		input >> item.date;
	else
		item.date = 0;
	input >> item.crc;
	input >> item.size;
	return input;
}

inline const OutStream & operator<<(const OutStream &output, const MirrorClient::Item & item )
{
	output << item.name;
	if ( (output.filter() & MirrorClient::FF_DATE) == 0 )
		output << item.date;
	output << item.crc;
	output << item.size;
	return output;
}

inline MirrorClient::Item & MirrorClient::Item::operator=( const MirrorClient::Item & copy )
{
	name = copy.name;
	date = copy.date;
	crc = copy.crc;
	size = copy.size;
	return *this;
}

//----------------------------------------------------------------------------



#endif

//----------------------------------------------------------------------------
// EOF
