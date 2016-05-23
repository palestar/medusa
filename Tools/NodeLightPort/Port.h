/*
	Port.h

	ResourcePort object
	(c)2005 Palestar, Richard Lyle
*/

#ifndef NODELIGHTPORT_H
#define NODELIGHTPORT_H

#include "Standard/Dll.h"
#include "Render3d/NodeLight.h"
#include "Tools/ScenePort/NodePort.h"

//-------------------------------------------------------------------------------

#undef DLL
#ifdef NODELIGHTPORT_DLL
#define DLL			DLL_EXPORT
#else
#define DLL			DLL_IMPORT
#endif

//----------------------------------------------------------------------------

class DLL NodeLightPort : public NodePort
{
public:
	DECLARE_WIDGET_CLASS();
	DECLARE_PROPERTY_LIST();

	// Types
	typedef Reference<NodeLightPort>	Ref;
	typedef NodeLight::Type				Type;
	typedef NodeLight::Effect			Effect;

	// Construction
	NodeLightPort();
	// Widget Interface
	bool						read( const InStream & input );
	// NodePort Interface
	CFrameWnd *					createView();
	BaseNode *					createNode();
	void						initializeNode( BaseNode * pNode );

	// Data
	Type						m_Type;
	Color						m_Color;
	float						m_Distance;
	Effect						m_Effect;
	float						m_EffectInterval;
};


#endif

//-------------------------------------------------------------------------------
// EOF
