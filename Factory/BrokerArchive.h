/*
	BrokerArchive.h

	This concrete broker uses a Archive object for storing / loading widgets
	(c)2005 Palestar, Richard Lyle
*/

#ifndef BROKER_ARCHIVE_H
#define BROKER_ARCHIVE_H

#include "File/Archive.h"
#include "Factory/Broker.h"
#include "MedusaDll.h"

//-------------------------------------------------------------------------------

class DLL BrokerArchive : public Broker 
{
public:
	// Exceptions
	class Failure {};

	// Construction
	BrokerArchive();
	BrokerArchive( const char * archiveFile, bool write = true, bool autoLoad = true );
	virtual	~BrokerArchive();

	// Broker interface
	dword					version( const WidgetKey & key );
	dword					size( const WidgetKey & key );
	Widget *				load( const WidgetKey & key );
	bool					store( Widget * pWidget, dword version, bool autoLoad );

	// Mutators
	bool					open( const char * archiveFile, bool write = true, bool autoLoad = true );
	void					close();

	Archive &				archive();

private:
	// Data
	Archive					m_Archive;
};

//----------------------------------------------------------------------------



#endif

//----------------------------------------------------------------------------
// EOF
