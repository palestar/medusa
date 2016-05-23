/*
	Port.h

	Port object
	(c)2005 Palestar, Richard Lyle
*/

#ifndef $$ROOT$$_H
#define $$ROOT$$_H

#include "Standard/Dll.h"
#include "Tools/ResourcerDoc/Port.h"

//-------------------------------------------------------------------------------

#undef DLL
#ifdef $$ROOT$$_DLL
#define DLL		DLL_EXPORT
#else
#define DLL		DLL_IMPORT

#ifdef _DEBUG
#pragma comment(lib,"$$root$$D.lib") 
#else
#pragma comment(lib,"$$root$$.lib") 
#endif

#endif

//-------------------------------------------------------------------------------

class DLL $$root$$ : public Port
{
public:
	DECLARE_WIDGET_CLASS();
	DECLARE_PROPERTY_LIST();

	typedef Reference<$$root$$>	Ref;

	// Construction
								$$root$$();
	// Widget Interface
	bool						read( InStream & input );
	bool						write( OutStream &output );

	// Port Interface
	bool						autoLoad();									// enable autoload for this resource
	void						dependencies( DependentArray & dep );		// get the depdendencies
	Resource * 					createResource();							// create the resource 
	CFrameWnd *					createView();								// create a view of the port

	bool						importFile( const char * fileName );		// import from file
	void						shellOpen() const;							// open the source object through a shell program

	// TODO: Add data of this port here
};


#endif

//-------------------------------------------------------------------------------
// EOF
