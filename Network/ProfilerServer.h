/*
	ProfilerServer.h

	This server objects handles sending profiling information one-way to a ProfilerClient object. This object is used
	when a program wants to allow remote monitoring of it's profiling information and CPU usage.

	(c)2004 Palestar Inc, Richard Lyle
*/

#ifndef PROFILERSERVER_H
#define PROFILERSERVER_H

#include "Network/ProfilerClient.h"
#include "Server.h"
#include "NetworkDLL.h"

//----------------------------------------------------------------------------

class DLL ProfilerServer : public Server
{
public:
	// Construction
	ProfilerServer();

	// Server2 interface
	void			onConnect( dword client );								// called when connection made
	void			onReceive( dword client, byte message, const InStream & );	// called when receiving from client
	void			onDisconnect( dword client );							// called when connection lost
};

//----------------------------------------------------------------------------


#endif

//----------------------------------------------------------------------------
//EOF
