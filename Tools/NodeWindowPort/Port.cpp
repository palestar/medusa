/*
	Port.cpp
	(c)2005 Palestar, Richard Lyle
*/

#define NODEWINDOWPORT_DLL
#include "stdafx.h"
#include "Port.h"

#include "Gui3d/NodeWindow.h"
#include "Tools/ScenePort/ChildFrame.h"
#include "Tools/ResourcerDoc/Port.h"

//----------------------------------------------------------------------------

IMPLEMENT_FACTORY( NodeWindowPort, BaseNodePort );
REGISTER_FACTORY_KEY(  NodeWindowPort, 3976979262574699418 );

BEGIN_PROPERTY_LIST( NodeWindowPort, BaseNodePort );
	ADD_BITS_PROPERTY( m_nStyle );				// Style
		ADD_BIT_OPTION( m_nStyle, NodeWindow::HAS_BACK );
		ADD_BIT_OPTION( m_nStyle, NodeWindow::CAN_MOVE );
		ADD_BIT_OPTION( m_nStyle, NodeWindow::CAN_SIZE );
		ADD_BIT_OPTION( m_nStyle, NodeWindow::CAN_SCROLL );
		ADD_BIT_OPTION( m_nStyle, NodeWindow::CAN_DOCK );
		ADD_BIT_OPTION( m_nStyle, NodeWindow::DOCUMENT );
		ADD_BIT_OPTION( m_nStyle, NodeWindow::CAN_FOCUS );
		ADD_BIT_OPTION( m_nStyle, NodeWindow::SMOOTH_UL );
		ADD_BIT_OPTION( m_nStyle, NodeWindow::SMOOTH_UR );
		ADD_BIT_OPTION( m_nStyle, NodeWindow::SMOOTH_LR );
		ADD_BIT_OPTION( m_nStyle, NodeWindow::SMOOTH_LL );
		ADD_BIT_OPTION( m_nStyle, NodeWindow::ANIM_BEAM );
		ADD_BIT_OPTION( m_nStyle, NodeWindow::ANIM_BACK );
		ADD_BIT_OPTION( m_nStyle, NodeWindow::ANIM_FLARE );
		ADD_BIT_OPTION( m_nStyle, NodeWindow::EFFECT_FADEIN );
		ADD_BIT_OPTION( m_nStyle, NodeWindow::EFFECT_HIGHLIGHT );
		ADD_BIT_OPTION( m_nStyle, NodeWindow::EFFECT_ROLLUV );
		ADD_BIT_OPTION( m_nStyle, NodeWindow::USER1 );
		ADD_BIT_OPTION( m_nStyle, NodeWindow::USER2 );
		ADD_BIT_OPTION( m_nStyle, NodeWindow::USER3 );
		ADD_BIT_OPTION( m_nStyle, NodeWindow::USER4 );
		ADD_BIT_OPTION( m_nStyle, NodeWindow::USER5 );
		ADD_BIT_OPTION( m_nStyle, NodeWindow::USER6 );
		ADD_BIT_OPTION( m_nStyle, NodeWindow::USER7 );
		ADD_BIT_OPTION( m_nStyle, NodeWindow::USER8 );
	ADD_BITS_PROPERTY( m_nFlags );
		ADD_BIT_OPTION( m_nFlags, NodeWindow::VISIBLE );
		ADD_BIT_OPTION( m_nFlags, NodeWindow::NOCLIP );
		ADD_BIT_OPTION( m_nFlags, NodeWindow::DOCKED );
		ADD_BIT_OPTION( m_nFlags, NodeWindow::DOCK_TOP_LEFT );
		ADD_BIT_OPTION( m_nFlags, NodeWindow::DOCK_TOP );
		ADD_BIT_OPTION( m_nFlags, NodeWindow::DOCK_TOP_RIGHT );
		ADD_BIT_OPTION( m_nFlags, NodeWindow::DOCK_RIGHT );
		ADD_BIT_OPTION( m_nFlags, NodeWindow::DOCK_BOTTOM_RIGHT );
		ADD_BIT_OPTION( m_nFlags, NodeWindow::DOCK_BOTTOM );
		ADD_BIT_OPTION( m_nFlags, NodeWindow::DOCK_BOTTOM_LEFT );
		ADD_BIT_OPTION( m_nFlags, NodeWindow::DOCK_OUTSIDE );
		ADD_BIT_OPTION( m_nFlags, NodeWindow::HIGHLIGHT );
		ADD_BIT_OPTION( m_nFlags, NodeWindow::HCENTER );
		ADD_BIT_OPTION( m_nFlags, NodeWindow::VCENTER );
		ADD_BIT_OPTION( m_nFlags, NodeWindow::PARENT_WIDTH );
		ADD_BIT_OPTION( m_nFlags, NodeWindow::PARENT_HEIGHT );
		ADD_BIT_OPTION( m_nFlags, NodeWindow::DISABLED );
	ADD_PROPERTY( m_Alpha );
	ADD_PROPERTY( m_Window );
	ADD_PROPERTY( m_Help );
	ADD_PORT_PROPERTY( m_WindowStyle, WindowStylePort );
	ADD_PROPERTY( m_ResourceLinks );
