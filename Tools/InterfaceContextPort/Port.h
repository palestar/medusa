/*
	Port.h

	Port object
	(c)2005 Palestar, Richard Lyle
*/

#ifndef INTERFACECONTEXTPORT_H
#define INTERFACECONTEXTPORT_H

#include "Standard/Dll.h"
#include "Tools/ResourcerDoc/Port.h"

//-------------------------------------------------------------------------------

#undef DLL
#ifdef INTERFACECONTEXTPORT_DLL
#define DLL		DLL_EXPORT
#else
#define DLL		DLL_IMPORT
#endif

//-------------------------------------------------------------------------------

class DLL InterfaceContextPort : public Port
{
public:
	DECLARE_WIDGET_CLASS();
	DECLARE_PROPERTY_LIST();

	typedef Reference<InterfaceContextPort>	Ref;

	// Construction
	InterfaceContextPort();
	// Widget Interface
	bool						read( const InStream & input );
	// Port interface
	void						dependencies( DependentArray & dep );		// get the depdendencies
	Resource::Ref				createResource();							// create the resource 
	CFrameWnd *					createView();								// create a view of the port

	bool						importFile( const char * fileName );		// import from file
	void						shellOpen() const;							// open the source object through a shell program
	
	ClassKey					m_DocumentClass;
	int							m_StartingScene;
	Array< CharString >
								m_SceneId;
	Array< CharString >
								m_Scenes;

	CharString					m_Style;
	CharString					m_Cursor;
};


#endif

//-------------------------------------------------------------------------------
// EOF
