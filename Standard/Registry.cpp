/*
	Registry.cpp
	(c)2005 Palestar, Richard Lyle
*/

#include "Standard/Registry.h"
#include "Standard/Array.h"

#if defined(_WIN32)
#include <windows.h>
#else
#define MAX_PATH		255
#endif

//-------------------------------------------------------------------------------

const char *	BASE_ROOT		= ("Software\\Palestar\\");

static bool ParseKeys( const char * pPath, char * pKey, char * pSubKey )
{
	strncpy( pKey, pPath, MAX_PATH );
	char * pLastSlash = strrchr( pKey, '\\' );
	if ( pLastSlash == NULL )
		return false;

	*pLastSlash = 0;
	strcpy<char>( pSubKey, pLastSlash + 1 );
	return true;
}

//-------------------------------------------------------------------------------

Registry::Registry( RootKey root, const char * pApplication ) : m_bUser( root == USER ), m_sApplication( pApplication )
{
	m_sApplication = CharString(BASE_ROOT) + m_sApplication + "\\";
}

Registry::~Registry()
{}

//-------------------------------------------------------------------------------

dword Registry::get( const char * pKey, dword nDefault )
{
#if defined(_WIN32)
	dword nValue = nDefault;
	char sKey[ MAX_PATH ], sSubKey[ MAX_PATH ];
	if ( ParseKeys( m_sApplication + pKey, sKey, sSubKey ) )
	{
		HKEY hKey = 0;
		if ( RegCreateKeyA( m_bUser ? HKEY_CURRENT_USER : HKEY_LOCAL_MACHINE, sKey, &hKey ) != ERROR_SUCCESS )
			return nValue;

		dword keySize, keyType;
		if ( !RegQueryValueExA( hKey, sSubKey, 0, &keyType, NULL, &keySize) 
			&& keyType == REG_DWORD && keySize == sizeof(dword))
		{
			RegQueryValueExA( hKey, sSubKey, 0, &keyType, (byte *)&nValue, &keySize);
		}

		RegCloseKey( hKey );
	}
	return nValue;
#else
	return nDefault;
#endif
}

CharString Registry::get( const char * pKey, const char * pDefault )
{
#if defined(_WIN32)
	char sValue[ MAX_PATH ];
	strncpy( sValue, pDefault, MAX_PATH );
	sValue[ MAX_PATH - 1 ] = 0;

	char sKey[ MAX_PATH ], sSubKey[ MAX_PATH ];
	if ( ParseKeys( m_sApplication + pKey, sKey, sSubKey ) )
	{
		HKEY hKey = 0;
		if ( RegCreateKeyA( m_bUser ? HKEY_CURRENT_USER : HKEY_LOCAL_MACHINE, sKey, &hKey ) == ERROR_SUCCESS )
		{
			dword keyType, keySize;
			if ( RegQueryValueExA( hKey, sSubKey, 0, &keyType, NULL, &keySize) == ERROR_SUCCESS 
				&& keyType == REG_SZ && keySize < MAX_PATH )
			{
				RegQueryValueExA( hKey, sSubKey, 0, &keyType, (byte *)sValue, &keySize);
			}

			RegCloseKey( hKey );
		}
	}
	return sValue;
#else
	return pDefault;
#endif
}

bool Registry::put( const char * pKey, dword nValue )
{
#if defined(_WIN32)
	char sKey[ MAX_PATH ], sSubKey[ MAX_PATH ];
	if (! ParseKeys( m_sApplication + pKey, sKey, sSubKey ) )
		return false;

	HKEY hKey = 0;
	if ( RegCreateKeyA( m_bUser ? HKEY_CURRENT_USER : HKEY_LOCAL_MACHINE, sKey, &hKey ) != ERROR_SUCCESS )
		return false;

	bool bSuccess = false;
	if ( !RegSetValueExA( hKey, sSubKey, 0, REG_DWORD, (byte *)&nValue, sizeof(nValue) ) )
		bSuccess = true;

	RegCloseKey( hKey );
	return bSuccess;
#else
	return false;
#endif
}

bool Registry::put( const char * pKey, const char * pValue )
{
#if defined(_WIN32)
	char sKey[ MAX_PATH ], sSubKey[ MAX_PATH ];
	if (! ParseKeys( m_sApplication + pKey, sKey, sSubKey ) )
		return false;

	HKEY hKey = 0;
	if ( RegCreateKeyA( m_bUser ? HKEY_CURRENT_USER : HKEY_LOCAL_MACHINE, sKey, &hKey ) != ERROR_SUCCESS )
		return false;
	bool bSuccess = false;
	if ( !RegSetValueExA( hKey, sSubKey, 0, REG_SZ, (byte *)pValue, strlen(pValue) + 1 ) )
		bSuccess = true;

	RegCloseKey( hKey );
	return bSuccess;
#else
	return false;
#endif
}

//-------------------------------------------------------------------------------
// EOF
