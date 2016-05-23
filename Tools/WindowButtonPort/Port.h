/*
	Port.h

	ResourcePort object
	(c)2005 Palestar, Richard Lyle
*/

#ifndef WINDOWBUTTONPORT_H
#define WINDOWBUTTONPORT_H

#include "Standard/Dll.h"
#include "Tools/NodeWindowPort/Port.h"

//-------------------------------------------------------------------------------

#undef DLL
#ifdef WINDOWBUTTONPORT_DLL
#define DLL			DLL_EXPORT
#else
#define DLL			DLL_IMPORT
#endif

//----------------------------------------------------------------------------

class DLL WindowButtonPort : public NodeWindowPort
{
public:
	DECLARE_WIDGET_CLASS();
	DECLARE_PROPERTY_LIST();

	// Types
	typedef Reference<WindowButtonPort>	Ref;

	// Construction
	WindowButtonPort();
	// Widget Interface
	bool						read( const InStream & input );
	// BaseNodePort Interface
	void						dependencies( DependentArray & dep );
	CFrameWnd *					createView();
	BaseNode *					createNode();
	void						initializeNode( BaseNode * pNode );

	// Data
	CharString					m_Label;
	CharString					m_Button;
	bool						m_Toggle;
	bool						m_ButtonDown;
	CharString					m_Tip;
	dword						m_HotKey;
};


#endif

//-------------------------------------------------------------------------------
// EOF
