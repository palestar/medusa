/*
	Port.h

	Port object
	(c)2005 Palestar, Richard Lyle
*/

#ifndef FONTPORT_H
#define FONTPORT_H

#include "Tools/ResourcerDoc/Port.h"

//-------------------------------------------------------------------------------

#undef DLL
#ifdef FONTPORT_DLL
#define DLL		DLL_EXPORT
#else
#define DLL		DLL_IMPORT
#endif

//-------------------------------------------------------------------------------

class DLL FontPort : public Port
{
public:
	DECLARE_WIDGET_CLASS(); 
	DECLARE_PROPERTY_LIST();

	// Construction
	FontPort();
	// Widget Interface
	bool						read( const InStream & input );
	// Port interface
	void						dependencies( DependentArray & dep );		// get the depdendencies
	Resource::Ref				createResource();							// create the resource 
	CFrameWnd *					createView();								// create a view of the port

	bool						importFile( const char * fileName );		// import from file
	void						shellOpen() const;							// open the source object through a shell program

	// Data
	LOGFONT						m_LogicalFont;
};


#endif

//-------------------------------------------------------------------------------
// EOF
