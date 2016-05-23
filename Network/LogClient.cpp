/*
	LogClient.cpp
	(c)2005 Palestar Inc, Richard Lyle
*/

#include "Debug/Trace.h"
#include "LogClient.h"

//----------------------------------------------------------------------------

LogClient::LogClient() : m_bActive( false ), m_bConnected( false ), m_bLoggedIn( false ), 
	m_nPort( 0 ), m_nSessionId( 0 ), m_nLogLevel( LL_STATUS )
{}

//---------------------------------------------------------------------------------------------------

void LogClient::onReport( const Logging::Report & a_Report )
{
	if (! m_bActive )
		return;				// open() has not ever been called
	if ( a_Report.m_nLevel < m_nLogLevel )
		return;				// level not high enough..

	lock();

	// attempt to re-connect to the server if we are not already connect and in the process of loggin into the server
	if ( m_bLoggedIn )
	{
		OutStream output( send( SERVER_RECV_LOG ) );
		output << CharString( a_Report.m_pFile ) 
			<< a_Report.m_nLine
			<< a_Report.m_nLevel
			<< CharString( a_Report.m_pSystem )
			<< CharString( a_Report.m_pMessage );
	}
	else if ( !m_bConnected )
		m_bConnected = Client::open( new Socket( "ZLIB" ), m_sAddress, m_nPort );

	unlock();
}

//----------------------------------------------------------------------------

void LogClient::onConnect()
{
	TRACE( "LogClient::onConnect" );

	// send login message to the server
	OutStream output( send( SERVER_LOGIN ) );
	output << m_nSessionId << m_sName;
	output.flush();
}

void LogClient::onReceive( byte message, const InStream & input )
{
	switch( message )
	{
	case CLIENT_LOGIN:
		{
			unsigned int nLogLevel;
			input >> nLogLevel;

			TRACE( "CLIENT_LOGIN" );

			lock();

			LOG_STATUS( "LogClient", 
				CharString().format( "LogClient connected to %s:%d", (const char *)m_sAddress, m_nPort) );

			m_nLogLevel = nLogLevel;
			m_bLoggedIn = true;

			unlock();
		}
		break;
	}
}

void LogClient::onDisconnect()
{
	LOG_STATUS( "LogClient", CharString().format( "LogClient disconnected from %s:%d", (const char *)m_sAddress, m_nPort) );

	m_bLoggedIn = false;
	m_bConnected = false;
	m_nLogLevel = LL_STATUS;
}

//----------------------------------------------------------------------------

bool LogClient::open( const char * pName, dword nSessionId, const char * pAddress, int nPort )
{
	// close current connection, if any is open
	close();

	// setup the variables
	m_bActive = true;
	m_bLoggedIn = false;
	m_bConnected = false;
	m_sAddress = pAddress;
	m_nPort = nPort;
	m_sName = pName;
	m_nSessionId = nSessionId;
	m_nLogLevel = LL_STATUS;

	// connect to the server
	m_bConnected = Client::open( new Socket( "ZLIB" ), m_sAddress, m_nPort );

	// login to the server is handled in onConnect()
	return true;
}

bool LogClient::close()
{
	if (! m_bActive )
		return false;		// client isn't open

	// set the active flag to false firstly
	m_bActive = false;
	// close our connect to the server
	Client::close();

	return true;
}
	
//----------------------------------------------------------------------------
//EOF
