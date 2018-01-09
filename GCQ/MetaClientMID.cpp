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
	HRESULT hr = ::CoInitializeSecurity(NULL, -1, NULL, NULL,
		RPC_C_AUTHN_LEVEL_DEFAULT, RPC_C_IMP_LEVEL_IMPERSONATE,
		NULL, EOAC_NONE, NULL);
	
	if (! FAILED( hr ) )
	{
		CComPtr<IWbemLocator> pWbemLocator;
		hr = pWbemLocator.CoCreateInstance(CLSID_WbemLocator);

		if (! FAILED( hr ) )
		{
			CComPtr<IWbemServices> pWbemServices;
			hr = pWbemLocator->ConnectServer(CComBSTR(L"root\\cimv2"), NULL, NULL, 0, NULL, 0, NULL, &pWbemServices);

			if (! FAILED( hr ) )
			{
				CComPtr<IEnumWbemClassObject> pEnum;
				CComBSTR cbsQuery = L"SELECT SerialNumber FROM Win32_DiskDrive";
				hr = pWbemServices->ExecQuery(CComBSTR("WQL"), cbsQuery, WBEM_FLAG_FORWARD_ONLY, NULL, &pEnum);

				if (! FAILED( hr ) )
				{
					unsigned long uObjectCount = 0;
					CComPtr<IWbemClassObject> pWmiObject;
					hr = pEnum->Next(WBEM_INFINITE, 1, &pWmiObject, &uObjectCount);

					if (! FAILED( hr ) )
					{
						CComVariant cvtSerial;
						hr = pWmiObject->Get(L"SerialNumber", 0, &cvtSerial, 0, 0);
						if (! FAILED( hr ) )
						{
							mid = CW2A(cvtSerial.bstrVal);
							mid = MD5( mid ).checksum();
						}
					}
				}
			}
		}
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
