/*
	BrokerCache.cpp
	(c)2005 Palestar, Richard Lyle
*/

#define FACTORY_DLL
#include "Debug/Trace.h"
#include "File/Path.h"
#include "File/FileDisk.h"
#include "File/FindFile.h"
#include "Factory/BrokerCache.h"

//----------------------------------------------------------------------------

BrokerCache::BrokerCache( const char * pMask, bool write /*= false*/, bool autoLoad /*= true*/ )
{
	Path brokerPath( pMask );

	FindFile ff( pMask );
	for(int i=0;i<ff.fileCount();i++)
	{
		BrokerArchive * pArchive = new BrokerArchive;
		if (! pArchive->open( brokerPath.directory() + ff.file( i ), write , autoLoad ) )
		{
			// failed
			delete pArchive;
			continue;
		}

		m_Brokers.push( pArchive );
	}
}

BrokerCache::~BrokerCache()
{
	for(int i=0;i<m_Brokers.size();i++)
		delete m_Brokers[i];
}

//-------------------------------------------------------------------------------

int BrokerCache::brokerCount() const
{
	return( m_Brokers.size() );
}

BrokerArchive * BrokerCache::broker( int n ) const
{
	return( m_Brokers[ n ] );
}

//-------------------------------------------------------------------------------
// EOF

