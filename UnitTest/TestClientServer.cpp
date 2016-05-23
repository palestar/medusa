/**
	@file TestClientServer.cpp
	
	(c)2012 Palestar Inc
	@author Richard Lyle @date 8/23/2012 5:31:18 PM
*/

#include "UnitTest.h"
#include "Debug/SafeThread.h"
#include "Network/Client.h"
#include "Network/Server.h"

#define TEST_CLIENT_COUNT	100
#define TEST_SECONDS		30

//---------------------------------------------------------------------------------------------------

class TestClient : public Client, public SafeThread
{
public:
	TestClient() : m_bActive( true )
	{
		// actually start our thread, this will call run() ..
		resume();
	}

	//! Client interface
	virtual void onConnect()
	{
		printf( "TestClient %p - onConnect()\n", this );
	}

	virtual void onReceive( byte message, const InStream & )
	{
		// expect a message with the ID of 1
		Test( message == 0x1 );

		//printf( "TestClient %p - OnReceive()\n", this );

		// send back a response with the ID of 2
		send( 0x2 );
	}

	virtual void onDisconnect()
	{
		printf( "TestClient %p - onDisconnect()\n", this );
		m_bActive = false;
	}

	//! SafeThread interface
	virtual int run()
	{
		Test( Client::open( new Socket(), "localhost", 8888 ) );
		
		while( m_bActive )
		{
			// update this client..
			Client::update();
			// sleep some time..
			Thread::sleep( 10 );
		}

		return 0;
	}

	volatile bool		m_bActive;
};

class TestServer : public Server
{
public:
	TestServer()
	{
		start( new Socket(), 8888, TEST_CLIENT_COUNT );

		for(int i=0;i<TEST_CLIENT_COUNT;++i)
		{
			m_Clients.push( new TestClient() );
			Thread::sleep( 10 );
			Server::update();
		}
	}

	~TestServer()
	{
		for(int i=0;i<m_Clients.size();++i)
		{
			TestClient * pClient = m_Clients[i];

			pClient->m_bActive = false;
			pClient->wait();		// wait for the thread to exit
			
			delete pClient;
		}
	}

	virtual void onConnect( dword client )
	{
		printf( "TestServer %p - onConnect( %u )\n", this, client );
		send(client, 0x1 );
	}

	virtual void onReceive( dword client, byte message, const InStream & )
	{
		//printf( "TestServer %p - OnReceive( %u, %u )\n", this, client, (int)message );

		Test( message == 0x2 );
		send( client, 0x1 );
	}

	virtual void onDisconnect( dword client )
	{
		printf( "TestServer %p - onDisconnect( %u )\n", this, client );
	}

	virtual void onReject( Socket * pConnection )
	{
		printf( "TestServer %p - onReject( %p )\n", this, pConnection );
	}

	Array< TestClient * >	m_Clients;
};

//----------------------------------------------------------------------------

class TestClientServer : public UnitTest
{
public:
	TestClientServer( const char * pName ) : UnitTest( pName )
	{}

	void test()
	{
		TestServer server;

		dword nStartTime = Time::seconds();
		while( server.update() )
		{
			Thread::sleep( 10 );

			if ( (Time::seconds() - nStartTime) > TEST_SECONDS )
				break;
		}
	}

} TEST_CLIENTSERVER( "TestClientServer" );

//---------------------------------------------------------------------------------------------------
//EOF
