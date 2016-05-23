/*
	Port.h

	ResourcePort object
	(c)2005 Palestar, Richard Lyle
*/

#ifndef NODEBOOLEANTIMEPORT_H
#define NODEBOOLEANTIMEPORT_H

#include "Standard/Dll.h"
#include "Tools/ScenePort/BaseNodePort.h"

//-------------------------------------------------------------------------------

#undef DLL
#ifdef NODEBOOLEANTIMEPORT_DLL
#define DLL			DLL_EXPORT
#else
#define DLL			DLL_IMPORT
#endif

//----------------------------------------------------------------------------

class DLL NodeBooleanTimePort : public BaseNodePort
{
public:
	DECLARE_WIDGET_CLASS();
	DECLARE_PROPERTY_LIST();

	// Types
	typedef Reference<NodeBooleanTimePort>	Ref;

	// Construction
	NodeBooleanTimePort();

	// Widget Interface
	bool						read( const InStream & input );
	// BaseNodePort Interface
	CFrameWnd *					createView();
	BaseNode *					createNode();
	void						initializeNode( BaseNode * pNode );

	float						m_Begin, m_End;
	bool						m_Invert;
	bool						m_WarpTime;
	bool						m_Loop;
	float						m_LoopTime;
};


#endif

//-------------------------------------------------------------------------------
// EOF
