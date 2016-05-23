/*
	Port.h

	ResourcePort object
	(c)2005 Palestar, Richard Lyle
*/

#ifndef NODEBLURPORT_H
#define NODEBLURPORT_H

#include "Standard/Dll.h"
#include "Tools/ScenePort/BaseNodePort.h"
#include "Render3d/NodeBlur.h"

//-------------------------------------------------------------------------------

#undef DLL
#ifdef NODEBLURPORT_DLL
#define DLL			DLL_EXPORT
#else
#define DLL			DLL_IMPORT
#endif

//----------------------------------------------------------------------------

class DLL NodeBlurPort : public BaseNodePort
{
public:
	DECLARE_WIDGET_CLASS();
	DECLARE_PROPERTY_LIST();

	// Types
	typedef Reference<NodeBlurPort>	Ref;
	typedef NodeBlur::Blending		Blending;

	// Construction
	NodeBlurPort();
	// Widget Interface
	bool						read( const InStream & input );
	// BaseNodePort Interface
	CFrameWnd *					createView();
	BaseNode *					createNode();
	void						initializeNode( BaseNode * pNode );

	// Data
	float						m_Time;
	int							m_Steps;
	Blending					m_Blending;
};


#endif

//-------------------------------------------------------------------------------
// EOF
