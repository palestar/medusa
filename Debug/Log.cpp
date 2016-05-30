/**
	@file Log.cpp

	(c)2009 Palestar Inc
	@author Richard Lyle @date 1/28/2012 2:08:55 PM
*/

#include "Log.h"
#include "Standard/Time.h"
#include "Standard/StringHelpers.h"

#include <stdarg.h>

#if defined(_WIN32)

#include <io.h>
#include <fcntl.h>
#include <share.h>
#include <sys/stat.h>

#define _WIN32_WINNT    0x0400        
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#else

#include <fcntl.h>
#include <unistd.h>

#endif

const size_t			MAX_LOG_MESSAGE = 1024 * 1;

#pragma warning( disable : 4996 )	// warning C4996: '_vsnprintf': This function or variable may be unsafe. Consider using _vsnprintf_s instead. To disable deprecation, use _CRT_SECURE_NO_WARNINGS. See online help for details.

//---------------------------------------------------------------------------------------------------

Logging::Reactor::Reactor()
{
	AutoLock lock( &getReactorLock() );
	getReactorList().push_back( this );
}

Logging::Reactor::~Reactor()
{
	AutoLock lock( &getReactorLock() );
	getReactorList().remove( this );
}

//---------------------------------------------------------------------------------------------------

void Logging::report( const char * a_pFile, 
					unsigned int a_nLine, 
					unsigned int a_nLevel,
					const char * a_pSystem, 
					const char * a_pMessage, ... )
{
	va_list va;
	va_start( va, a_pMessage );

	char pBuffer[ MAX_LOG_MESSAGE ];
#if defined(_WIN32)
	_vsnprintf( pBuffer, sizeof(pBuffer) - 1, a_pMessage, va );
#else
	vsnprintf( pBuffer, sizeof(pBuffer) - 1, a_pMessage, va );
#endif
	va_end(va);

	Report r;
	r.m_pFile = a_pFile;
	r.m_nLine = a_nLine;
	r.m_nLevel = a_nLevel;
	r.m_pSystem = a_pSystem;
	r.m_pMessage = pBuffer;

	report( r );
}

void Logging::report( const Report & a_Report )
{
	AutoLock lock( &getReactorLock() );

	ReactorList & reactors = getReactorList();
	for( ReactorList::iterator iReactor = reactors.begin();
		iReactor != reactors.end(); ++iReactor )
	{
		(*iReactor)->onReport( a_Report );
	}
}

const char * Logging::logLevelText( unsigned int a_nLevel )
{
	static const char * LEVELS[] = 
	{
		"DBGL",
		"DBGM",
		"DBGH",
		"STAT",
		"WARN",
		"ERRO",
		"CRIT"
	};

	if ( a_nLevel < (sizeof(LEVELS)/sizeof(LEVELS[0])) )
		return LEVELS[ a_nLevel ];

	return "?";
}

//---------------------------------------------------------------------------------------------------

CriticalSection & Logging::getReactorLock()
{
	static CriticalSection * pLock = new CriticalSection();
	return *pLock;
}

Logging::ReactorList & Logging::getReactorList()
{
	static ReactorList * pList = new ReactorList();
	return *pList;
}

//---------------------------------------------------------------------------------------------------

FileReactor::FileReactor( const std::string & a_sLogFile, 
						 unsigned int a_nMinLevel /*= LL_STATUS*/, 
						 const std::string & a_sLogExclude /*= ""*/,
						 unsigned int a_nOptions /*= O_TIMESTAMP|O_LEVEL|O_SYSTEM*/ ) : 
	m_sLogExclude( a_sLogExclude ),
	m_nMinLevel( a_nMinLevel ), 
	m_nOptions( a_nOptions ),
	m_pOutputThread( new OutputThread( a_sLogFile ) )
{
	if ( m_nOptions & O_RESET )
	{
#if defined(_WIN32)
		// zero out the log file if the O_RESET flag is provided..
		int fp = _creat( a_sLogFile.c_str(), _S_IREAD | _S_IWRITE );
		if ( fp >= 0 )
			_close( fp );
#else
		int fp = ::open( a_sLogFile.c_str(), O_CREAT | O_TRUNC | O_WRONLY, 0664 );
		if ( fp >= 0 )
			::close( fp );
#endif
	}

	// start up the thread now..
	m_pOutputThread->resume();
}

