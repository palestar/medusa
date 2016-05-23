/*
	Port.h

	ResourcePort object
	(c)2005 Palestar, Richard Lyle
*/

#ifndef NODESPINPORT_H
#define NODESPINPORT_H

#include "Standard/Dll.h"
#include "Render3d/NodeSpin.h"

#include "Tools/ScenePort/NodePort.h"

//-------------------------------------------------------------------------------

#undef DLL
#ifdef NODESPINPORT_DLL
#define DLL			DLL_EXPORT
#else
#define DLL			DLL_IMPORT
#endif

//----------------------------------------------------------------------------

class DLL NodeSpinPort : public NodePort
{
public:
	DECLARE_WIDGET_CLASS();
	DECLARE_PROPERTY_LIST();

	// Construction
	NodeSpinPort();
	// Widget Interface
	bool						read( const InStream & input );
	// NodePort Interface
	CFrameWnd *					createView();
	BaseNode *					createNode();
	void						initializeNode( BaseNode * pNode );

	float						m_Pitch,m_Yaw,m_Roll;
};


#endif

//-------------------------------------------------------------------------------
// EOF
