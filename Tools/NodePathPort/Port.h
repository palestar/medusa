/*
	Port.h

	ResourcePort object
	(c)2005 Palestar, Richard Lyle
*/

#ifndef NODEPATHPORT_H
#define NODEPATHPORT_H

#include "Standard/Dll.h"
#include "Render3d/NodePath.h"
#include "Tools/ScenePort/NodePort.h"

//-------------------------------------------------------------------------------

#undef DLL
#ifdef NODEPATHPORT_DLL
#define DLL			DLL_EXPORT
#else
#define DLL			DLL_IMPORT
#endif

//----------------------------------------------------------------------------

class DLL NodePathPort : public NodePort
{
public:
	DECLARE_WIDGET_CLASS();
	DECLARE_PROPERTY_LIST();

	// Types
	typedef Reference<NodePathPort>	Ref;

	// Construction
	NodePathPort();
	// Widget Interface
	bool						read( const InStream & input );
	// NodePort Interface
	CFrameWnd *					createView();
	BaseNode *					createNode();
	void						initializeNode( BaseNode * pNode );

	// Data
	Vector3						m_Velocity;
	Vector3						m_Acceleration;
};


#endif

//-------------------------------------------------------------------------------
// EOF
