/*
	NetworkWS.cpp

	Implemented DllMain so we can call WSAStartup to initialize WinSock
	(c)2005 Palestar Inc, Richard Lyle
*/

#include "Debug/Log.h"

#if defined(WIN32) && !defined(_LIB)

#include <windows.h>

//----------------------------------------------------------------------------

extern "C" int APIENTRY
DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		WSADATA wsaData;
		WSAStartup( MAKEWORD( 2, 2 ), &wsaData );

		LOG_STATUS( "Network", "WinSock initialized, version %d.%d", wsaData.wHighVersion, wsaData.wVersion );
	}
	else if ( dwReason == DLL_PROCESS_DETACH )
	{
		// release winsock!
		WSACleanup();
	}

	return 1;
}

#endif

//----------------------------------------------------------------------------
//EOF
