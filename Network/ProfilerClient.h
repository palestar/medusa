/*
	ProfilerClient.h

	This client connects to a remote ProfilerServer to allow remote viewing of a processes profiling information
	(c)2004 Palestar Inc, Richard Lyle
*/

#ifndef PROFILERCLIENT_H
#define PROFILERCLIENT_H

#include "Client.h"
#include "NetworkDLL.h"

//----------------------------------------------------------------------------

class DLL ProfilerClient : public Client
{
public:
	// Types
	enum Messages 
	{
		CLIENT_RECV_STATE,		// send by the server to the client
		SERVER_REQ_STATE,		// sent by the client to request profiling state
	};
	struct Profile
	{
		dword		nThread;	// thread id
		CharString	sName;		// name of the profile section
		qword		nCPU;		// cpu usage per second
		dword		nHits;		// hits per second
		int			nBytes;		// bytes allocated

		Profile & operator=( const Profile & copy );
	};

	// Construction
	ProfilerClient();

	// Client interface
	void				onConnect();									// called when connection made
	void				onReceive( byte message, const InStream & );			// called on incoming data
	void				onDisconnect();									// called when connection lost

	// Accessors
	bool				connected() const;	

	qword				totalCPU() const;				
	int					messageCount() const;
	const char *		message( int n ) const;
	int					profileCount() const;
	const Profile &		profile( int n ) const;

	// Mutators
	void				sendUpdate();									// send state update request

private:
	bool				m_bConnected;		// true if we are connected

	// State information
	qword				m_nTotalCPU;
	Array< CharString >	m_Messages;
	Array< Profile >	m_Profiles;
};

//----------------------------------------------------------------------------

inline bool ProfilerClient::connected() const
{
	return m_bConnected;
}

inline qword ProfilerClient::totalCPU() const
{
	return m_nTotalCPU;
}

inline int ProfilerClient::messageCount() const
{
	return m_Messages.size();
}

inline const char * ProfilerClient::message( int n ) const
{
	return m_Messages[ n ];
}

inline int ProfilerClient::profileCount() const
{
	return m_Profiles.size();
}

inline const ProfilerClient::Profile & ProfilerClient::profile( int n ) const
{
	return m_Profiles[ n ];
}

//----------------------------------------------------------------------------

inline const InStream & operator>>(const InStream &input, ProfilerClient::Profile & profile )
{
	input >> profile.nThread;
	input >> profile.sName;
	input >> profile.nCPU;
	input >> profile.nHits;
	input >> profile.nBytes;
	return input;
}

inline const OutStream & operator<<(const OutStream &output, const ProfilerClient::Profile & profile )
{
	output << profile.nThread;
	output << profile.sName;
	output << profile.nCPU;
	output << profile.nHits;
	output << profile.nBytes;
	return output;
}

inline ProfilerClient::Profile & ProfilerClient::Profile::operator=( const ProfilerClient::Profile & copy )
{
	nThread = copy.nThread;
	sName = copy.sName;
	nCPU = copy.nCPU;
	nHits = copy.nHits;
	nBytes = copy.nBytes;
	return *this;
}

//----------------------------------------------------------------------------




#endif

//----------------------------------------------------------------------------
//EOF
