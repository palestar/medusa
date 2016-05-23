/*
	ProcessClient.h

	ProcessClient class is used to communicate with the ProcessServer class. 
	The ProcessServer is used to run/control processes running on a machine.
	(c)2000 Palestar Inc, Richard Lyle
*/

#ifndef PROCESSCLIENT_H
#define PROCESSCLIENT_H

#include "Standard/Event.h"
#include "Network/Client.h"
#include "GCQ/GCQDll.h"

//-------------------------------------------------------------------------------

class DLL ProcessClient : public Client
{
public:
	enum Messages
	{
		CLIENT_RECV_PROCESS_LIST	= 0x01,		// SERVER_SEND_PROCESSES
		CLIENT_RECV_CONFIG			= 0x02,		// SERVER_SEND_CONFIG
		CLIENT_RECV_LOG				= 0x03,		// SERVER_SEND_LOG
		CLIENT_RECV_STATUS			= 0x04,		// SERVER_SEND_STATUS
		CLIENT_RECV_LOG_ID			= 0x05,		// SERVER_OPEN_LOG
		CLIENT_RECV_LOG_UPDATE		= 0x06,		
		CLIENT_RECV_SEARCHRESULT	= 0x07,		// SERVER_SEARCH_LOGS
		CLIENT_JOB_DONE				= 0x50,		// job is complete, sent in response to many commands
			
		SERVER_LOGIN				= 0x80,		// client is requesting to login
		SERVER_SESSION_LOGIN		= 0x81,		// client is using session id to login
		SERVER_SEND_PROCESS_LIST	= 0x82,		// client is requesting a process list from the server
		SERVER_SEND_CONFIG			= 0x83,		// client is requesting the configuration file for a process
		SERVER_RECV_CONFIG			= 0x84,		// client is sending a updated configuration file for a process
		SERVER_SEND_LOG				= 0x85,		// client is requesting the log file for a process
		SERVER_ADD_PROCESS			= 0x86,		// client is adding a new process
		SERVER_SET_PROCESS			= 0x87,		// client is changing a process
		SERVER_DEL_PROCESS			= 0x88,		// client is removing a process
		SERVER_STOP_PROCESS			= 0x89,		// client is stopping a process
		SERVER_START_PROCESS		= 0x8a,		// client is starting a process
		SERVER_RESTART_PROCESS		= 0x8b,		// client is restarting a process
		SERVER_SEND_STATUS			= 0x8c,		// client is requesting the machine status
		SERVER_STOP_ALL				= 0x8d,		// stop all processes
		SERVER_START_ALL			= 0x8e,		// start all processes
		SERVER_RESTART_ALL			= 0x8f,		// restart all processes
		SERVER_REBOOT				= 0x90,		// reboot the machine
		SERVER_EXIT					= 0x91,		// stop and exit

		SERVER_TERMINATE_PROCESS	= 0x92,		// terminate a process
		SERVER_OPEN_LOG				= 0x93,
		SERVER_CLOSE_LOG			= 0x94,
		SERVER_SEARCH_LOGS			= 0x95,		// search through all existing logfiles

		PING						= 0xff,
		PONG						= 0xfe,
	};
	struct Status
	{
		dword				processGroup;	// what is the servers process group
		dword				networkGroup;	// network group
		int					cpuUsage;		// what is the CPU usage percentage
		int					memoryUsage;	// how many MB are being used currently
		int					processCount;	// how many processes are currently running

		Status &			operator=( const Status & copy );
	};

	// NOTE: The flags are structured to match the old format of the old booleans running & disabled. 
	enum ProcessFlags {
		PF_RUNNING			= 0x0001,		// is the process currently running
		PF_RUNONCE			= 0x0002,		// if true, process will not be restarted once it exits
		PF_DISABLED			= 0x0100,		// if true, process is disabled
	};
	struct Process
	{
		Process() : processId( 0 ), flags( 0 )
		{}

		dword				processId;		// unique id for this process
		CharString			name;			// text name of the process
		CharString			executable;		// process executable file name
		CharString			arguments;		// executable arguments
		CharString			config;			// configuration file name
		CharString			log;			// log file name
		u16					flags;			// process flags

		Process &			operator=( const Process & copy );
	};
	struct SearchLogRequest
	{
		CharString			filemask;		// files to search
		CharString			searchString;	// string to search for
		bool				isRegExp;		// search string is a regular expression
		byte				searchLevel;	// 0: full lines, 1: matching filenames, 2: all files (without search)
		bool				resolveClientId;// try to resolve clientIds to name/userId

		SearchLogRequest &	operator=( const SearchLogRequest & copy );
	};

