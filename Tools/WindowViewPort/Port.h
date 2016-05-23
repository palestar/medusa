/*
	Port.h

	ResourcePort object
	(c)2005 Palestar, Richard Lyle
*/

#ifndef WINDOWVIEWPORT_H
#define WINDOWVIEWPORT_H

#include "Standard/Dll.h"
#include "Standard/SortedArray.h"
#include "Tools/NodeWindowPort/Port.h"

//-------------------------------------------------------------------------------

#undef DLL
#ifdef WINDOWVIEWPORT_DLL
#define DLL			DLL_EXPORT
#else
#define DLL			DLL_IMPORT
#endif

//----------------------------------------------------------------------------

class DLL WindowViewPort : public NodeWindowPort
{
public:
	DECLARE_WIDGET_CLASS();
	DECLARE_PROPERTY_LIST();

	// Types
	typedef Reference<WindowViewPort>	Ref;

	// Construction
	WindowViewPort();
	// Widget Interface
	bool						read( const InStream & input );
	// BaseNodePort Interface
	void						dependencies( DependentArray & dep );
	CFrameWnd *					createView();
	BaseNode *					createNode();
	void						initializeNode( BaseNode * pNode );

	// Data
	ClassKey					m_ViewClass;

private:
	// Helpers
	void						checkNames( BaseNodePort * pNode, 
									Tree< CharString, int > & names );
};


#endif

//-------------------------------------------------------------------------------
// EOF
