/*
	Port.h

	ResourcePort object
	(c)2005 Palestar, Richard Lyle
*/

#ifndef NODEDETAILPORT_H
#define NODEDETAILPORT_H

#include "Standard/Dll.h"
#include "Tools/ScenePort/BaseNodePort.h"

//-------------------------------------------------------------------------------

#undef DLL
#ifdef NODEDETAILPORT_DLL
#define DLL			DLL_EXPORT
#else
#define DLL			DLL_IMPORT
#endif

//----------------------------------------------------------------------------

class DLL NodeDetailPort : public BaseNodePort
{
public:
	DECLARE_WIDGET_CLASS();
	DECLARE_PROPERTY_LIST();

	// Types
	typedef Reference<NodeDetailPort>	Ref;

	// Construction
	NodeDetailPort();
	// Widget Interface
	bool						read( const InStream & input );
	// BaseNodePort Interface
	void						dependencies( DependentArray & dep );
	CFrameWnd *					createView();
	BaseNode *					createNode();
	void						initializeNode( BaseNode * pNode );

	float						m_Distance;
};


#endif

//-------------------------------------------------------------------------------
// EOF