FileReactor::~FileReactor()
{
	m_pOutputThread->m_bOutputActive = false;
	m_pOutputThread->m_WakeUp.signal();
	m_pOutputThread->wait();

	delete m_pOutputThread;
	m_pOutputThread = NULL;
}

void FileReactor::onReport( const Logging::Report & a_Report )
{
	if (! m_pOutputThread )
		return;
	if ( a_Report.m_nLevel < m_nMinLevel )
		return;
	if ( stristr( m_sLogExclude.c_str(), a_Report.m_pSystem ) != NULL )
		return;

	std::string sLine;
	sLine.reserve( MAX_LOG_MESSAGE );

	if ( m_nOptions & O_TIMESTAMP )
	{
		sLine += Time::format( Time::seconds(), "%m/%d/%y %H:%M:%S" );
		sLine += " | ";
	}

	if ( m_nOptions & O_LEVEL )
	{
		sLine += Logging::logLevelText( a_Report.m_nLevel );
		sLine += " | ";
	}
	if ( m_nOptions & O_SYSTEM )
	{
		sLine += a_Report.m_pSystem;
		sLine += " | ";
	}
	
	sLine += a_Report.m_pMessage;

	if ( m_nOptions & O_FILE_LINE )
	{
		sLine += " | ";
		sLine += CharString().format( "%s:%u", a_Report.m_pFile, a_Report.m_nLine );
	}

	sLine += "\n";

	m_pOutputThread->m_OutputLock.lock();
	m_pOutputThread->m_sOutputBuffer += sLine;
	m_pOutputThread->m_OutputLock.unlock();
}

//---------------------------------------------------------------------------------------------------

FileReactor::OutputThread::OutputThread( const std::string & a_sLogFile ) : 
	m_sLogFile( a_sLogFile ), m_bOutputActive( true ), SafeThread( LOW )
{
	m_sOutputBuffer.reserve( 1024 * 32 );
}

int FileReactor::OutputThread::run()
{
#if defined(_WIN32)
	int fp = _sopen( m_sLogFile.c_str(), _O_APPEND | _O_CREAT | _O_WRONLY, _SH_DENYNO, _S_IREAD | _S_IWRITE );
#else
	int fp = ::open( m_sLogFile.c_str(), O_APPEND | O_CREAT | O_WRONLY, 0664 );
#endif
	if ( fp < 0 )
	{
		fprintf( stderr, "ERROR: Failed to open log file %s\n", m_sLogFile.c_str() );
		return -1;
	}

	while( m_bOutputActive )
	{
		m_WakeUp.wait( 1000 * 3 );

		m_OutputLock.lock();
		if (! m_sOutputBuffer.empty() )
		{
			// copy into a buffer for writing, so we don't keep the lock while we are doing disk I/O...
			std::string sWrite( m_sOutputBuffer );
			m_sOutputBuffer.clear();
			m_OutputLock.unlock();

#if defined(_WIN32)
			_write( fp, sWrite.c_str(), sWrite.size() );
#else
			if ( ::write( fp, sWrite.c_str(), sWrite.size() ) != sWrite.size() )
				fprintf( stderr, "Failed to write: %s\n", sWrite.c_str() );
#endif
		}
		else
		{
			m_OutputLock.unlock();
		}
	}

#if defined(_WIN32)
	_close( fp );
#else
	::close( fp );
#endif

	return 0;
}

//---------------------------------------------------------------------------------------------------

DebugReactor::DebugReactor( unsigned int a_nLogLevel /*= LL_DEBUG_LOW*/ ) : m_nLogLevel( a_nLogLevel )
{}

void DebugReactor::onReport( const Logging::Report & a_Report )
{
	if ( a_Report.m_nLevel < m_nLogLevel )
		return;
#if defined(_WIN32)
	if (! IsDebuggerPresent() )
		return;

	OutputDebugString( (std::string( a_Report.m_pMessage ) + "\n").c_str() );
#endif
}

//---------------------------------------------------------------------------------------------------

StandardOutReactor::StandardOutReactor( unsigned int a_nLogLevel /*= LL_STATUS*/ ) : m_nLogLevel( a_nLogLevel )
{}

void StandardOutReactor::onReport( const Logging::Report & a_Report )
{
	if ( a_Report.m_nLevel < m_nLogLevel )
		return;
	printf( "%s | %s | %s\n", Time::format( Time::seconds(), "%H:%M:%S" ).cstr(), 
		a_Report.m_pSystem, a_Report.m_pMessage );
}

//---------------------------------------------------------------------------------------------------
//EOF
