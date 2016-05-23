/*
	Port.h

	Port object
	(c)2005 Palestar, Richard Lyle
*/

#ifndef TEXTPORT_H
#define TEXTPORT_H

#include "Standard/Dll.h"
#include "Resource/Text.h"
#include "Tools/ResourcerDoc/Port.h"

//-------------------------------------------------------------------------------

#undef DLL
#ifdef TEXTPORT_DLL
#define DLL		DLL_EXPORT
#else
#define DLL		DLL_IMPORT

#ifdef _DEBUG
#pragma comment(lib,"TextPortD.lib") 
#else
#pragma comment(lib,"TextPort.lib") 
#endif

#endif

//-------------------------------------------------------------------------------

class DLL TextPort : public Port
{
public:
	DECLARE_WIDGET_CLASS();
	DECLARE_PROPERTY_LIST();

	typedef Reference<TextPort>	Ref;

	// Construction
	TextPort();
	// Widget Interface
	bool						read( const InStream & input );
	// Port Interface
	bool						autoLoad();									// enable autoload for this resource
	void						dependencies( DependentArray & dep );		// get the depdendencies
	Resource::Ref 				createResource();							// create the resource 
	CFrameWnd *					createView();								// create a view of the port

	bool						importFile( const char * fileName );		// import from file
	void						shellOpen() const;							// open the source object through a shell program

	CharString					m_Text;
};


#endif

//-------------------------------------------------------------------------------
// EOF
