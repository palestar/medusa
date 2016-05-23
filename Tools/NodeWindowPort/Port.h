/*
	Port.h

	ResourcePort object
	(c)2005 Palestar, Richard Lyle
*/

#ifndef NODEWINDOWPORT_H
#define NODEWINDOWPORT_H

#include "Standard/Dll.h"
#include "Gui3d/NodeWindow.h"
#include "Tools/ScenePort/BaseNodePort.h"

//-------------------------------------------------------------------------------

#undef DLL
#ifdef NODEWINDOWPORT_DLL
#define DLL			DLL_EXPORT
#else
#define DLL			DLL_IMPORT
#endif

//----------------------------------------------------------------------------

class DLL NodeWindowPort : public BaseNodePort
{
public:
	DECLARE_WIDGET_CLASS();
	DECLARE_PROPERTY_LIST();

	// Types
	typedef Reference<NodeWindowPort>	Ref;

	// Construction
	NodeWindowPort();
	// Widget Interface
	bool						read( const InStream & input );
	// BaseNodePort Interface
	void						dependencies( DependentArray & dep );
	CFrameWnd *					createView();
	BaseNode *					createNode();
	void						initializeNode( BaseNode * pNode );

	bool						canMove();
	void						moveXY( int x, int y );
	bool						canScale();
	void						scale( int x, int y );

	// Data
	dword						m_nStyle;
	dword						m_nFlags;
	float						m_Alpha;

	RectInt						m_Window;
	CharString					m_Help;
	CharString					m_WindowStyle;

	Array< CharString >			m_ResourceLinks;

	// depricated
	bool						m_bHasBack;				// Style
	bool						m_bCanMove;
	bool						m_bCanSize;
	bool						m_bCanScroll;
	bool						m_bCanDock;
	bool						m_bDocument;
	bool						m_bCanFocus;
	bool						m_bSmoothUL;
	bool						m_bSmoothUR;
	bool						m_bSmoothLR;
	bool						m_bSmoothLL;
	bool						m_bAnimBeam;
	bool						m_bAnimBack;
	bool						m_bAnimFlare;
	bool						m_bEffectFadeIn;
	bool						m_bEffectHighlight;
	bool						m_bEffectRollUV;
	bool						m_bUser1;
	bool						m_bUser2;
	bool						m_bUser3;
	bool						m_bUser4;
	bool						m_bUser5;
	bool						m_bUser6;
	bool						m_bUser7;
	bool						m_bUser8;

	bool						m_bVisible;				// Flags
	bool						m_bNoClip;
	bool						m_bDocked;
	bool						m_bDockX;
	bool						m_bDockY;
	bool						m_bDockZ;
	bool						m_bDockOutside;
	bool						m_bParentSize;
	bool						m_bParentW;
	bool						m_bParentH;
	bool						m_bCenterH;
	bool						m_bCenterV;
	bool						m_bDisabled;
	
	// helpers
	void						importStyle( dword nStyle );
	void						importFlags( dword nFlags );
	dword						exportStyle();
	dword						exportFlags();
};


#endif

//-------------------------------------------------------------------------------
// EOF