END_PROPERTY_LIST();

NodeWindowPort::NodeWindowPort() : BaseNodePort()
{
	m_Type = m_Class = CLASS_KEY( NodeWindow );
	m_Name = "Window";
	
	m_bHasBack = false;				// Style
	m_bCanMove = false;
	m_bCanSize = false;
	m_bCanScroll = false;
	m_bCanDock = false;
	m_bDocument = false;
	m_bCanFocus = false;
	m_bSmoothUL = false;
	m_bSmoothUR = false;
	m_bSmoothLR = false;
	m_bSmoothLL = false;
	m_bAnimBeam = false;
	m_bAnimBack = false;
	m_bAnimFlare = false;
	m_bEffectFadeIn = false;
	m_bEffectHighlight = false;
	m_bEffectRollUV = false;
	m_bUser1 = false;
	m_bUser2 = false;
	m_bUser3 = false;
	m_bUser4 = false;
	m_bUser5 = false;
	m_bUser6 = false;
	m_bUser7 = false;
	m_bUser8 = false;

	m_bVisible = true;				// Flags
	m_bNoClip = false;
	m_bDocked = false;
	m_bDockX = false;
	m_bDockY = false;
	m_bDockZ = false;
	m_bDockOutside = false;
	m_bParentSize = false;
	m_bParentW = false;
	m_bParentH = false;
	m_bCenterH = false;
	m_bCenterV = false;
	m_bDisabled = false;

	m_nStyle = 0;
	m_nFlags = NodeWindow::VISIBLE;
	m_Alpha = 1.0f;
	m_Window.set( 0,0,128,128 );
}

//-------------------------------------------------------------------------------

const int VERSION_102299 = 102299;
const int VERSION_102799 = 102799;
const int VERSION_110899 = 110899;
const int VERSION_121599 = 121599;
const int VERSION_121799 = 121799;
const int VERSION_031400 = 31400;
const int VERSION_031500 = 31500;
const int VERSION_072602 = 72602;
const int VERSION_20040514 = 20040514;
const int VERSION_20040630 = 20040630;

