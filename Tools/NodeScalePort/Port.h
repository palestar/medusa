/*
	Port.h

	ResourcePort object
	(c)2005 Palestar, Richard Lyle
*/

#ifndef NODESCALEPORT_H
#define NODESCALEPORT_H

#include "Standard/Dll.h"
#include "Tools/ScenePort/NodePort.h"

//-------------------------------------------------------------------------------

#undef DLL
#ifdef NODESCALEPORT_DLL
#define DLL			DLL_EXPORT
#else
#define DLL			DLL_IMPORT
#endif

//----------------------------------------------------------------------------

class DLL NodeScalePort : public NodePort
{
public:
	DECLARE_WIDGET_CLASS();
	DECLARE_PROPERTY_LIST();

	// Types
	typedef Reference<NodeScalePort>	Ref;

	// Construction
	NodeScalePort();
	// Widget Interface
	bool						read( const InStream & input );
	// NodePort Interface
	CFrameWnd *					createView();
	BaseNode *					createNode();
	void						initializeNode( BaseNode * pNode );

	float						m_XV, m_XA;
	float						m_YV, m_YA;
	float						m_ZV, m_ZA;
};


#endif

//-------------------------------------------------------------------------------
// EOF
