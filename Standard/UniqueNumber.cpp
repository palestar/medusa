/*
	UniqueNumber.cpp
	(c)2005 Palestar, Richard Lyle
*/

#include "UniqueNumber.h"
#include "String.h"
#include "Time.h"
#include "Atomic.h"
#include "Settings.h"
#include "Debug/Log.h"

#include <time.h>
#include <stdio.h>

#pragma warning(disable:4996)	// This function or variable may be unsafe. Consider using sscanf_s instead. To disable deprecation, use _CRT_SECURE_NO_WARNINGS. See online help for details.

//---------------------------------------------------------------------------------------------------

//! When AtomicID is persisted, we allocate in blocks, writing the top most ID into a file. This is so 
//! we don't need to write into the file each time we allocate an ID, we just write the top and consider all those ID's lost
//! if the process is stopped.

const dword BLOCK_BITS = 12;
const dword BLOCK_SIZE = 1 << BLOCK_BITS;
const dword BLOCK_MASK = BLOCK_SIZE - 1;

//---------------------------------------------------------------------------------------------------

UniqueNumberProvider * GetDefaultProvider()
{
	static TimeUniqueNumberProvider PROVIDER;
	return &PROVIDER;
}

UniqueNumberProvider *	UniqueNumber::sm_pProvider = NULL;

//-------------------------------------------------------------------------------

PersistedUniqueNumberProvider::PersistedUniqueNumberProvider( UniqueNumberType a_eType, dword a_nPersistID, const char * a_pPersistFile /*= NULL*/ ) : 
	m_eType( a_eType ),
	m_nPersistID( a_nPersistID ), 
	m_nWrapCount( 0 ),
	m_nAtomicID( 0 ),
	m_nPersistTypeMask( 0 ),
	m_bEnablePersistFile( a_pPersistFile != NULL ),
	m_sPersistFile( a_pPersistFile != NULL ? a_pPersistFile : "" ),
	m_nLastAtomicID( BLOCK_SIZE )
{
	if ( m_nPersistID > ((1 << PERSISTID_BITS) - 1 ) )
		throw UniqueNumberInvalidPersistID();		// persist ID is out of range

	if ( m_bEnablePersistFile )
	{
		Settings persist( "UIDProvider", m_sPersistFile );
		m_nWrapCount = persist.get( "WrapCount", (dword)0 );

		dword nLastPersistID = persist.get( "LastPersistID", (dword)0 );
		if ( nLastPersistID == m_nPersistID )
		{
			// PersistID is the same, continue the ID's from where we left off..
			m_nAtomicID = persist.get( "LastAtomicID", (dword)0 );
			m_nLastAtomicID = m_nAtomicID + BLOCK_SIZE;			// start a new block, write it out now..
			if ( m_nLastAtomicID < m_nAtomicID )
			{
				// atomic ID's have wrapped, reset back to the start..
				persist.put( "WrapCount", ++m_nWrapCount );

				m_nAtomicID = 0;
				m_nLastAtomicID = BLOCK_SIZE;
			}

			persist.put( "LastAtomicID", m_nLastAtomicID );		
		}
		else
		{
			// PersistID is different, so write out new values..
			persist.put( "LastPersistID", m_nPersistID );
			persist.put( "LastAtomicID", m_nLastAtomicID );
			persist.put( "WrapCount", m_nWrapCount );
		}

		// offset our PersistID by the number of times we have wrapped..
		m_nPersistID += m_nWrapCount;
		LOG_DEBUG_LOW( "PersistUNP", "UID block %u-%u", m_nAtomicID, m_nLastAtomicID );
	}

	m_nPersistTypeMask = ((qword)m_eType << TYPE_SHIFT) & TYPE_MASK;
	m_nPersistTypeMask |= ((qword)m_nPersistID << PERSISTID_SHIFT) & PERSISTID_MASK;
}

qword PersistedUniqueNumberProvider::requestID()
{
	if ( m_nAtomicID >= (((qword)1 << ATOMICID_BITS) - 1) )
		throw UniqueNumberExhaused();

	qword nID = m_nPersistTypeMask | Atomic::increment( (volatile int *)&m_nAtomicID );
	if ( m_bEnablePersistFile && m_nAtomicID == m_nLastAtomicID )
	{
		LOG_DEBUG_LOW( "PersistUNP", "UID block %u-%u", m_nAtomicID, m_nLastAtomicID );

		Settings persist( "UIDProvider", m_sPersistFile );

		// allocate a new block of ID's... 
		m_nLastAtomicID += BLOCK_SIZE;
		if ( m_nLastAtomicID < m_nAtomicID )
		{
			persist.put( "WrapCount", ++m_nWrapCount );
			m_nPersistID += 1;
			m_nPersistTypeMask = ((qword)m_eType << TYPE_SHIFT) & TYPE_MASK;
			m_nPersistTypeMask |= ((qword)m_nPersistID << PERSISTID_SHIFT) & PERSISTID_MASK;

			m_nAtomicID = 0;
			m_nLastAtomicID = BLOCK_SIZE;
		}

		persist.put( "LastAtomicID", m_nLastAtomicID );
	}

	return nID;
}

//---------------------------------------------------------------------------------------------------

TimeUniqueNumberProvider::TimeUniqueNumberProvider() : 
	m_nSeedID( ((qword)Time::seconds() << 32 ) ^ Time::CPU() ),			// this should be a unique value 99.99999999% of the time for any run of an application
	m_nMachineID( m_nSeedID & 0xffffffff00000000LL ),
	m_nAtomicID( (dword)(m_nSeedID & 0x00000000ffffffffLL) )
{}

qword TimeUniqueNumberProvider::requestID()
{
	return m_nMachineID | Atomic::increment( (volatile int *)&m_nAtomicID );
}

//-------------------------------------------------------------------------------

UniqueNumber::UniqueNumber( bool a_bGenerateKey /*= true*/ )
{
	if ( a_bGenerateKey )
		requestNumber();
	else
		m_Id = 0;
}

UniqueNumber::UniqueNumber( const UniqueNumber &src )
{
	m_Id = src.m_Id;
}

UniqueNumber::UniqueNumber( const char * id )
{
	set( id );
}

UniqueNumber::UniqueNumber(qword id)
{
	m_Id = id;
}

//---------------------------------------------------------------------------------------------------

void UniqueNumber::set( const char * pKey )
{
	if ( strncmp( pKey, "0x", 2 ) == 0 )
		sscanf( pKey, "0x%llX", &m_Id );
	else
		sscanf( pKey, "%llu", &m_Id );
}

void UniqueNumber::setHex( const char * pHexKey )
{
	sscanf( pHexKey, "0xllX", &m_Id );
}

//-------------------------------------------------------------------------------

CharString UniqueNumber::string() const
{
	return CharString().format("%llu", m_Id);
}

CharString UniqueNumber::hexString() const
{
	return CharString().format("0x%llX", m_Id );
}

//---------------------------------------------------------------------------------------------------

void UniqueNumber::requestNumber()
{
	if ( sm_pProvider == NULL )
		sm_pProvider = GetDefaultProvider();

	m_Id = sm_pProvider->requestID();
	if ( m_Id == 0 )
		throw UniqueNumberExhaused();
}

//---------------------------------------------------------------------------------------------------

bool UniqueNumber::setProvider( UniqueNumberProvider * pProvider /*= NULL*/ )
{
	if ( (sm_pProvider = pProvider) == NULL )
		sm_pProvider = GetDefaultProvider();
	return true;
}

//-------------------------------------------------------------------------------
// EOF
