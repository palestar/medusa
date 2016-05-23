/*
	Port.h

	ResourcePort object
	(c)2005 Palestar, Richard Lyle
*/

#ifndef NODEGLAREPORT_H
#define NODEGLAREPORT_H

#include "Standard/Dll.h"
#include "Tools/ScenePort/BaseNodePort.h"

//-------------------------------------------------------------------------------

#undef DLL
#ifdef NODEGLAREPORT_DLL
#define DLL			DLL_EXPORT
#else
#define DLL			DLL_IMPORT
#endif

//----------------------------------------------------------------------------

class DLL NodeGlarePort : public BaseNodePort
{
public:
	DECLARE_WIDGET_CLASS();
	DECLARE_PROPERTY_LIST();

	// Types
	typedef Reference<NodeGlarePort>	Ref;

	// Construction
	NodeGlarePort();
	// Widget Interface
	bool						read( const InStream & input );
	// BaseNodePort Interface
	void						dependencies( DependentArray & dep );
	CFrameWnd *					createView();
	BaseNode *					createNode();
	void						initializeNode( BaseNode * pNode );
	// Data
	dword						m_Color;
	float						m_Intensity;
	float						m_IntensityV;
	float						m_IntensityA;
	float						m_Focus;
	float						m_Range;
};


#endif

//-------------------------------------------------------------------------------
// EOF