bool NodeWindowPort::read( const InStream & input )
{
	if (! BaseNodePort::read( input ) )
	{
		int version;
		input >> version;

		switch( version )
		{
		case VERSION_20040630:
			{
				input >> m_bHasBack;				// Style
				input >> m_bCanMove;
				input >> m_bCanSize;
				input >> m_bCanScroll;
				input >> m_bCanDock;
				input >> m_bDocument;
				input >> m_bCanFocus;
				input >> m_bSmoothUL;
				input >> m_bSmoothUR;
				input >> m_bSmoothLR;
				input >> m_bSmoothLL;
				input >> m_bAnimBeam;
				input >> m_bAnimBack;
				input >> m_bAnimFlare;
				input >> m_bEffectFadeIn;
				input >> m_bEffectHighlight;
				input >> m_bEffectRollUV;
				input >> m_bUser1;
				input >> m_bUser2;
				input >> m_bUser3;
				input >> m_bUser4;
				input >> m_bUser5;
				input >> m_bUser6;
				input >> m_bUser7;
				input >> m_bUser8;

				input >> m_bVisible;				// Flags
				input >> m_bNoClip;
				input >> m_bDocked;
				input >> m_bDockX;
				input >> m_bDockY;
				input >> m_bDockZ;
				input >> m_bDockOutside;
				input >> m_bParentSize;
				input >> m_bParentW;
				input >> m_bParentH;
				input >> m_bCenterH;
				input >> m_bCenterV;
				input >> m_bDisabled;

				input >> m_Alpha;
				input >> m_Window;
				input >> m_Help;
				input >> m_WindowStyle;
				input >> m_ResourceLinks;

				m_nStyle = exportStyle();
				m_nFlags = exportFlags();
			}
			break;
		case VERSION_20040514:
			{
				input >> m_bHasBack;				// Style
				input >> m_bCanMove;
				input >> m_bCanSize;
				input >> m_bCanScroll;
				input >> m_bCanDock;
				input >> m_bDocument;
				input >> m_bCanFocus;
				input >> m_bSmoothUL;
				input >> m_bSmoothUR;
				input >> m_bSmoothLR;
				input >> m_bSmoothLL;
				input >> m_bAnimBeam;
				input >> m_bAnimBack;
				input >> m_bAnimFlare;
				input >> m_bEffectFadeIn;
				input >> m_bEffectHighlight;
				input >> m_bEffectRollUV;
				input >> m_bUser1;
				input >> m_bUser2;
				input >> m_bUser3;
				input >> m_bUser4;
				input >> m_bUser5;
				input >> m_bUser6;
				input >> m_bUser7;
				input >> m_bUser8;

				input >> m_bVisible;				// Flags
				input >> m_bNoClip;
				input >> m_bDocked;
				input >> m_bDockX;
				input >> m_bDockY;
				input >> m_bDockZ;
				input >> m_bDockOutside;
				input >> m_bParentSize;
				input >> m_bParentW;
				input >> m_bParentH;
				input >> m_bCenterH;
				input >> m_bCenterV;
				input >> m_bDisabled;

				input >> m_Alpha;
				input >> m_Class;
				input >> m_Type;
				input >> m_Window;
				input >> m_Help;
				input >> m_WindowStyle;

				Array< CharString > id, port;
				input >> id;
				input >> port;

				for(int i=0;i<Min(id.size(),port.size());i++)
					m_ResourceLinks.push( CharString().format("%s;%s", id[i], port[i]) );

				m_nStyle = exportStyle();
				m_nFlags = exportFlags();
			}
			break;
		case VERSION_072602:
			{
				input >> m_nFlags;
				input >> m_nStyle;
				input >> m_Alpha;
				input >> m_Class;
				input >> m_Type;
				input >> m_Window;
				input >> m_Help;
				input >> m_WindowStyle;

				Array< CharString > id, port;
				input >> id;
				input >> port;

				for(int i=0;i<Min(id.size(),port.size());i++)
					m_ResourceLinks.push( CharString().format("%s;%s", id[i], port[i]) );
			}
			break;
		case VERSION_031500:
			{
				input >> m_nFlags;
				input >> m_nStyle;
				input >> m_Alpha;
				input >> m_Class;
				input >> m_Type;
				input >> m_Window;
				input >> m_WindowStyle;

				Array< CharString > id, port;
				input >> id;
				input >> port;

				for(int i=0;i<Min(id.size(),port.size());i++)
					m_ResourceLinks.push( CharString().format("%s;%s", id[i], port[i]) );
			}
			break;
		}

		return false;
	}

	return true;
}

//-------------------------------------------------------------------------------

void NodeWindowPort::dependencies( DependentArray & dep )
{
	BaseNodePort::dependencies( dep );
	for(int i=0;i<m_ResourceLinks.size();i++)
	{
		int nLink = m_ResourceLinks[i].find(';');
		if ( nLink >= 0 )
			dep.push( m_ResourceLinks[i].buffer() + nLink + 1 );
	}
	dep.push( m_WindowStyle );
}

CFrameWnd * NodeWindowPort::createView()
{ 
	return BaseNodePort::createView();
}

BaseNode * NodeWindowPort::createNode()
{
	return BaseNodePort::createNode();
}

void NodeWindowPort::initializeNode( BaseNode * pNode )
{
	BaseNodePort::initializeNode( pNode );

	NodeWindow * pWindow = WidgetCast<NodeWindow>( pNode );
	if ( pWindow != NULL )
	{
		pWindow->setFlags( m_nFlags );
		pWindow->setStyle( m_nStyle );
		pWindow->setAlpha( m_Alpha );
		pWindow->setWindow( m_Window );
		pWindow->setHelp( m_Help );
		pWindow->setWindowStyle( WindowStyle::Link( Port::portResource( m_WindowStyle ) ) );

		for(int i=0;i<m_ResourceLinks.size();i++)
		{
			CharString sLink = m_ResourceLinks[ i ];
			if ( sLink.length() == 0 )
				continue;

			CharString sID;
			if (! sLink.tokenize( sID, ";" ) )
				continue;

			pWindow->addResource( sID, Port::portResource( sLink ) );
		}
	}
}

