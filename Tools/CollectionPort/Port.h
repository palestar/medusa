/*
	Port.h

	Port object
	(c)2005 Palestar, Richard Lyle
*/

#ifndef COLLECTIONPORT_H
#define COLLECTIONPORT_H

#include "Standard/Dll.h"
#include "Standard/Array.h"
#include "Tools/ResourcerDoc/Port.h"



//-------------------------------------------------------------------------------

#undef DLL
#ifdef COLLECTIONPORT_DLL
#define DLL		DLL_EXPORT
#else

#define DLL		DLL_IMPORT

#ifdef _DEBUG
#pragma comment(lib,"CollectionPortD.lib") 
#else
#pragma comment(lib,"CollectionPort.lib") 
#endif

#endif

//-------------------------------------------------------------------------------

class DLL CollectionPort : public Port
{
public:
	DECLARE_WIDGET_CLASS();
	DECLARE_PROPERTY_LIST();

	typedef Reference<CollectionPort>	Ref;

	// Construction
	CollectionPort();
	// Widget Interface
	bool						read( const InStream & input );
	// Port Interface
	bool						autoLoad();									// enable autoload for this resource
	void						dependencies( DependentArray & dep );		// get the depdendencies
	Resource::Ref 				createResource();							// create the resource 
	CFrameWnd *					createView();								// create a view of the port

	bool						importFile( const char * fileName );		// import from file
	void						shellOpen() const;							// open the source object through a shell program

	ClassKey					m_Type;
	Array< CharString >			m_Collection;
};


#endif

//-------------------------------------------------------------------------------
// EOF
