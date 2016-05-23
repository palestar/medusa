/*
	Client.h

	This object is used by a client program to connect to a Server object
	(c)2005 Palestar, Richard Lyle
*/

#ifndef CLIENT_H
#define CLIENT_H

#include "Standard/CriticalSection.h"
#include "Standard/Event.h"
#include "File/Dictionary.h"
#include "File/Stream.h"

#include "FileSocket.h"
#include "Socket.h"

#include <map>

#include "NetworkDLL.h"

//-------------------------------------------------------------------------------

/*
	Notes:

	* Derive your own class from this client to implement your clients functionality.
	* On creation this object connects to the server, when the connection is made a 
	thread is created which waits for incoming data from the server. When data is received
	onReceive is called with the received Widget, you are responsible for deleting the Widget.
	* send() transmits the object to the server.
	* onConnect(), onReceive() and onDisconnect() will be called outside the main thread.
*/

class DLL Client
{
public:
	// Exceptions
	class Failure {};
	class JobFailure {};

	// Constants
	enum {
		DEFAULT_TIMEOUT = 30 * 1000,
	};

	// Construction	
	Client();
	Client( Socket *pDevice, const char * pServerAddress, int serverPort );
	virtual ~Client();


	// Accessors
	const char *		address() const;
	int					port() const;
	Socket *			socket() const;
	Dictionary *		dictionary() const;

	// Mutators
	bool				open( Socket::Ref a_pSocket, const char * a_pAddress, 
							int a_nPort );
	bool				update();											// update this client,  returns false if connection is closed.
	bool				reconnect();
	void				close();
	
	void				setSocket( Socket * pSocket );

	OutStream			send( byte message );								// begin sending to server

	AutoLock			autoLock();	
	void				lock();
	void				unlock();

	virtual void		onConnect();										// called when connection made
	virtual void		onReceive( byte message, const InStream & );				// called on incoming data
	virtual void		onDisconnect();										// called when connection lost

	dword				createJob();
	dword				createJob( void * pResult );
	void *				jobResult( dword job );

	int					checkJob( dword job );									// returns 1 if job is completed, 0 if job is pending, -1 for an invalid job
	int					waitJob( dword job, int timeout = DEFAULT_TIMEOUT );	// returns value passed to signalJob, -1 on timeout
	void				cancelJob( dword job );
	bool				signalJob( dword job, int status );						// signal job is done, int is returned by waitJob()
	void				cancelAllJobs();


protected:
	//! Types
	struct Job {
		Job() : m_pResult( NULL ), m_nStatus( 0 )
		{}
		Job( void * a_pResult ) : m_pResult( a_pResult ), m_nStatus( 0 )
		{}
		Job( const Job & a_Copy ) : m_pResult( a_Copy.m_pResult ), m_nStatus( a_Copy.m_nStatus )
		{}

		~Job()
		{}

		Event 			m_Event;
		void *			m_pResult;
		int				m_nStatus;
	};
	typedef std::map< dword, Job >		JobMap;

	//! Data
	bool				m_bConnecting;
	bool				m_bConnected;
	CharString			m_sAddress;
	int					m_nPort;
	Socket::Ref			m_pSocket;
	FileSocket::Ref		m_pFileSocket;
	Dictionary::Ref		m_pDictionary;
	CriticalSection		m_Lock;

	dword				m_NextJobId;
	JobMap				m_Jobs;
};

//----------------------------------------------------------------------------

inline const char * Client::address() const
{
	return m_sAddress;
}

inline int Client::port() const
{
	return m_nPort;
}

inline Dictionary * Client::dictionary() const
{
	return m_pDictionary;
}

#endif

//----------------------------------------------------------------------------
// EOF


