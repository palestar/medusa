/*
	Port.h

	ResourcePort object
	(c)2005 Palestar, Richard Lyle
*/

#ifndef NODEDECALPORT_H
#define NODEDECALPORT_H

#include "Standard/Dll.h"
#include "Tools/ScenePort/NodePort.h"

//-------------------------------------------------------------------------------

#undef DLL
#ifdef NODEDECALPORT_DLL
#define DLL			DLL_EXPORT
#else
#define DLL			DLL_IMPORT
#endif

//----------------------------------------------------------------------------

class DLL NodeDecalPort : public NodePort
{
public:
	DECLARE_WIDGET_CLASS();
	DECLARE_PROPERTY_LIST();

	// Types
	typedef Reference<NodeDecalPort>	Ref;

	// Construction
	NodeDecalPort();
	// Widget Interface
	bool						read( const InStream & input );
	// BaseNodePort Interface
	void						dependencies( DependentArray & dep );
	CFrameWnd *					createView();
	BaseNode *					createNode();
	void						initializeNode( BaseNode * pNode );

	CharString					m_Material;
};


#endif

//-------------------------------------------------------------------------------
// EOF
