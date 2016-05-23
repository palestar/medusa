/*
	Port.h

	ResourcePort object
	(c)2005 Palestar, Richard Lyle
*/

#ifndef NODEBOOLEANPORT_H
#define NODEBOOLEANPORT_H

#include "Tools/ScenePort/BaseNodePort.h"
#include "Render3d/NodeBoolean.h"

//---------------------------------------------------------------------------------------------------

#undef DLL
#ifdef NODEBOOLEANPORT_DLL
#define DLL			DLL_EXPORT
#else
#define DLL			DLL_IMPORT
#endif

//-------------------------------------------------------------------------------

class DLL NodeBooleanPort : public BaseNodePort
{
public:
	DECLARE_WIDGET_CLASS();
	DECLARE_PROPERTY_LIST();

	// Types
	typedef NodeBoolean::Type			Type;
	typedef NodeBoolean::DetailLevel	DetailLevel;

	// Construction
	NodeBooleanPort();
	// Widget Interface
	bool						read( const InStream & input );
	// NodePort Interface
	CFrameWnd *					createView();
	BaseNode *					createNode();
	void						initializeNode( BaseNode * pNode );

	// Data
	dword						m_Bits;
	Type						m_Type;
	float						m_MinDistance, m_MaxDistance;
	float						m_fFadePercent;
	DetailLevel					m_MinDetailLevel, m_MaxDetailLevel;
	bool						m_bZeroTime;
};


#endif

//-------------------------------------------------------------------------------
// EOF
