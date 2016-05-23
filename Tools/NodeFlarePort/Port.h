/*
	Port.h

	ResourcePort object
	(c)2005 Palestar, Richard Lyle
*/

#ifndef NODEFLAREPORT_H
#define NODEFLAREPORT_H

#include "Standard/Dll.h"
#include "Tools/ScenePort/NodePort.h"

//-------------------------------------------------------------------------------

#undef DLL
#ifdef NODEFLAREPORT_DLL
#define DLL			DLL_EXPORT
#else
#define DLL			DLL_IMPORT
#endif

//----------------------------------------------------------------------------

class DLL NodeFlarePort : public NodePort
{
public:
	DECLARE_WIDGET_CLASS();
	DECLARE_PROPERTY_LIST();

	// Types
	typedef Reference<NodeFlarePort>	Ref;

	// Construction
	NodeFlarePort();
	// Widget Interface
	bool						read( const InStream & input );
	// NodePort Interface
	void						dependencies( DependentArray & dep );
	CFrameWnd *					createView();
	BaseNode *					createNode();
	void						initializeNode( BaseNode * pNode );

	// Data
	CharString					m_MaterialPort;
	float						m_Scale;
};


#endif

//-------------------------------------------------------------------------------
// EOF
