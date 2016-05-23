// Server2_Test.cpp : Defines the entry point for the console application.
//

/*
	****************** IMPORTANT NOTE ****************

	* This test eventually dead-locks when running under a debugger, both release and debug versions. 
	If you run this test program outside the debugger, it runs without a dead-lock.
*/

#include "stdafx.h"
#include <stdio.h>

#include "Debug/Assert.h"
#include "Network/Server.h"
#include "Network/Client.h"
#include "Network/FileSocket.h"
#include "Standard/Time.h"
#include "Standard/Test.h"

const char * SERVER_ADDRESS = "localhost";
const int	 SERVER_PORT = 9010;

//----------------------------------------------------------------------------

class TestServer : public Server
{
public:
	// Construction
	TestServer()
	{}

	// Server interface
	void onConnect( dword client )
	{
		printf( "TestServer::onConnect %u / %u\n", client, clientCount() );

		//TRACE( String().format("TestServer::onConnect(), client = %u, address = %s", client, clientAddress( client ) ) );
		if ( (rand() % 100) > 75 )
		{
			printf( "TestServer::onConnect, rejecting client %u\n", client );

			removeClient( client );
			send( client, 2 ) << String("This is a test" );
		}
	}
	void onReceive( dword client, byte message, InStream & input )
	{
		//TRACE( String().format("TestServer::onReceive(), client = %u", client ) );

		switch( message )
		{
		case 0:
			{
				int size = 512;
				byte * pBytes = new byte[ size ];

				try {
					input.read( pBytes, size );
				}
				catch( FileSocket::FileError )
				{
					//ASSERT( 0 );
				}

				delete [] pBytes;

				send( client, 1 );
			}
			break;
		}

	}
	void onDisconnect( dword client )
	{
		//printf( "TestServer::onDisconnect %u\n", client );
		//TRACE( String().format("TestServer::onDisconnect(), client = %u", client) );

		send( client, 2 ) << String("This is a test" );
	}
	void onReject( Socket * pConnection )
	{
		//TRACE( "TestServer::onReject()" );
	}

	// Mutators
	bool start()
	{
		return Server::start( Socket::create( "SocketWSC" ), SERVER_PORT, 128 );
	}
};

//----------------------------------------------------------------------------

class TestClient : public Client
{
public:
	// Construction
	TestClient() : m_Connected( false ), m_nThreadId( 0 ), m_nClientId( 0 )
	{}

	int		m_nThreadId;
	int		m_nClientId;

	// Client interface
	void onConnect()
	{
		printf( "TestClient::onConnect %d / %d\n", m_nThreadId, m_nClientId );

		m_Connected = true;

		int size = 512;
		byte * pBytes = new byte[ size ];
		for(int i=0;i<size;i++)
			pBytes[i] = rand();

		send( 0x0 ).write( pBytes, size );

		delete [] pBytes;
	}

	void onReceive( byte message, InStream & input )
	{
		switch( message )
		{
		case 1:
			close();
			break;
		}
	}

	void onDisconnect()
	{
		printf( "TestClient::onDisconnect %d / %d\n", m_nThreadId, m_nClientId );

		//TRACE( "TestClient::onDisconnect()" );
		m_Connected = false;
	}

	// Accessors
	bool connected() const
	{
		return m_Connected;
	}

	// Mutators
	bool open()
	{
		printf( "TestClient::open %d / %d\n", m_nThreadId, m_nClientId );
		return Client::open( Socket::create( "SocketWSC" ), SERVER_ADDRESS, SERVER_PORT );
	}
	void close()
	{
		printf( "TestClient::close %d / %d\n", m_nThreadId, m_nClientId );
		Client::close();
	}
	
private:
	// Data
	bool		m_Connected;
};

//----------------------------------------------------------------------------

const int CLIENT_COUNT = 4;
const int TEST_COUNT = 2000000;

class TestThread : public Thread
{
public:
	// Construction
	TestThread() : m_Running( true ), m_nThreadId( 0 )
	{}

	int		m_nThreadId;

	// Thread interface
	int run()
	{
		printf( "TestThread %d running...\n", m_nThreadId );

		srand( Time::milliseconds() );

		TestClient clients[ CLIENT_COUNT ];
		for(int i=0;i<CLIENT_COUNT;i++)
		{
			clients[i].m_nThreadId = m_nThreadId;
			clients[i].m_nClientId = i + 1;
		}

		for(i=0;i<TEST_COUNT;i++)
		{
			for(int j=0;j<CLIENT_COUNT;j++)
				if (! clients[j].connected() )
					clients[ j ].open();
				else
					clients[ j ].close();

			Thread::sleep( 500 );
		}

		printf( "TestThread %d stopping...\n", m_nThreadId );
		m_Running = false;
		return 0;
	}

	bool m_Running;
};

//----------------------------------------------------------------------------

const int THREAD_COUNT = 4;
const int GARBAGE_SIZE = 256;

int main(int argc, char* argv[])
{
	int i;

	// start the server
	TestServer server;
	Test( server.start() );

	TestThread threads[ THREAD_COUNT ];
	for(i=0;i<THREAD_COUNT;i++)
	{
		threads[i].m_nThreadId = i + 1;
		threads[i].resume();
	}

	bool done = false;
	while(! done )
	{
		Thread::sleep( 5000 );

		done = true;
		for(i=0;i<THREAD_COUNT && done;i++)
			if ( threads[i].m_Running )
				done = false;
	}

	return 0;
}
