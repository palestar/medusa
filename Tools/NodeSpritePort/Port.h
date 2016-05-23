/*
	Port.h

	ResourcePort object
	(c)2005 Palestar, Richard Lyle
*/

#ifndef NODESPRITEPORT_H
#define NODESPRITEPORT_H

#include "Standard/Dll.h"
#include "Render3d/NodeSprite.h"
#include "Tools/ScenePort/NodePort.h"

//-------------------------------------------------------------------------------

#undef DLL
#ifdef NODESPRITEPORT_DLL
#define DLL			DLL_EXPORT
#else
#define DLL			DLL_IMPORT
#endif

//----------------------------------------------------------------------------

class DLL NodeSpritePort : public NodePort
{
public:
	DECLARE_WIDGET_CLASS();
	DECLARE_PROPERTY_LIST();

	// Types
	typedef Reference<NodeSpritePort>		Ref;
	typedef NodeSprite::AxisLock			AxisLock;

	// Construction
	NodeSpritePort();
	// Widget Interface
	bool						read( const InStream & input );
	// NodePort Interface
	void						dependencies( DependentArray & dep );
	CFrameWnd *					createView();
	BaseNode *					createNode();
	void						initializeNode( BaseNode * pNode );

	bool						canScale();
	void						scale( int x, int y );

	// Data
	float						m_Scale;
	bool						m_ScaleLock;
	bool						m_ClampBack;
	bool						m_EnableAlpha;
	float						m_Alpha, m_AlphaV, m_AlphaA;
	AxisLock					m_AxisLock;
	CharString					m_MaterialPort;
};


#endif

//-------------------------------------------------------------------------------
// EOF
