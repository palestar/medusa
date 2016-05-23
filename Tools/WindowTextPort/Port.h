/*
	Port.h

	ResourcePort object
	(c)2005 Palestar, Richard Lyle
*/

#ifndef WINDOWTEXTPORT_H
#define WINDOWTEXTPORT_H

#include "Standard/Dll.h"
#include "Tools/NodeWindowPort/Port.h"

//-------------------------------------------------------------------------------

#undef DLL
#ifdef WINDOWTEXTPORT_DLL
#define DLL			DLL_EXPORT
#else
#define DLL			DLL_IMPORT
#endif

//----------------------------------------------------------------------------

class DLL WindowTextPort : public NodeWindowPort
{
public:
	DECLARE_WIDGET_CLASS();
	DECLARE_PROPERTY_LIST();

	// Types
	typedef Reference<WindowTextPort>	Ref;

	// Construction
	WindowTextPort();
	// Widget Interface
	bool						read( const InStream & input );
	// BaseNodePort Interface
	void						dependencies( DependentArray & dep );
	CFrameWnd *					createView();
	BaseNode *					createNode();
	void						initializeNode( BaseNode * pNode );

	CharString					m_Text;
};


#endif

//-------------------------------------------------------------------------------
// EOF
