/**
	@file Log.h
	@brief This header includes all macros which use the loggin subsystem for Medusa. All logging in the application
	goes through the single function Logging::report() which then passes on the data to Reactor objects.

	Implementing a Reactor object is done by inheriting from Logging::Reactor, and implementing the onReport() pure virtual function.
	
	(c)2009 Palestar Inc
	@author Richard Lyle @date 1/28/2012 12:57:18 PM
*/

#ifndef LOG_H
#define LOG_H

#include <string>
#include <list>

#include "Standard/CriticalSection.h"
#include "Standard/Event.h"
#include "SafeThread.h"

#include "MedusaDll.h"

#pragma warning( disable : 4251 )		// warning C4251: 'Logging::sm_Reactors' : class 'std::list<_Ty>' needs to have dll-interface to be used by clients of class 'Logging'

//---------------------------------------------------------------------------------------------------

enum LogLevels {
	LL_DEBUG_LOW		= 0,		// low-level usually unimportant debugging messages
	LL_DEBUG_MED		= 1,		// mid-level debug messages
	LL_DEBUG_HIGH		= 2,		// high-level debug messages
	LL_STATUS			= 3,		// status messages
	LL_WARNING			= 4,		// warning message, something you might be concerned about
	LL_ERROR			= 5,		// error message, some action should be taken to fix the issue
	LL_CRITICAL			= 6,		// critical show stopping error messages
};

//---------------------------------------------------------------------------------------------------

class DLL Logging
{
public:
	struct Report 
	{
		const char *		m_pFile;
		unsigned int		m_nLine;
		unsigned int		m_nLevel;
		const char *		m_pSystem;
		const char *		m_pMessage;
	};

	//! This class allows another class to hook into the logging system and process log messages.
	class DLL Reactor 
	{
	public:
		//! Construction
		Reactor();
		virtual ~Reactor();
		//! Interface
		virtual void	onReport( const Report & a_Report ) = 0;
	};

	static void		report( const char * a_pFile, 
						unsigned int a_nLine, 
						unsigned int a_nLevel,
						const char * a_pSystem, 
						const char * a_pMessage, ... );
	static void		report( const Report & a_Report );
	
	//! This returns a text description for the given level
	static const char * 
					logLevelText( unsigned int a_nLevel );

protected:
	//! Types
	typedef std::list< Reactor * >		ReactorList;

	//! Data
	static CriticalSection &	getReactorLock();
	static ReactorList &		getReactorList();
};

//---------------------------------------------------------------------------------------------------

#define LOG_DEBUG_LOW( SYSTEM, ... )	Logging::report( __FILE__, __LINE__, LL_DEBUG_LOW, SYSTEM, __VA_ARGS__ )
#define LOG_DEBUG_MED( SYSTEM, ... )	Logging::report( __FILE__, __LINE__, LL_DEBUG_MED, SYSTEM, __VA_ARGS__ )
#define LOG_DEBUG_HIGH( SYSTEM, ... )	Logging::report( __FILE__, __LINE__, LL_DEBUG_HIGH, SYSTEM, __VA_ARGS__ )
#define LOG_STATUS( SYSTEM, ... )		Logging::report( __FILE__, __LINE__, LL_STATUS, SYSTEM, __VA_ARGS__ )
#define LOG_WARNING( SYSTEM, ... )		Logging::report( __FILE__, __LINE__, LL_WARNING, SYSTEM, __VA_ARGS__ )
#define LOG_ERROR( SYSTEM, ... )		Logging::report( __FILE__, __LINE__, LL_ERROR, SYSTEM, __VA_ARGS__ )
#define LOG_CRITICAL( SYSTEM, ... )		Logging::report( __FILE__, __LINE__, LL_CRITICAL, SYSTEM, __VA_ARGS__ )

//---------------------------------------------------------------------------------------------------

//! This reactor outputs to a log file in various formats specified by the Option flags passed into the reactor.
class DLL FileReactor : public Logging::Reactor
{
public:
	//! Types
	enum Options {
		O_TIMESTAMP			= 0x1,		// add timetamp to all output
		O_FILE_LINE			= 0x2,		// add file & line to all output
		O_LEVEL				= 0x4,		// display log level
		O_SYSTEM			= 0x8,		// display system
		O_RESET				= 0x10,		// reset the log file to zero length on start up

		O_DEFAULTS			= O_TIMESTAMP|O_LEVEL|O_SYSTEM,
	};

	//! Construction
	FileReactor( const std::string & a_sLogFile,	// name of the log file
		unsigned int a_nMinLevel = LL_STATUS,		// the minimum logging level for a report to be outputted to the log file
		const std::string & a_sLogExclude = "",		// This string should contain any system you want this reactor to ignore.. 
		unsigned int a_nOptions = O_DEFAULTS );		// options for the log file format.
	virtual ~FileReactor();

	//! Logging::Reactor interface
	virtual void	onReport( const Logging::Report & a_Report );

protected:
	//! Types
	class OutputThread : public SafeThread
	{
	public:
		// Construction
		OutputThread( const std::string & a_sLogFile );
		// Thread interface
		int run();
	
		bool				m_bOutputActive;
		Event				m_WakeUp;
		std::string			m_sLogFile;
		CriticalSection		m_OutputLock;
		std::string			m_sOutputBuffer;
	};

	//! Data
	unsigned int		m_nMinLevel;
	unsigned int		m_nOptions;
	std::string 		m_sLogExclude;

	OutputThread *		m_pOutputThread;
};

//---------------------------------------------------------------------------------------------------

// This reactor logs to the debug output if a debugger is attached..
class DLL DebugReactor : public Logging::Reactor
{
public:
	//! Construction
	DebugReactor( unsigned int a_nLogLevel = LL_DEBUG_LOW );
	//! Logging::Reactor interface
	virtual void	onReport( const Logging::Report & a_Report );

protected:
	//! Data
	unsigned int	m_nLogLevel;
};

//---------------------------------------------------------------------------------------------------

//! This reactor outputs to stdout, useful for console programs..
class DLL StandardOutReactor : public Logging::Reactor
{
public:
	//! Construction
	StandardOutReactor( unsigned int a_nLogLevel = LL_STATUS );
	//! Logging::Reactor interface
	virtual void	onReport( const Logging::Report & a_Report );

protected:
	//! Data
	unsigned int	m_nLogLevel;
};

//---------------------------------------------------------------------------------------------------

#endif

//---------------------------------------------------------------------------------------------------
//EOF