	// Construction
	ProcessClient();

	// Client interface
	void				onConnect();										// called when connection made
	void				onReceive( byte message, const InStream & );				// called on incoming data
	void				onDisconnect();										// called when connection lost
	
	// Accessors
	bool				connected() const;									// are we connected
	bool				loggedIn() const;									// are we logged in
	dword				sessionId() const;

	// Mutators
	bool				open( const char * serverAddress, int serverPort );	// connect to process server
	bool				login( const char * pID, const char * pPW );		// login into the server
	bool				login( dword sessionId );							// login with sessionId

	bool				getProcessList( Array< Process > & process );		

	bool				getConfig( dword processId, CharString & config );		// get configuration file, save into filename
	bool				putConfig( dword processId, const CharString & config );
	bool				getLog( dword processId, CharString & log );			// get log file
	bool				addProcess( const Process & process );				// add new process
	bool				setProcess( const Process & process );
	bool				deleteProcess( dword processId );
	bool				stopProcess( dword processId );
	bool				startProcess( dword processId );
	bool				restartProcess( dword processId );
	bool				getStatus( Status & status );						// get server status
	
	bool				stopAll();
	bool				startAll();
	bool				restartAll();

	bool				reboot();
	bool				exit();												// request server exit with the following exit code

	bool				terminateProcess( dword processId );				// force a process to be terminated

	dword				openLog( dword processId );							// returns a handle for the log
	bool				isLogValid( dword logId );							// returns true if logId is valid
	void				closeLog( dword logId );							// close the log file
	bool				popLog( dword logId, CharString & line );				// retreive any log updates

	bool				searchLogs( SearchLogRequest req , CharString & result);// search all available logfiles

private:
	// Data
	bool				m_Connected;
	bool				m_LoggedIn;
	dword				m_nSessionId;

	Hash< dword, Queue< CharString > >
						m_Log;
};


//----------------------------------------------------------------------------

inline const InStream & operator>>( const InStream & input, ProcessClient::Status & status )
{
	input >> status.processGroup;
	input >> status.networkGroup;
	input >> status.cpuUsage;
	input >> status.memoryUsage;
	input >> status.processCount;
	return input;
}

inline const OutStream & operator<<( const OutStream & output, const ProcessClient::Status & status )
{
	output << status.processGroup;
	output << status.networkGroup;
	output << status.cpuUsage;
	output << status.memoryUsage;
	output << status.processCount;
	return output;
}

inline ProcessClient::Status & ProcessClient::Status::operator=( const Status & copy )
{
	processGroup = copy.processGroup;
	networkGroup = copy.networkGroup;
	cpuUsage = copy.cpuUsage;
	memoryUsage = copy.memoryUsage;
	processCount = copy.processCount;
	return *this;
}

//----------------------------------------------------------------------------

inline const InStream & operator>>( const InStream & input, ProcessClient::Process & process )
{
	input >> process.processId;
	input >> process.name;
	input >> process.executable;
	input >> process.arguments;
	input >> process.config;
	input >> process.log;
	input >> process.flags;
	return input;
}

inline const OutStream & operator<<( const OutStream & output, const ProcessClient::Process & process )
{
	output << process.processId;
	output << process.name;
	output << process.executable;
	output << process.arguments;
	output << process.config;
	output << process.log;
	output << process.flags;
	return output;
}

inline ProcessClient::Process & ProcessClient::Process::operator=( const Process & copy )
{
	processId = copy.processId;
	name = copy.name;
	executable = copy.executable;
	arguments = copy.arguments;
	config = copy.config;
	log = copy.log;
	flags = copy.flags;
	return *this;
}

//----------------------------------------------------------------------------

inline const InStream & operator>>( const InStream & input, ProcessClient::SearchLogRequest & request )
{
	input >> request.filemask;
	input >> request.searchLevel;
	input >> request.isRegExp;
	input >> request.searchString;
	input >> request.resolveClientId;
	return input;
}

inline const OutStream & operator<<( const OutStream & output, const ProcessClient::SearchLogRequest & request )
{
	output << request.filemask;
	output << request.searchLevel;
	output << request.isRegExp;
	output << request.searchString;
	output << request.resolveClientId;
	return output;
}

inline ProcessClient::SearchLogRequest & ProcessClient::SearchLogRequest::operator=( const SearchLogRequest & copy )
{
	filemask = copy.filemask;
	searchLevel = copy.searchLevel;
	isRegExp = copy.isRegExp;
	searchString = copy.searchString;
	resolveClientId = copy.resolveClientId;
	return *this;
}
//----------------------------------------------------------------------------



#endif

//-------------------------------------------------------------------------------
// EOF

