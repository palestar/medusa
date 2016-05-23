/*
	TestUDPServer.cpp
	(c)2005 Palestar Inc, Richard Lyle
*/

#include "UnitTest.h"
#include "Network/UDPServer.h"

//---------------------------------------------------------------------------------------------------

class MyUDPServer : public UDPServer
{
public:
	MyUDPServer() : m_nNextClientId( 1 ), m_nMessageCRC( 0 )
	{}

	// UDPServer interface
	virtual void onUDP( u32 nAddress, u16 nPort, u8 nMessage, UDP * pUDP )
	{
		dword nClientId = m_nNextClientId++;

		printf( "onUDP - New Client, nClientId = %u, nAddress = %8.8x, nPort = %u, nMessage = %u\n", 
			nClientId, nAddress, nPort, nMessage );
		registerClient( nClientId, nAddress, nPort );
		m_nMessageCRC += nMessage;

		InStream input ( pUDP );
		u32 nValidateCRC;
		input >> nValidateCRC;

		Test( nValidateCRC == m_nMessageCRC );
	}

	virtual void onClientUDP( u32 nClient, u8 nMessage, UDP * pUDP )
	{
		printf( "onClientUDP - nClientId = %u, nMessage = %u\n", nClient, nMessage );
		m_nMessageCRC += nMessage;

		InStream input ( pUDP );
		u32 nValidateCRC;
		input >> nValidateCRC;

		Test( nValidateCRC == m_nMessageCRC );
	}

	u32			m_nNextClientId;
	u32			m_nMessageCRC;
};

//---------------------------------------------------------------------------------------------------

class TestUDPServer : public UnitTest
{
public:
	TestUDPServer( const char * pName ) : UnitTest( pName )
	{}

	void test()
	{
		MyUDPServer MyServer;
		Test( MyServer.start( 5555 ) );

		u32 nMessageCRC = 0;
		u32 nLoopCount = 0;
		while( nLoopCount < 1000 )
		{
			u8 nMessage = rand() % 255;
			nMessageCRC += nMessage;

			MyServer.sendUDP( UDPServer::getAddress( "127.0.0.1" ), 5555, nMessage ) << nMessageCRC;

			++nLoopCount;

			Thread::sleep( 10 );
		}

		Test( MyServer.stop() );
	}

} TEST_UDPSERVER( "TestUDPServer" );

//----------------------------------------------------------------------------
//EOF
