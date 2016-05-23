/*
	MasterClient.cpp
	(c)2000 Palestar Inc, Richard Lyle
*/

#include "Standard/MD5.h"
#include "GCQ/MasterClient.h"

//----------------------------------------------------------------------------

dword MasterClient::VERSION = 10000;	

//----------------------------------------------------------------------------

MasterClient::MasterClient() : m_Connected( false )
{}

MasterClient::~MasterClient()
{}

//----------------------------------------------------------------------------

void MasterClient::onConnect()
{
	m_Connected = true;
}

void MasterClient::onReceive( byte message, const InStream & input )
{
	switch( message )
	{
	case CLIENT_RECV_PUBLIC:
		{
			CharString key;
			input >> key;

			AutoLock lock( autoLock() );
			m_PublicKey = key;
			m_PublicKeyEvent.signal();
		}
		break;
	case CLIENT_RECV_KEY:
		{
			dword job;
			input >> job;
			dword version;
			input >> version;
			int jobDone;
			input >> jobDone;

			AutoLock lock( autoLock() );
			if ( version == VERSION )
				signalJob( job, jobDone );
			else
				signalJob( job, -1 );		// not authorized
		}
		break;
	}
}

void MasterClient::onDisconnect()
{
	m_Connected = false;
}

//----------------------------------------------------------------------------

bool MasterClient::open()
{
	m_Connected = false;
	m_PublicKeyEvent.clear();

	// open connection to server
	if (! Client::open( new Socket("ZLIB"), "master-server.palestar.com", 8999 ) )
		return false;
	if ( m_PublicKeyEvent.wait( 30000 ) )
		return false;

	return true;
}

bool MasterClient::authorize()
{
	if (! m_Connected ) 
		if ( !open() )
			return false;

	AutoLock lock( autoLock() );
	CharString myAddress( socket()->address() );
	CharString peerAddress( socket()->peerAddress() );
	CharString key( MD5( CharString(m_PublicKey + myAddress + peerAddress) ).checksum() );
	
	dword job = createJob();
	send( SERVER_RECV_KEY ) << job << VERSION << key;

	lock.release();

	return waitJob( job ) > 0;
}

//----------------------------------------------------------------------------
//EOF