bool NodeWindowPort::canMove()
{
	return true;
}

const float MOVE_SCALE = 0.05f;

void NodeWindowPort::moveXY( int x, int y )
{
	m_Window += PointInt( x, y );

	NodeWindow * pWindow = WidgetCast<NodeWindow>( getCachedNode() );
	if ( pWindow != NULL )
	{
		pWindow->setWindow( m_Window );
		UpdateAllViews(NULL);
	}
}

bool NodeWindowPort::canScale()
{
	return true;
}

const float SCALE_RATE = 0.01f;

void NodeWindowPort::scale( int x, int y )
{
	m_Window.set( m_Window.left, m_Window.top, 
		m_Window.right + x, 
		m_Window.bottom + y );

	NodeWindow * pWindow = WidgetCast<NodeWindow>( getCachedNode() );
	if ( pWindow != NULL )
	{
		pWindow->setWindow( m_Window );
		UpdateAllViews(NULL);
	}
}

//----------------------------------------------------------------------------

void NodeWindowPort::importStyle( dword nStyle )
{
	m_bHasBack = (nStyle & NodeWindow::HAS_BACK) ? true : false;
	m_bCanMove = (nStyle & NodeWindow::CAN_MOVE) ? true : false;
	m_bCanSize = (nStyle & NodeWindow::CAN_SIZE) ? true : false;
	m_bCanScroll = (nStyle & NodeWindow::CAN_SCROLL) ? true : false;
	m_bCanDock = (nStyle & NodeWindow::CAN_DOCK) ? true : false;
	m_bDocument = (nStyle & NodeWindow::DOCUMENT) ? true : false;
	m_bCanFocus = (nStyle & NodeWindow::CAN_FOCUS) ? true : false;
	m_bSmoothUL = (nStyle & NodeWindow::SMOOTH_UL) ? true : false;
	m_bSmoothUR = (nStyle & NodeWindow::SMOOTH_UR) ? true : false;
	m_bSmoothLR = (nStyle & NodeWindow::SMOOTH_LR) ? true : false;
	m_bSmoothLL = (nStyle & NodeWindow::SMOOTH_LL) ? true : false;
	m_bAnimBeam = (nStyle & NodeWindow::ANIM_BEAM) ? true : false;
	m_bAnimBack = (nStyle & NodeWindow::ANIM_BACK) ? true : false;
	m_bAnimFlare = (nStyle & NodeWindow::ANIM_FLARE) ? true : false;
	m_bEffectFadeIn = (nStyle & NodeWindow::EFFECT_FADEIN) ? true : false;
	m_bEffectHighlight = (nStyle & NodeWindow::EFFECT_HIGHLIGHT) ? true : false;
	m_bEffectRollUV = (nStyle & NodeWindow::EFFECT_ROLLUV) ? true : false;
	m_bUser1 = (nStyle & NodeWindow::USER1) ? true : false;
	m_bUser2 = (nStyle & NodeWindow::USER2) ? true : false;
	m_bUser3 = (nStyle & NodeWindow::USER3) ? true : false;
	m_bUser4 = (nStyle & NodeWindow::USER4) ? true : false;
	m_bUser5 = (nStyle & NodeWindow::USER5) ? true : false;
	m_bUser6 = (nStyle & NodeWindow::USER6) ? true : false;
	m_bUser7 = (nStyle & NodeWindow::USER7) ? true : false;
	m_bUser8 = (nStyle & NodeWindow::USER8) ? true : false;
}

