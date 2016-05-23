/*
	ProfilerServer.cpp

	This server objects handles sending profiling information one-way to a ProfilerClient object. This object is used
	when a program wants to allow remote monitoring of it's profiling information and CPU usage.

	(c)2004 Palestar Inc, Richard Lyle
*/

#include "Debug/Profile.h"
#include "Network/ProfilerServer.h"

//----------------------------------------------------------------------------

ProfilerServer::ProfilerServer()
{}

void ProfilerServer::onConnect( dword client )
{}

void ProfilerServer::onReceive( dword client, byte message, const InStream & input )
{
	switch( message )
	{
	case ProfilerClient::SERVER_REQ_STATE:
		{
			// lock up the profiler, gather information and send it to the client
			Profiler::lock( true );

			qword nTotalCPU = Profiler::totalCPU();

			Array< CharString > messages;
			for(int i=0;i<Profiler::messageCount();i++)
				messages.push( Profiler::message( i ) );

			Array< ProfilerClient::Profile > profiles;
			for(int j=0;j<Profiler::threadCount();j++)
			{
				dword nThread = Profiler::thread( j );
				
				int nCount = Profiler::profileCount( nThread );
				for( int k=0;k<nCount;k++)
				{
					const Profiler::Profile & profile = Profiler::profile( nThread, k );

					ProfilerClient::Profile & init = profiles.push();
					init.nThread = nThread;
					init.sName = profile.pName;
					init.nCPU = profile.nAvCPU;
					init.nHits = profile.nAvHits;
					init.nBytes = profile.nBytes;
				}
			}

			Profiler::lock( false );

			OutStream message( send( client, ProfilerClient::CLIENT_RECV_STATE ) ); 
			message << nTotalCPU << messages << profiles;
		}
		break;
	default:
		// invalid message, just remove the client
		removeClient( client );
		break;
	}
}

void ProfilerServer::onDisconnect( dword client )
{}

//----------------------------------------------------------------------------
//EOF
