/*
	WindowTip.cpp
	(c)2005 Palestar, Richard Lyle
*/

#define GUI3D_DLL
#include "Debug/Assert.h"
#include "System/Platform.h"
#include "Gui3d/WindowTip.h"

//----------------------------------------------------------------------------

const float TIP_ALPHA = 0.75f;

dword		WindowTip::s_TipStyle = HAS_BACK | SMOOTH_UL | SMOOTH_UR | SMOOTH_LR | SMOOTH_LL | USER2;
int			WindowTip::s_TipBorderSize = 2;
Color		WindowTip::s_TipBorderColor = BLACK;
Color		WindowTip::s_TipBackColor = BLACK;
Color		WindowTip::s_TipTextColor = WHITE;

//---------------------------------------------------------------------------------------------------

IMPLEMENT_FACTORY( WindowTip, WindowText );
REGISTER_FACTORY_KEY(  WindowTip, 4898646345827286529 );
IMPLEMENT_FACTORY( NodeTip, BaseNode );
REGISTER_FACTORY_KEY(  NodeTip, 4898646466177704342 );

//-------------------------------------------------------------------------------

WindowTip::WindowTip()
{}

WindowTip::WindowTip( NodeWindow * pParent, const char * pTip, int iMaxWidth /*= 150*/  )
{
	ASSERT( pTip && pParent );

	if ( getWindowTip( pParent ) != NULL )
		throw TipAlreadyAttached();

	m_wpParent = pParent;
	m_Style |= s_TipStyle;
	m_Alpha = 0.0f;

	setText( pTip );
	setContext( pParent->context() );
	setWindowStyle( new WindowStyle( *pParent->windowStyle() ) );
	windowStyle()->setBorderSize( s_TipBorderSize );
	windowStyle()->setBorderColor( s_TipBorderColor );
	windowStyle()->setBackColor( s_TipBackColor );
	windowStyle()->setColor( s_TipTextColor );
	windowStyle()->setHighColor( s_TipTextColor );

	SizeInt textSize( windowStyle()->font()->size( WideString( pTip ) ) );
	textSize.width += (windowStyle()->borderSize() * 2);

	int iWidth = Min( textSize.width + 5, iMaxWidth );
	setWindow( RectInt( PointInt(0,0), SizeInt( iWidth, 1 ) ) );

	updateWindowPosition();

	pParent->attachNode( new NodeTip( this ) );				// attach helper class to the window we're watching
	pParent->rootWindow()->attachNode( this );				// attach ourselves to the root window
}

WindowTip::~WindowTip()
{
	if ( m_wpParent.valid() )
	{
		// detach the NodeTip object from our parent
		for(int i=0;i<m_wpParent->childCount();i++)
		{
			NodeTip * pTip = WidgetCast<NodeTip>( m_wpParent->child(i) );
			if ( pTip != NULL && pTip->m_pTip == this )
			{
				pTip->m_pTip = NULL;
				m_wpParent->detachNode( i );
				break;
			}
		}
	}
}

//-------------------------------------------------------------------------------


void WindowTip::onUpdate( float t )
{
	// fade window in
	if ( alpha() < TIP_ALPHA )
		setAlpha( Min( alpha() + t + t, TIP_ALPHA ) );

	if ( m_wpParent.valid() && m_wpParent->parent() != NULL )
	{
		// update our window position
		updateWindowPosition();

		// get the current cursor position
		PointInt screenPosition( Platform::sm_pPlatform->cursorPosition() );

		// convert to our parent window coordinates
		PointInt parentPosition( m_wpParent->screenToWindow( screenPosition ) );
		RectInt parentWindow( m_wpParent->localWindow() );
		if ( ! parentWindow.inRect( parentPosition ) )
			destroy();	// cursor no longer in parent window
	}
	else
		destroy();		// our parent window was detached
}

//----------------------------------------------------------------------------

WindowTip * WindowTip::getWindowTip( NodeWindow * pCheck )
{
	ASSERT( pCheck );

	for(int i=0;i<pCheck->childCount();i++)
	{
		NodeTip * pTip = WidgetCast<NodeTip>( pCheck->child(i) );
		if ( pTip != NULL )
			return pTip->m_pTip;
	}

	// no tool tip found
	return NULL;
}

//----------------------------------------------------------------------------

void WindowTip::updateWindowPosition()
{
	NodeWindow * pRoot = m_wpParent->rootWindow();
	if ( pRoot != NULL )
	{
		PointInt parentUpperLeft( m_wpParent->windowToScreen( PointInt( 0, 0 ) ) );
		RectInt parentWindow( parentUpperLeft, m_wpParent->window().size() );
		RectInt rootWindow( pRoot->window() );
		PointInt rootCenter( rootWindow.center() );

		SizeInt mySize( window().size() );

		// dock this window to it's parent, based on it's location in the full screen
		if ( parentUpperLeft.m_X > rootCenter.m_X )
		{
			// tip to the left
			if ( parentUpperLeft.m_Y > rootCenter.m_Y )			// tip upper left
				setWindow( RectInt( parentWindow.upperLeft() + PointInt( -mySize.width, -mySize.height), mySize ) );
			else												// tip lower left
				setWindow( RectInt( parentWindow.lowerLeft() + PointInt( -mySize.width, 0), mySize) );
		}
		else
		{
			// tip right
			if ( parentUpperLeft.m_Y > rootCenter.m_Y )			// tip upper right
				setWindow( RectInt( parentWindow.upperRight() + PointInt( 0, -mySize.height), mySize) );
			else												// tip lower right
				setWindow( RectInt( parentWindow.lowerRight(), mySize ) );
		}

		// make sure this window is display inside of the rootWindow
		if (! rootWindow.inRect( window() ) )
		{
			RectInt myWindow( window() );

			if ( myWindow.left < rootWindow.left )
				myWindow += PointInt( rootWindow.left - myWindow.left, 0 );		// slide window right
			else if ( myWindow.right > rootWindow.right )
				myWindow -= PointInt( myWindow.right - rootWindow.right, 0 );	// slide window left

			if ( myWindow.top < rootWindow.top )
				myWindow += PointInt( 0, rootWindow.top - myWindow.top );		// slide window down
			else if ( myWindow.bottom > rootWindow.bottom )
				myWindow -= PointInt( 0, myWindow.bottom - rootWindow.bottom );	// slide window up

			setWindow( myWindow );
		}
	}
}

//-------------------------------------------------------------------------------
// EOF