void NodeWindowPort::importFlags( dword nFlags )
{
	m_bVisible = (nFlags & NodeWindow::VISIBLE) ? true : false;
	m_bNoClip = (nFlags & NodeWindow::NOCLIP) ? true : false;
	m_bDocked = (nFlags & NodeWindow::DOCKED) ? true : false;
	m_bDockX = (nFlags & NodeWindow::DOCK_X) ? true : false;
	m_bDockY = (nFlags & NodeWindow::DOCK_Y) ? true : false;
	m_bDockZ = (nFlags & NodeWindow::DOCK_Z) ? true : false;
	m_bDockOutside = (nFlags & NodeWindow::DOCK_OUTSIDE) ? true : false;
	m_bParentSize = (nFlags & NodeWindow::PARENT_SIZE) ? true : false;
	m_bParentW = (nFlags & NodeWindow::PARENT_WIDTH) ? true : false;
	m_bParentH = (nFlags & NodeWindow::PARENT_HEIGHT) ? true : false;
	m_bCenterH = (nFlags & NodeWindow::HCENTER) ? true : false;
	m_bCenterV = (nFlags & NodeWindow::VCENTER) ? true : false;
	m_bDisabled = (nFlags & NodeWindow::DISABLED) ? true : false;
}

dword NodeWindowPort::exportStyle()
{
	dword nStyle = 0;
	nStyle |= m_bHasBack ? NodeWindow::HAS_BACK : 0;
	nStyle |= m_bCanMove ? NodeWindow::CAN_MOVE : 0;
	nStyle |= m_bCanSize ? NodeWindow::CAN_SIZE : 0;
	nStyle |= m_bCanScroll ? NodeWindow::CAN_SCROLL : 0;
	nStyle |= m_bCanDock ? NodeWindow::CAN_DOCK : 0;
	nStyle |= m_bDocument ? NodeWindow::DOCUMENT : 0;
	nStyle |= m_bCanFocus ? NodeWindow::CAN_FOCUS : 0;
	nStyle |= m_bSmoothUL ? NodeWindow::SMOOTH_UL : 0;
	nStyle |= m_bSmoothUR ? NodeWindow::SMOOTH_UR : 0;
	nStyle |= m_bSmoothLR ? NodeWindow::SMOOTH_LR : 0;
	nStyle |= m_bSmoothLL ? NodeWindow::SMOOTH_LL : 0;
	nStyle |= m_bAnimBeam ? NodeWindow::ANIM_BEAM : 0;
	nStyle |= m_bAnimBack ? NodeWindow::ANIM_BACK : 0;
	nStyle |= m_bAnimFlare ? NodeWindow::ANIM_FLARE : 0;
	nStyle |= m_bEffectFadeIn ? NodeWindow::EFFECT_FADEIN : 0;
	nStyle |= m_bEffectHighlight ? NodeWindow::EFFECT_HIGHLIGHT : 0;
	nStyle |= m_bEffectRollUV ? NodeWindow::EFFECT_ROLLUV : 0;
	nStyle |= m_bUser1 ? NodeWindow::USER1 : 0;
	nStyle |= m_bUser2 ? NodeWindow::USER2 : 0;
	nStyle |= m_bUser3 ? NodeWindow::USER3 : 0;
	nStyle |= m_bUser4 ? NodeWindow::USER4 : 0;
	nStyle |= m_bUser5 ? NodeWindow::USER5 : 0;
	nStyle |= m_bUser6 ? NodeWindow::USER6 : 0;
	nStyle |= m_bUser7 ? NodeWindow::USER7 : 0;
	nStyle |= m_bUser8 ? NodeWindow::USER8 : 0;

	return nStyle;
}

dword NodeWindowPort::exportFlags()
{
	dword nFlags = 0;
	nFlags |= m_bVisible ? NodeWindow::VISIBLE : 0;
	nFlags |= m_bNoClip ? NodeWindow::NOCLIP : 0;
	nFlags |= m_bDocked ? NodeWindow::DOCKED : 0;
	nFlags |= m_bDockX ? NodeWindow::DOCK_X : 0;
	nFlags |= m_bDockY ? NodeWindow::DOCK_Y : 0;
	nFlags |= m_bDockZ ? NodeWindow::DOCK_Z : 0;
	nFlags |= m_bDockOutside ? NodeWindow::DOCK_OUTSIDE : 0;
	nFlags |= m_bCenterH ? NodeWindow::HCENTER : 0;
	nFlags |= m_bCenterV ? NodeWindow::VCENTER : 0;
	nFlags |= m_bParentSize ? NodeWindow::PARENT_SIZE : 0;
	nFlags |= m_bParentW ? NodeWindow::PARENT_WIDTH : 0;
	nFlags |= m_bParentH ? NodeWindow::PARENT_HEIGHT : 0;
	nFlags |= m_bDisabled ? NodeWindow::DISABLED : 0;

	return nFlags;
}


//-------------------------------------------------------------------------------
// EOF

