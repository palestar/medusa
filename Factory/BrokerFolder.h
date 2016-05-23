/*
	BrokerFolder.h

	This concrete broker uses a Archive object for storing / loading widgets
	(c)2005 Palestar, Richard Lyle
*/

#ifndef BROKER_FOLDER_H
#define BROKER_FOLDER_H

#include "File/FindFile.h"
#include "Factory/Broker.h"
#include "MedusaDll.h"



//-------------------------------------------------------------------------------

class DLL BrokerFolder : public Broker 
{
public:
	// Exceptions
	class Failure {};

	// Construction
	BrokerFolder();
	BrokerFolder( const char * brokerDirectory, bool write = true, bool autoLoad = true );
	virtual	~BrokerFolder();

	// Broker interface
	dword					version( const WidgetKey & key );	
	dword					size( const WidgetKey & key );
	Widget *				load( const WidgetKey & key );
	bool					store( Widget * pWidget, dword version, bool autoLoad );

	// Mutators
	bool					open( const char * brokerDirectory, bool write = true, bool autoLoad = true );
	void					close();

private:
	// Data
	FindFile				m_Widgets;
	FindFile				m_AutoLoadWidgets;

	CharString				m_BrokerFolder;
	bool					m_Write;
};

//----------------------------------------------------------------------------



#endif

//----------------------------------------------------------------------------
// EOF
