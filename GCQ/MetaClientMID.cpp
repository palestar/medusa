/*
	MetaClientMID.cpp
	(c)2014 Palestar Inc, Richard Lyle, Jack Wallace
*/

#if defined(_WIN32)
#define _WIN32_DCOM
#include <atlbase.h>
#include <objbase.h>
#include <wbemidl.h>

#pragma comment(lib, "wbemuuid.lib")
#endif

#include "Debug/Log.h"
#include "Standard/MD5.h"
#include "Standard/Time.h"
#include "Standard/Registry.h"
#include "GCQ/MetaClient.h"

//----------------------------------------------------------------------------

CharString MetaClient::getMID()
{
	CharString mid;

#if defined(_WIN32) || defined(_WIN64)
	// in windows, try to use the HD serial number as the MID
	DWORD dSerial;
	if (GetVolumeInformation(_T("C:\\"), NULL, 0, &dSerial, NULL, NULL, NULL, 0) != NULL)
	{
		CharString sSerial = CharString().format("%d", dSerial);
		mid = MD5(sSerial).checksum();
	}
#endif
	// go get a mid from the registry if we can't get it from the HD..
	if ( mid.length() == 0 )
	{
		// get/put the machine key
		Registry machine( Registry::LOCAL, "GCQ" );
		mid = machine.get( "MID", "" );
		if ( mid.length() == 0 )
		{
			mid = MD5( CharString().format( "%4.4x-%4.4x-%4.4x-%4.4x", rand(), rand(), rand(), rand() ) ).checksum();
			machine.put( "MID", mid );
		}
	}

	return mid;
}

//---------------------------------------------------------------------------------------------------
// EOF
