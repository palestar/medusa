/*
	BrokerCache.h

	This object is used to open server broker archives using a file mask
	(c)2005 Palestar, Richard Lyle
*/

#ifndef BROKER_CACHE_H
#define BROKER_CACHE_H

#include "Factory/BrokerArchive.h"
#include "MedusaDll.h"



//-------------------------------------------------------------------------------

class DLL BrokerCache 
{
public:
	// Construction
	BrokerCache( const char * pMask, bool write = false, bool autoLoad = true );
	virtual ~BrokerCache();

	// Accessors
	int				brokerCount() const;
	BrokerArchive *	broker( int n ) const;

private:
	// Data
	Array< BrokerArchive * >
					m_Brokers;
};

//----------------------------------------------------------------------------



#endif

//----------------------------------------------------------------------------
// EOF
