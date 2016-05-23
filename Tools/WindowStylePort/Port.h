/*
	Port.h

	Port object
	(c)2005 Palestar, Richard Lyle
*/

#ifndef WINDOWSTYLEPORT_H
#define WINDOWSTYLEPORT_H

#include "Standard/Dll.h"
#include "Tools/ResourcerDoc/Port.h"

//-------------------------------------------------------------------------------

#undef DLL
#ifdef WINDOWSTYLEPORT_DLL
#define DLL		DLL_EXPORT
#else
#define DLL		DLL_IMPORT
#endif

//-------------------------------------------------------------------------------

class DLL WindowStylePort : public Port
{
public:
	DECLARE_WIDGET_CLASS();
	DECLARE_PROPERTY_LIST();

	typedef Reference<WindowStylePort>	Ref;
	typedef Color						Color;

	// Construction
	WindowStylePort();
	// Widget Interface
	bool						read( const InStream & input );
	// Port Interface
	bool						autoLoad();									// enable autoload for this resource
	void						dependencies( DependentArray & dep );		// get the depdendencies
	Resource::Ref 				createResource();							// create the resource 
	CFrameWnd *					createView();								// create a view of the port

	bool						importFile( const char * fileName );		// import from file
	void						shellOpen() const;							// open the source object through a shell program

	CharString					m_Font;
	CharString					m_LargeFont;
	CharString					m_SmallFont;

	Color						m_Color;
	Color						m_HighColor;
	Color						m_BorderColor;
	Color						m_BackColor;
	Color						m_ScrollBarColor;
	Color						m_ScrollTabColor;

	CharString					m_Background;
	int							m_BorderSize;
	int							m_ScrollSize;
	CharString					m_HoverSound;
	CharString					m_SelectSound;
};


#endif

//-------------------------------------------------------------------------------
// EOF
