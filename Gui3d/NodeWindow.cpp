/*
	NodeWindow.cpp
	
	This node is used to display 2D information in the 3d space.
	(c)2005 Palestar, Richard Lyle
*/

#define GUI3D_DLL

#include "Debug/Assert.h"
#include "Debug/Trace.h"
#include "Audio/Sound.h"
#include "Standard/Bits.h"
#include "Standard/Limits.h"
#include "Standard/StringHash.h"
#include "Standard/Constant.h"
#include "Display/PrimitiveWindow.h"
#include "Display/PrimitiveSetTransform.h"
#include "Display/PrimitiveLineList.h"
#include "Display/PrimitiveLineStrip.h"
#include "Display/PrimitiveTriangleFan.h"
#include "Display/PrimitiveTriangleStrip.h"
#include "Display/PrimitiveMaterial.h"
#include "Draw/Draw.h"
#include "Math/Helpers.h"

#include "NodeWindow.h"
#include "WindowTip.h"
#include "InterfaceContext.h"

//----------------------------------------------------------------------------

float NodeWindow::s_Alpha = 0.0f;
float NodeWindow::s_RollSpeed = 0.1f;

Constant		WINDOW_FLARE_CHECK_TIME( "WINDOW_FLARE_CHECK_TIME", 0.5f );
Constant		WINDOW_FLARE_CHANCE( "WINDOW_FLARE_CHANCE", 5 );
Constant		WINDOW_FADE_IN_TIME( "WINDOW_FADE_IN_TIME", 0.5f );			// how long to fade in windows with the EFFECT_FADEIN style
Constant		WINDOW_WHEEL_SCROLL_SPEED( "WINDOW_WHEEL_SCROLL_SPEED", 0.1f );
Constant		WINDOW_SHADE( "WINDOW_SHADE", 0.8f );
Constant		WINDOW_LIGHT( "WINDOW_LIGHT", 1.5f );
Constant		WINDOW_GLOW_SIZE( "WINDOW_GLOW_SIZE", 5.0f );
Constant		WINDOW_GLOW_INSET( "WINDOW_GLOW_INSET", -1.0f );
Constant		WINDOW_BORDER_THICKNESS( "WINDOW_BORDER_THICKNESS", 20.0f );

const Color		WINDOW_GLOW_OUTER_COLOR( 0,0,0,0 );
const Color		WINDOW_GLOW_INNER_COLOR( 255,255,255,255 );

const int		MAX_GLOW_VERTS = 64;
const int		MAX_BORDER_VERTS = 9;
const int		MAX_BACKGROUND_VERTS = 10;

const dword		WINDOW_SCROLL_STYLE = NodeWindow::SMOOTH_UL|NodeWindow::SMOOTH_LL|NodeWindow::SMOOTH_UR|NodeWindow::SMOOTH_LR;
const int		WINDOW_SCROLL_BORDER = 2;

//----------------------------------------------------------------------------

IMPLEMENT_FACTORY( NodeWindow, NodeInterfaceClient );
REGISTER_FACTORY_KEY(  NodeWindow, 3976961571858502462 );

BEGIN_PROPERTY_LIST( NodeWindow, NodeInterfaceClient );
	ADD_BITS_PROPERTY( m_Style );
		ADD_BIT_OPTION( m_Style, HAS_BACK );
		ADD_BIT_OPTION( m_Style, CAN_MOVE );
		ADD_BIT_OPTION( m_Style, CAN_SIZE );
		ADD_BIT_OPTION( m_Style, CAN_SCROLL );
		ADD_BIT_OPTION( m_Style, CAN_DOCK );
		ADD_BIT_OPTION( m_Style, DOCUMENT );
		ADD_BIT_OPTION( m_Style, CAN_FOCUS );
		ADD_BIT_OPTION( m_Style, SMOOTH_UL );
		ADD_BIT_OPTION( m_Style, SMOOTH_UR );
		ADD_BIT_OPTION( m_Style, SMOOTH_LR );
		ADD_BIT_OPTION( m_Style, SMOOTH_LL );
		ADD_BIT_OPTION( m_Style, ANIM_BEAM );
		ADD_BIT_OPTION( m_Style, ANIM_BACK );
		ADD_BIT_OPTION( m_Style, ANIM_FLARE );
		ADD_BIT_OPTION( m_Style, EFFECT_FADEIN );
		ADD_BIT_OPTION( m_Style, EFFECT_HIGHLIGHT );
		ADD_BIT_OPTION( m_Style, EFFECT_ROLLUV );
	ADD_BITS_PROPERTY( m_Flags );
		ADD_BIT_OPTION( m_Flags, VISIBLE );
		ADD_BIT_OPTION( m_Flags, NOCLIP );
		ADD_BIT_OPTION( m_Flags, DOCKED );
		ADD_BIT_OPTION( m_Flags, DOCK_OUTSIDE );
		ADD_BIT_OPTION( m_Flags, PARENT_WIDTH );
		ADD_BIT_OPTION( m_Flags, PARENT_HEIGHT );
		ADD_BIT_OPTION( m_Flags, DISABLED );
		ADD_BIT_OPTION( m_Flags, DOCK_TOP_LEFT );
		ADD_BIT_OPTION( m_Flags, DOCK_TOP );
		ADD_BIT_OPTION( m_Flags, DOCK_TOP_RIGHT );
		ADD_BIT_OPTION( m_Flags, DOCK_RIGHT );
		ADD_BIT_OPTION( m_Flags, DOCK_BOTTOM_RIGHT );
		ADD_BIT_OPTION( m_Flags, DOCK_BOTTOM );
		ADD_BIT_OPTION( m_Flags, DOCK_BOTTOM_LEFT );
		ADD_BIT_OPTION( m_Flags, VISIBLE );
		ADD_BIT_OPTION( m_Flags, VISIBLE );
		ADD_BIT_OPTION( m_Flags, VISIBLE );
	ADD_PROPERTY( m_Window );
	ADD_PROPERTY( m_Offset );
	ADD_PROPERTY( m_Alpha );
	ADD_PROPERTY( m_Help );
	ADD_PROPERTY( m_sScript );
	ADD_PROPERTY( m_WindowStyle );
	ADD_PROPERTY( m_Resources );
END_PROPERTY_LIST();

NodeWindow::NodeWindow()
{
	m_Style = 0;
	m_Flags = VISIBLE;
	m_Window.set( 0, 0, 128, 128 );
	m_Alpha = m_TargetAlpha = 1.0f;
	m_Offset.set( 0, 0 );

	m_ActiveTime = 1.0f;
	m_VisibleTime = 1.0f;
	m_ActionTime = 0;
	m_Flare = 1.0f;
}

//----------------------------------------------------------------------------

bool NodeWindow::read( const InStream & input )
{
	if (! NodeInterfaceClient::read( input ) )
		return false;

	if ( m_Flags & PARENT_SIZE )
	{
		m_Flags &= ~PARENT_SIZE;
		m_Flags |= PARENT_WIDTH|PARENT_HEIGHT;
	}

	m_Help = LocalizedString( m_Help );
	m_TargetAlpha = m_Alpha;
	return true;
}

//----------------------------------------------------------------------------

void NodeWindow::preRender( RenderContext &context, 
				const Matrix33 & frame, 
				const Vector3 & position )
{
	// make sure window is visible and has a valid style
	if ( (m_Flags & VISIBLE) != 0 && windowStyle() != NULL )
		render( context, frame, position );
}

const RectFloat	DEFAULT_WINDOW_UV(0,0,1,1);
const int		ALPHA_HIGHLIGHT = 128;
const int		BACK_OUTSET = 0;
const int		SCROLL_BAR_ALPHA = 64;
const int		SCROLL_TAB_ALPHA = 200;
const float		BEAM_ANIM_TIME = 0.25f;

void NodeWindow::render( RenderContext &context, 
				const Matrix33 & frame, 
				const Vector3 & position )
{
	DisplayDevice * pDisplay = context.display();
	ASSERT( pDisplay );

	context.beginScene();

	// get the time
	float t = context.elapsed();
	// update the visible time
	m_VisibleTime += t;

	if ( (m_Style & EFFECT_FADEIN) != 0 && m_VisibleTime < WINDOW_FADE_IN_TIME )
	{
		// fade the alpha towards the target alpha
		m_Alpha = (m_VisibleTime / WINDOW_FADE_IN_TIME) * m_TargetAlpha;
	}
	else if ( m_Alpha != m_TargetAlpha )
	{
		if ( m_Alpha < m_TargetAlpha )
			m_Alpha = Min( m_Alpha + t, m_TargetAlpha );
		else
			m_Alpha = Max( m_Alpha - t, m_TargetAlpha );
	}

	// resize / move this window
	checkPlacement();

	RectInt window( m_Window );
	window += context.offset();

	if ( m_Style & HAS_BACK )
	{
		bool isDepressed = (m_Flags & DEPRESSED) != 0;

		// get the border color
		Color kBorder = borderColor();
		// calculate the light and shade colors
		Color kLight = isDepressed ? kBorder * WINDOW_SHADE : kBorder * WINDOW_LIGHT;
		Color kShade = isDepressed ? kBorder * WINDOW_LIGHT : kBorder * WINDOW_SHADE;
		kLight.alpha = kShade.alpha = (u8)(m_Alpha * 255);

		// get the back ground color, adjust if we are doing a flare animation
		Color kBack( backColor() );
		// darken the background color when depressed
		if ( isDepressed )
			kBack *= WINDOW_SHADE;
		// handle any flare
		if ( m_Style & ANIM_FLARE )
			kBack *= m_Flare;
		kBack.alpha = (u8)(m_Alpha * 255);

		if ( kBorder.alpha > 0 || kBack.alpha > 0 )
		{
			// set the material
			PrimitiveMaterial::push( context.display(), WHITE, PrimitiveMaterial::ALPHA );

			// render the background & border
			if ( kBack.alpha > 0 )
				pushBackground( context, window, kBack, style(), borderSize() );
			if ( kBorder.alpha > 0 )
				pushBorder( context, window, kLight, kShade, style(), borderSize() );
		}
	}

	// highlight window if needed
	if ( (m_Flags & HIGHLIGHT) != 0 )
	{
		pushGlow( context, window, WINDOW_GLOW_SIZE, WINDOW_GLOW_INNER_COLOR, 
			WINDOW_GLOW_OUTER_COLOR, style(), borderSize() );
	}

	// display pink outline for the resourcer
	if ( s_ShowSelected && s_pSelectedNode == this )
	{
		const Color	PINK( 255, 0, 255, 255 );
		PrimitiveMaterial::push( pDisplay, PrimitiveMaterial::NONE );
		PrimitiveLineStripDTL::push( pDisplay, window, PINK, 0.0f );
	}

	if ( m_Style & CAN_SCROLL )
	{
		WindowStyle * pStyle = windowStyle();
		ASSERT( pStyle );

		// calculate the size of the contents
		SizeInt content( contentSize() );

		int windowHeight = m_Window.size().height - borderSize();
		if ( windowHeight < content.height )
		{
			int scrollHeight = content.height - windowHeight;
			m_Offset.m_Y = Clamp( m_Offset.m_Y, 0, scrollHeight );
			m_Flags |= VSCROLL;

			// firstly set the material
			PrimitiveMaterial::push( pDisplay, PrimitiveMaterial::ALPHA );

			// display vertical scroll bar
			RectInt scrollWindow( window.right - pStyle->scrollSize() - borderSize(), 
				window.top + borderSize() ,
				window.right - borderSize(), 
				window.bottom - borderSize() );
			window.right = scrollWindow.left;

			Color barColor( pStyle->scrollBarColor() );
			Color barLightColor( barColor * WINDOW_LIGHT );
			Color barShadeColor( barColor * WINDOW_SHADE );
			barLightColor.alpha = barShadeColor.alpha = (u8)(m_Alpha * 255);

			pushBorder( context, scrollWindow, barLightColor, barShadeColor, WINDOW_SCROLL_STYLE, WINDOW_SCROLL_BORDER );

			if ( m_Flags & HIGHLIGHT_SCROLL )
			{
				RectInt glowWindow( scrollWindow );
				glowWindow.inset( -1 );

				// mouse is over the scroll bar, display glow around the scroll bar
				pushGlow( context, glowWindow, WINDOW_GLOW_SIZE, WINDOW_GLOW_INNER_COLOR, 
					WINDOW_GLOW_OUTER_COLOR, WINDOW_SCROLL_STYLE, WINDOW_SCROLL_BORDER );
			}

			float tabScale = float( scrollWindow.height() ) / content.height;
			int tabSize = (int)(tabScale * windowHeight);
			int tabOffset = (int)(tabScale * m_Offset.m_Y);

			RectInt tabWindow( scrollWindow.left, scrollWindow.top + tabOffset,
				scrollWindow.right, scrollWindow.top + tabOffset + tabSize );
			Color tabColor( pStyle->scrollTabColor() );

			bool isScrolling = (m_Flags & SCROLLING) != 0;
			Color tabLightColor( isScrolling ? tabColor * WINDOW_SHADE : tabColor * WINDOW_LIGHT );
			Color tabShadeColor( isScrolling ? tabColor * WINDOW_LIGHT : tabColor * WINDOW_SHADE );
			if ( isScrolling )
				tabColor *= WINDOW_SHADE;

			tabColor.alpha = tabLightColor.alpha = tabShadeColor.alpha = (u8)(m_Alpha * 255);

			pushBackground( context, tabWindow, tabColor, WINDOW_SCROLL_STYLE, WINDOW_SCROLL_BORDER );
			pushBorder( context, tabWindow, tabLightColor, tabShadeColor, WINDOW_SCROLL_STYLE, WINDOW_SCROLL_BORDER );
		}
		else
		{
			m_Flags &= ~VSCROLL;
			m_Offset.m_Y = 0;
		}
	}

	// call client render
	onRender( context, window );
	// flush the render device
	context.endScene();

	// recurse into child nodes now
	if ( childCount() > 0 )
	{
		if ( (m_Flags & NOCLIP) == 0 )
		{
			// save previous window and offset
			RectInt parentWindow( context.window() );
			PointInt parentOffset( context.offset() );

			RectInt clipWindow( window );
			if ( windowStyle() )
				clipWindow.inset( borderSize() );
			clipWindow &= parentWindow;
			if ( clipWindow.valid() )
			{
				context.setWindow( clipWindow );
				context.setOffset( PointInt( window.left, window.top) - m_Offset );

				// render the children nodes
				for(int i=0;i<childCount();i++)
					child(i)->preRender( context, frame, position );

				// restore parent settings
				context.setWindow( parentWindow );
				context.setOffset( parentOffset );
			}
		}
		else
		{
			PointInt parentOffset( context.offset() );
			context.setOffset( PointInt( window.left, window.top) - m_Offset );

			// render the children nodes, no clipping by this window because it's a child window
			for(int i=0;i<childCount();i++)
				child(i)->preRender( context, frame, position );

			context.setOffset( parentOffset );
		}
	}
}

//----------------------------------------------------------------------------

bool NodeWindow::enabled() const
{
	return( (m_Flags & DISABLED) == 0 );
}

void NodeWindow::onActivate()
{
	NodeInterfaceClient::onActivate();

	m_ActiveTime = 0.0f;
	m_VisibleTime = 0.0f;
}

void NodeWindow::onUpdate( float t )
{
	NodeInterfaceClient::onUpdate( t );

	// update the active time
	m_ActiveTime += t;

	if ( m_Style & ANIM_FLARE )
	{
		// decay the flare
		m_Flare -= (m_Flare - 1.0f) * t;

		// check for new flare
		m_ActionTime += t;
		if ( m_ActionTime > WINDOW_FLARE_CHECK_TIME )
		{
			m_ActionTime -= WINDOW_FLARE_CHECK_TIME;

			if ( (rand() % 100) < WINDOW_FLARE_CHANCE )
				m_Flare = RandomFloat( 1.5f, 2.0f );
		}
	}
}

bool NodeWindow::onMessage( const Message & msg )
{
	if ( visible() )
	{
		switch( msg.message )
		{
		case HM_MOUSEMOVE:
			return( cursorMove( *((PointInt *)msg.lparam), *((PointInt *)msg.wparam) ) );
		case HM_LBUTTONDOWN:
			return( leftDown( *((PointInt *)msg.lparam) ) );
		case HM_LBUTTONUP:
			return( leftUp( *((PointInt *)msg.lparam) ) );
		case HM_RBUTTONDOWN:
			return( onRightDown( localWindow(), screenToWindow( *((PointInt *)msg.lparam) ) ) );
		case HM_RBUTTONUP:
			return( onRightUp( localWindow(), screenToWindow( *((PointInt *)msg.lparam) ) ) );
		case HM_MOUSEWHEEL:
			return( mouseWheel( *((PointInt *)msg.lparam), msg.wparam ) );
		}
	}

	return NodeInterfaceClient::onMessage( msg );
}

//----------------------------------------------------------------------------

Resource * NodeWindow::resource( const char * pName, bool a_bBlocking /*= true*/ ) const
{
	u32 nID = StringHash( pName );
	// look for the resource in this window
	ResourceHashIt find = m_Resources.find( nID );
	if ( find.valid() )
		return (*find).pointer( true, a_bBlocking );

	// check the parent window for the resource
	if ( WidgetCast<NodeWindow>( parent() ) )
	{
		Resource * pResource = ((NodeWindow *)parent())->resource( pName, a_bBlocking );
		if ( pResource )
			return pResource;
	}

	// lastly, try to load the resource directly from the broker..
	Resource::Link pResourceLink( StringHash64( pName ).hash() );
	if ( pResourceLink.valid( true, a_bBlocking ) )
	{
		// attach the resource onto the window by name..
		const_cast<NodeWindow *>( this )->m_Resources[ nID ] = pResourceLink;
		return pResourceLink;
	}

	return NULL;
}

WindowStyle * NodeWindow::windowStyle() const
{
	if (! m_WindowStyle.valid() )
	{
		if ( WidgetCast<NodeWindow>( parent() ) )
			return ((NodeWindow *)parent())->windowStyle();

		// return the default style from the InterfaceContext
		if ( context() != NULL )
			return context()->windowStyle();
	}
	return( m_WindowStyle );
}

//----------------------------------------------------------------------------

void NodeWindow::checkPlacement()
{
	if ( (m_Flags & (DOCKED|PARENT_WIDTH|PARENT_HEIGHT|HCENTER|VCENTER)) != 0 && (m_Flags & MOVING) == 0 )
	{
		NodeWindow * pParent = parentWindow();
		if ( pParent != NULL )
		{
			RectInt parentWindow( pParent->localWindow() );

			if ( m_Flags & (PARENT_WIDTH|PARENT_HEIGHT) )
			{
				int borderSize = pParent->borderSize();
				if ( m_Flags & PARENT_WIDTH )
				{
					m_Window.left = parentWindow.left + borderSize;
					m_Window.right = parentWindow.right - borderSize;
					if ( pParent->m_Style & CAN_SCROLL )
						m_Window.right -= pParent->windowStyle()->scrollSize();
				}
				if ( m_Flags & PARENT_HEIGHT )
				{
					m_Window.top = parentWindow.top + borderSize;
					m_Window.bottom = parentWindow.bottom - borderSize;
				}
			}

			if ( m_Flags & HCENTER )
				m_Window += PointInt( ((parentWindow.width() / 2) - (m_Window.width() / 2)) - m_Window.left, 0 );
			if ( m_Flags & VCENTER )
				m_Window += PointInt( 0, ((parentWindow.height() / 2) - (m_Window.height() / 2)) - m_Window.top );

			if ( m_Flags & DOCKED )
			{
				if ( (m_Flags & DOCK_OUTSIDE) == 0 )
				{
					switch( m_Flags & DOCK_MASK )
					{
					case DOCK_LEFT:
						m_Window += PointInt( parentWindow.left - m_Window.left, 0 );
						break;
					case DOCK_TOP_LEFT:
						m_Window += PointInt( parentWindow.left - m_Window.left, parentWindow.top - m_Window.top );
						break;
					case DOCK_TOP:
						m_Window += PointInt( 0, parentWindow.top - m_Window.top );
						break;
					case DOCK_TOP_RIGHT:
						m_Window += PointInt( parentWindow.right - m_Window.right, parentWindow.top - m_Window.top );
						break;
					case DOCK_RIGHT:
						m_Window += PointInt( parentWindow.right - m_Window.right, 0 );
						break;
					case DOCK_BOTTOM_RIGHT:
						m_Window += PointInt( parentWindow.right - m_Window.right, parentWindow.bottom - m_Window.bottom );
						break;
					case DOCK_BOTTOM:
						m_Window += PointInt( 0, parentWindow.bottom - m_Window.bottom );
						break;
					case DOCK_BOTTOM_LEFT:
						m_Window += PointInt( parentWindow.left - m_Window.left, parentWindow.bottom - m_Window.bottom );
						break;
					}
				}
				else
				{
					// dock the window on the outside of the parent window
					switch( m_Flags & DOCK_MASK )
					{
					case DOCK_LEFT:
						m_Window += PointInt( parentWindow.left - m_Window.right, 0 );
						break;
					case DOCK_TOP_LEFT:
						m_Window += PointInt( parentWindow.left - m_Window.right, parentWindow.top - m_Window.bottom );
						break;
					case DOCK_TOP:
						m_Window += PointInt( 0, parentWindow.top - m_Window.bottom );
						break;
					case DOCK_TOP_RIGHT:
						m_Window += PointInt( parentWindow.right - m_Window.left, parentWindow.top - m_Window.bottom );
						break;
					case DOCK_RIGHT:
						m_Window += PointInt( parentWindow.right - m_Window.left, 0 );
						break;
					case DOCK_BOTTOM_RIGHT:
						m_Window += PointInt( parentWindow.right - m_Window.left, parentWindow.bottom - m_Window.top );
						break;
					case DOCK_BOTTOM:
						m_Window += PointInt( 0, parentWindow.bottom - m_Window.top );
						break;
					case DOCK_BOTTOM_LEFT:
						m_Window += PointInt( parentWindow.left - m_Window.right, parentWindow.bottom - m_Window.top );
						break;
					}
				}
			}
		}
		else
		{
			TRACE("Docked window with no valid parent window!");
		}
	}
}

//----------------------------------------------------------------------------


void NodeWindow::addResource( const char * pName, Resource::Link pResource )
{
	m_Resources[ StringHash( pName ) ] = pResource;
}

void NodeWindow::destroy()
{
	// destroy this window
	context()->detachNode( this );
}

void NodeWindow::resetActiveTime()
{
	// set the active time to 0
	m_ActiveTime = 0.0f;

	// recurse into any child nodes
	for(int i=0;i<childCount();i++)
		if ( WidgetCast<NodeWindow>( child(i) ) != NULL )
			((NodeWindow *)child(i))->resetActiveTime();
}

//----------------------------------------------------------------------------

void NodeWindow::onShow()
{
	// reset the visible time
	setVisibleTime( 0 );
}

void NodeWindow::onHide()
{
	// clear the highlight flag
	m_Flags &= ~HIGHLIGHT;
}

void NodeWindow::onEnable()
{
	// reset the active time for this window
	setActiveTime( 0 );
}

void NodeWindow::onDisable()
{}

void NodeWindow::onRender( RenderContext & context, const RectInt & window )
{}

bool NodeWindow::onCursorMove( const RectInt & window, const PointInt & position, const PointInt & delta )
{
	if ( window.inRect( position ) )
	{
		if ( m_Style & CAN_MOVE )
		{
			setCursorState( MOVE );
			return true;
		}
	}

	return false;
}

bool NodeWindow::onLeftDown( const RectInt & window, const PointInt & position )
{
	if ( window.inRect( position ))
	{
		if ( m_Style & CAN_MOVE )
		{
			m_Flags |= MOVING;
			return true;
		}
	}

	return false;
}

bool NodeWindow::onLeftUp( const RectInt & window, const PointInt & position )
{
	if ( m_Flags & RESIZING )
		m_Flags &= ~RESIZING;
	if ( m_Flags & MOVING )
		m_Flags &= ~MOVING;
	if ( m_Flags & SCROLLING )
		m_Flags &= ~SCROLLING;

	return false;
}

bool NodeWindow::onRightDown( const RectInt & window, const PointInt & position )
{
	return false;
}

bool NodeWindow::onRightUp( const RectInt & window, const PointInt & position )
{
	return false;
}

Color NodeWindow::backColor() const
{
	return windowStyle()->backColor();
}

Color NodeWindow::borderColor() const
{
	return windowStyle()->borderColor();
}

int	NodeWindow::borderSize() const
{
	return windowStyle()->borderSize();
}

//----------------------------------------------------------------------------

bool NodeWindow::visible() const
{
	if ( (flags() & VISIBLE) == 0 )
		return false;

	// check the parent windows, if any are not visible then this window is not visible
	BaseNode * pNode = parent();
	while( pNode != NULL )
	{
		if ( WidgetCast<NodeWindow>( pNode ) )
			if ( (((NodeWindow *)pNode)->flags() & VISIBLE) == 0 )
				return false;

		pNode = pNode->parent();
	}

	// this window is visible
	return true;
}

void NodeWindow::playSound( const char * pName )
{
	Sound * pSound = WidgetCast<Sound>( resource( pName, false ) );
	if ( pSound != NULL )
	{
		AudioDevice * pAudio = context() ? context()->audio() : NULL;
		if ( pAudio != NULL )
			pSound->play( pAudio, 1.0f, 0.0f );
	}
}

void NodeWindow::hoverSound()
{
	AudioDevice * pAudio = context() ? context()->audio() : NULL;
	Sound * pSound = windowStyle()->hoverSound();

	if ( pAudio && pSound )
		pSound->play( pAudio, 1.0f, 0.0f );
}

void NodeWindow::selectSound()
{
	AudioDevice * pAudio = context() ? context()->audio() : NULL;
	Sound * pSound = windowStyle()->selectSound();

	if ( pAudio && pSound )
		pSound->play( pAudio, 1.0f, 0.0f );
}

void NodeWindow::setCursorState( CursorState state )
{
	if ( context() != NULL )
		context()->setCursorState( state );
}

void NodeWindow::setCursorTip( const char * pTip )
{
	if ( context() != NULL )
		context()->setCursorTip( pTip );
}

void NodeWindow::setVisible( bool visible )
{
	if ( ((m_Flags & VISIBLE) != 0) != visible )
	{
		if ( visible )
		{
			// make this window visible
			m_Flags |= VISIBLE;
			// notify this node and all children that they are visible
			onShow( this );
		}
		else
		{
			// notify this window and it's children they are hidden
			onHide( this );
			// hide this window
			m_Flags &= ~VISIBLE;
		}
	}	
}

void NodeWindow::setEnable( bool enable )
{
	if ( ((m_Flags & DISABLED) == 0 ) != enable )
	{
		if ( enable )
		{
			// remove disabled flag
			m_Flags &= ~DISABLED;
			// set the active time to 0
			onEnable( this );
		}
		else
		{
			// notify the windows before we disable them
			onDisable( this );
			// set the disabled flag
			m_Flags |= DISABLED;
		}

		// invalidate the message queue
		if ( context() != NULL )
			context()->setMessageQueueInvalid();
	}
}

void NodeWindow::hideWindow(bool enable /*= false*/)
{
	setVisible( false );
	setEnable( enable );
}

void NodeWindow::showWindow(bool visible /*= true*/ )
{
	setVisible( visible );
	setEnable( visible );
}

void NodeWindow::hideChildren()
{
	for(int i=0;i<childCount();i++)
	{
		NodeWindow * pChild = WidgetCast<NodeWindow>( child(i) );
		if ( pChild != NULL )
			pChild->hideWindow();
	}
}

void NodeWindow::destroyChildren()
{
	for(int i=0;i<childCount();i++)
	{
		NodeWindow * pChild = WidgetCast<NodeWindow>( child(i) );
		if ( pChild != NULL )
			pChild->destroy();
	}
}

void NodeWindow::cullChildren( int keep )
{
	for(int i=keep;i<childCount();i++)
	{
		NodeWindow * pChild = WidgetCast<NodeWindow>( child(i) );
		if ( pChild != NULL )
			pChild->destroy();
	}
}

RectInt NodeWindow::localWindow()
{
	RectInt window( m_Window );

	// clip the window by the parent
	NodeWindow * pParent = parentWindow();
	if ( pParent != NULL )
	{
		if ( (pParent->m_Flags & NOCLIP) == 0 )
			window &= (pParent->localWindow() + pParent->m_Offset);
	}

	window -= m_Window.upperLeft();
	return( window );
}

RectInt NodeWindow::screenWindow()
{
	RectInt window( localWindow() );
	return RectInt( windowToScreen( window.upperLeft() ), window.size() );
}

SizeInt NodeWindow::contentSize()
{
	RectInt content( 0, 0, 0, 0 ); 

	for(int i=0;i<childCount();i++)
		if ( WidgetCast<NodeWindow>( child(i) ) )
		{
			if ( ((NodeWindow *)child(i))->visible() )
				content |= ((NodeWindow *)child(i))->m_Window;
		}

	return( content.size() );
}

NodeWindow * NodeWindow::parentWindow() const
{
	BaseNode * pParent = parent();
	while( pParent != NULL )
	{
		if ( WidgetCast<NodeWindow>( pParent ) )
			return (NodeWindow *)pParent;

		pParent = pParent->parent();
	}

	return NULL;
}

NodeWindow * NodeWindow::rootWindow() const
{
	NodeWindow * pRoot = parentWindow();
	if ( pRoot != NULL )
	{
		NodeWindow * pParent = pRoot->parentWindow();
		while( pParent != NULL )
		{
			pRoot = pParent;
			pParent = pRoot->parentWindow();
		}
	}

	return pRoot;
}

// translate a screen coordinate to a window coordinate
PointInt NodeWindow::screenToWindow( const PointInt & point )
{
	PointInt windowPoint( point );

	BaseNode * pNode = this;
	while( pNode != NULL )
	{
		pNode = pNode->parent();

		if ( WidgetCast<NodeWindow>( pNode ) )
		{
			NodeWindow * pWindow = (NodeWindow *)pNode;
			windowPoint -= PointInt( pWindow->m_Window.left, pWindow->m_Window.top );
			windowPoint += pWindow->m_Offset;
		}
	}

	windowPoint -= m_Window.upperLeft();
	//windowPoint -= m_Offset;
	return windowPoint;
}

// translate a window coordinate to a screen coordinate
PointInt NodeWindow::windowToScreen( const PointInt & point )
{
	PointInt screenPoint( point );

	BaseNode * pNode = this;
	while( pNode != NULL )
	{
		if ( WidgetCast<NodeWindow>( pNode ) )
		{
			NodeWindow * pWindow = (NodeWindow *)pNode;

			screenPoint += PointInt( pWindow->m_Window.left, pWindow->m_Window.top );
			screenPoint -= pWindow->m_Offset;
		}

		pNode = pNode->parent();
	}

	return screenPoint;
}

void NodeWindow::renderGlow( RenderContext & context )
{
	// set a material
	PrimitiveMaterial::push( context.display(), PrimitiveMaterial::ALPHA, false );
	// push the glow
	pushGlow( context, screenWindow(), WINDOW_GLOW_SIZE, WINDOW_GLOW_INNER_COLOR,  
			WINDOW_GLOW_OUTER_COLOR, style(), borderSize() );
}

//----------------------------------------------------------------------------

void NodeWindow::pushBackground( RenderContext & context, const RectInt & window, Color color, dword style, int bs )
{
	VertexTL	back[ MAX_BACKGROUND_VERTS ];

	// use a triangle fan for the background
	int			verts = 0;
	
	// calculate the center of the window, this is the triangle fan origin
	PointInt center( (window.left + window.right) / 2, (window.top + window.bottom) / 2 );
	// begin triangle fan at the center
	back[ verts++ ] = VertexTL( Vector3( center.x, center.y, 0), 1, color, 0, 0);

	if ( style & SMOOTH_UL )
	{
		back[ verts++ ] = VertexTL( Vector3( window.left, window.top + bs, 0), 1, color, 0, 0 );
		back[ verts++ ] = VertexTL( Vector3( window.left + bs, window.top, 0), 1, color, 0, 0 );
	}
	else
		back[ verts++ ] = VertexTL( Vector3( window.left, window.top, 0), 1, color, 0, 0 );

	if ( style & SMOOTH_UR )
	{
		back[ verts++ ] = VertexTL( Vector3( window.right - bs, window.top, 0), 1, color, 0,0 );
		back[ verts++ ] = VertexTL( Vector3( window.right, window.top + bs, 0), 1, color, 0,0 );
	}
	else
		back[ verts++ ] = VertexTL( Vector3( window.right, window.top, 0), 1, color, 0,0 );

	if ( style & SMOOTH_LR )
	{
		back[ verts++ ] = VertexTL( Vector3( window.right, window.bottom - bs, 0), 1, color, 0,0 );
		back[ verts++ ] = VertexTL( Vector3( window.right - bs, window.bottom, 0), 1, color, 0,0 );
	}
	else
		back[ verts++ ] = VertexTL( Vector3( window.right, window.bottom, 0), 1, color, 0,0 );

	if ( style & SMOOTH_LL )
	{
		back[ verts++ ] = VertexTL( Vector3( window.left + bs, window.bottom, 0), 1, color, 0,0 );
		back[ verts++ ] = VertexTL( Vector3( window.left, window.bottom - bs, 0), 1, color, 0,0 );
	}
	else
		back[ verts++ ] = VertexTL( Vector3( window.left, window.bottom, 0), 1, color, 0,0 );

	back[ verts++ ] = back[ 1 ];

	// render the background
	PrimitiveTriangleFanDTL::push( context.display(), verts, back );
}

void NodeWindow::pushBorder( RenderContext & context, const RectInt & window, Color lightColor, Color shadeColor, dword style, int bs )
{
	VertexTL border[ MAX_BORDER_VERTS ];

	int	verts = 0;
	if ( style & SMOOTH_LR )
	{
		border[ verts++ ] = VertexTL( Vector3( window.right, window.bottom - bs, 0), 1, shadeColor, 0, 0 );
		border[ verts++ ] = VertexTL( Vector3( window.right - bs, window.bottom, 0), 1, shadeColor, 0, 0 );
	}
	else
		border[ verts++ ] = VertexTL( Vector3( window.right, window.bottom, 0), 1, shadeColor, 0, 0 );

	if ( style & SMOOTH_LL )
	{
		border[ verts++ ] = VertexTL( Vector3( window.left + bs, window.bottom, 0), 1, shadeColor, 0, 0 );
		border[ verts++ ] = VertexTL( Vector3( window.left, window.bottom - bs, 0), 1, shadeColor, 0, 0 );
	}
	else
		border[ verts++ ] = VertexTL( Vector3( window.left, window.bottom, 0), 1, shadeColor, 0, 0 );

	if ( style & SMOOTH_UL )
	{
		border[ verts++ ] = VertexTL( Vector3( window.left, window.top + bs, 0), 1, lightColor, 0, 0 );
		border[ verts++ ] = VertexTL( Vector3( window.left + bs, window.top, 0), 1, lightColor, 0, 0 );
	}
	else
		border[ verts++ ] = VertexTL( Vector3( window.left, window.top, 0), 1, lightColor, 0, 0 );

	if ( style & SMOOTH_UR )
	{
		border[ verts++ ] = VertexTL( Vector3( window.right - bs, window.top, 0), 1, shadeColor, 0, 0 );
		border[ verts++ ] = VertexTL( Vector3( window.right, window.top + bs, 0), 1, shadeColor, 0, 0 );
	}
	else
		border[ verts++ ] = VertexTL( Vector3( window.right, window.top, 0), 1, shadeColor, 0, 0 );

	border[ verts++ ] = border[ 0 ];

	PrimitiveLineStripDTL::push( context.display(), verts, border );
}

void NodeWindow::pushGlow( RenderContext & context, const RectInt & rect, int size, Color innerColor, Color outerColor, dword style, int bs )
{
	// outset the glow rect by 1 pixel
	RectInt window( rect );
	window.inset( -1 );

	// get the border midpoint
	int bs2 = bs / 2;
	// get the center points
	int cx = (window.left + window.right) / 2;
	int cy = (window.top + window.bottom) / 2;

	VertexTL glow[ MAX_GLOW_VERTS ];

	int vert = 0;
	if ( style & SMOOTH_UL )
	{
		glow[ vert++ ] = VertexTL( Vector3( window.left, window.top + bs, 0), 1, innerColor, 0, 0 );		// 1
		glow[ vert++ ] = VertexTL( Vector3( window.left - size, window.top, 0), 1, outerColor, 0, 0 );	// 2
		glow[ vert++ ] = VertexTL( Vector3( window.left + bs, window.top, 0), 1, innerColor, 0, 0 );	 // 3
		glow[ vert++ ] = VertexTL( Vector3( window.left, window.top - size, 0 ), 1, outerColor, 0, 0 );	// 4
	}
	else
	{
		glow[ vert++ ] = VertexTL( Vector3( window.left, window.top, 0), 1,innerColor,0,0 );		// 1
		glow[ vert++ ] = VertexTL( Vector3( window.left - size, window.top - size, 0), 1, outerColor, 0, 0 );  // 2
	}

	// top inner center point
	glow[ vert++ ] = VertexTL( Vector3( cx, window.top, 0), 1, innerColor, 0, 0 ); // 3, 5

	if ( style & SMOOTH_UR )
	{
		glow[ vert++ ] = VertexTL( Vector3( window.right, window.top - size, 0), 1, outerColor, 0, 0 );	// 6
		glow[ vert++ ] = VertexTL( Vector3( window.right - bs, window.top, 0), 1, innerColor, 0, 0 );	// 7
		glow[ vert++ ] = VertexTL( Vector3( window.right + size, window.top, 0), 1, outerColor, 0, 0 );	// 8
		glow[ vert++ ] = VertexTL( Vector3( window.right, window.top + bs, 0 ), 1, innerColor, 0, 0 );	// 9
	}
	else
	{
		glow[ vert++ ] = VertexTL( Vector3( window.right + size, window.top - size, 0 ), 1, outerColor, 0, 0 );		// 4
		glow[ vert++ ] = VertexTL( Vector3( window.right, window.top, 0 ), 1, innerColor, 0, 0 );					// 5
	}

	// right inner center point
	glow[ vert++ ] = VertexTL( Vector3( window.right + size, cy, 0 ), 1, outerColor, 0, 0 );		// 6, 10

	if ( style & SMOOTH_LR )
	{
		glow[ vert++ ] = VertexTL( Vector3( window.right, window.bottom - bs , 0), 1, innerColor, 0, 0 );	// 11
		glow[ vert++ ] = VertexTL( Vector3( window.right + size, window.bottom, 0 ), 1, outerColor, 0, 0 );	// 12
		glow[ vert++ ] = VertexTL( Vector3( window.right - bs, window.bottom , 0), 1, innerColor, 0, 0 );	// 13
		glow[ vert++ ] = VertexTL( Vector3( window.right, window.bottom + size, 0), 1, outerColor, 0, 0 );	// 14
	}
	else
	{
		glow[ vert++ ] = VertexTL( Vector3( window.right, window.bottom, 0 ), 1, innerColor, 0, 0 );		// 7
		glow[ vert++ ] = VertexTL( Vector3( window.right + size, window.bottom + size, 0 ), 1, outerColor, 0, 0 );	// 8
	}

	// bottom inner center point
	glow[ vert++ ] = VertexTL( Vector3( cx, window.bottom, 0), 1, innerColor, 0, 0 );	// 9, 15

	if ( style & SMOOTH_LL )
	{
		glow[ vert++ ] = VertexTL( Vector3( window.left, window.bottom + size, 0), 1, outerColor, 0, 0 );	// 16
		glow[ vert++ ] = VertexTL( Vector3( window.left + bs, window.bottom, 0 ), 1, innerColor, 0, 0 );	// 17
		glow[ vert++ ] = VertexTL( Vector3( window.left - size, window.bottom, 0 ), 1, outerColor, 0, 0 );	// 18
		glow[ vert++ ] = VertexTL( Vector3( window.left, window.bottom - bs, 0 ), 1, innerColor, 0, 0 );	// 19
	}
	else
	{
		glow[ vert++ ] = VertexTL( Vector3( window.left - size, window.bottom + size, 0 ), 1, outerColor, 0, 0 );	// 10
		glow[ vert++ ] = VertexTL( Vector3( window.left, window.bottom, 0 ), 1, innerColor, 0, 0 );		// 11
	}

	// right inner center point
	glow[ vert++ ] = VertexTL( Vector3( window.left - size, cy, 0 ), 1, outerColor, 0, 0 );	// 12, 20

	// connect back to the upper left corner
	glow[ vert++ ] = glow[ 0 ];
	glow[ vert++ ] = glow[ 1 ];

	PrimitiveTriangleStripDTL::push( context.display(), vert, glow );
}

//----------------------------------------------------------------------------

const int BREAK_DOCK = 20;		// how many pixels does a window need to be away to break it's dock
const int BEGIN_DOCK = 5;

bool NodeWindow::cursorMove( const PointInt & position, const PointInt & delta )
{
	ASSERT( context() );

	// get this windows parent
	NodeWindow * pParent = parentWindow();

	if ( m_Flags & RESIZING )
	{
		if ( pParent == NULL )
			return true;

		const int MIN_WINDOW_SIZE = borderSize() * 2;

		if ( m_Flags & SIZE_LEFT )
			m_Window.left += delta.m_X;
		else if ( m_Flags & SIZE_RIGHT )
			m_Window.right += delta.m_X;
		
		if ( m_Flags & SIZE_TOP )
			m_Window.top += delta.m_Y;
		else if ( m_Flags & SIZE_BOTTOM )
			m_Window.bottom += delta.m_Y;

		if ( m_Window.height() < MIN_WINDOW_SIZE )
			m_Window.setHeight( MIN_WINDOW_SIZE );
		if ( m_Window.width() < MIN_WINDOW_SIZE )
			m_Window.setWidth( MIN_WINDOW_SIZE );

		return true;
	}
	if ( m_Flags & MOVING )
	{
		// modify the window position by the mouse delta
		m_Window += delta;

		if ( pParent != NULL )
		{
			RectInt parentWindow( pParent->screenWindow() );
			RectInt myWindow( screenWindow() );

			// handle dockin / undocking while moving the window
			if ( m_Style & CAN_DOCK )
			{
				if ( m_Flags & DOCKED )
				{
					switch( m_Flags & DOCK_MASK )
					{
					case DOCK_LEFT:
						if ( (myWindow.left - parentWindow.left) > BREAK_DOCK )
							m_Flags &= ~(DOCKED | DOCK_MASK);
						break;
					case DOCK_TOP:
						if ( (myWindow.top - parentWindow.top) > BREAK_DOCK )
							m_Flags &= ~(DOCKED | DOCK_MASK);
						break;
					case DOCK_RIGHT:
						if ( (parentWindow.right - myWindow.right) > BREAK_DOCK )
							m_Flags &= ~(DOCKED | DOCK_MASK);
						break;
					case DOCK_BOTTOM:
						if ( (parentWindow.bottom - myWindow.bottom) > BREAK_DOCK )
							m_Flags &= ~(DOCKED | DOCK_MASK);
						break;
					}
				}
				else
				{
					if ( (myWindow.left - parentWindow.left) < BEGIN_DOCK )
						m_Flags |= DOCKED | DOCK_LEFT;
					else if ( (myWindow.top - parentWindow.top) < BEGIN_DOCK )
						m_Flags |= DOCKED | DOCK_TOP;
					else if ( (myWindow.right - m_Window.right) < BEGIN_DOCK )
						m_Flags |= DOCKED | DOCK_RIGHT;
					else if ( (myWindow.bottom - m_Window.bottom) < BEGIN_DOCK )
						m_Flags |= DOCKED | DOCK_BOTTOM;
				}
			}
		}

		return true;
	}
	if ( m_Flags & SCROLLING )
	{
		// calculate the size of the contents
		SizeInt content( contentSize() );
		if ( m_Flags & VSCROLL )
		{
			SizeInt windowSize( m_Window.size() );

			int windowHeight = windowSize.height - borderSize();
			int yy = (delta.y * content.height) / windowHeight;
			m_Offset.m_Y = Clamp( m_Offset.m_Y + yy, 0, content.height - windowHeight );
		}
		return true;
	}

	// check for parent window being resized, scrolled, or moved... if so return false so that the
	// parent gets the mouse move message
	while( pParent != NULL )
	{
		if ( pParent->m_Flags & (SCROLLING | RESIZING | MOVING) )
			return false;
		pParent = pParent->parentWindow();
	}

	// get the local position of the cursor
	PointInt localPosition( screenToWindow( position ) );
	//PointInt realPosition( localPosition - m_Offset );		// remove the scroll of this window

	RectInt rLocalWindow( localWindow() );
	if ( rLocalWindow.inRect( localPosition ) )
	{
		// move this window to the top of the render
		if ( m_Style & CAN_FOCUS )
			context()->setFocus( this );

		if ( (m_Style & EFFECT_HIGHLIGHT) )
			m_Flags |= HIGHLIGHT;

		int bs = borderSize();
		if ( m_Style & CAN_SCROLL )
		{
			if ( (m_Flags & VSCROLL) && 
				localPosition.m_X <= (rLocalWindow.right - bs) &&
				localPosition.m_X >= (rLocalWindow.right - bs - windowStyle()->scrollSize()) )
			{
				// over vertical scroll bar
				setCursorState( VARROW );
				m_Flags |= HIGHLIGHT_SCROLL;
				return true;
			}
			else
				m_Flags &= ~HIGHLIGHT_SCROLL;
		}

		if ( m_Style & CAN_SIZE )
		{
			if ( localPosition.m_X <= (rLocalWindow.left + bs) ||
				localPosition.m_X >= (rLocalWindow.right - bs) )
			{
				// over left or right border
				setCursorState( HARROW );
				return true;
			}
			if ( //localPosition.m_Y <= (rLocalWindow.top + bs) ||
				localPosition.m_Y >= (rLocalWindow.bottom - bs) )
			{
				// over bottom border
				setCursorState( VARROW );
				return true;
			}
		}

		if ( m_Style & CAN_MOVE )
		{
			// if cusor over the top border, then allow the play to move
			if ( localPosition.m_Y <= (rLocalWindow.top + bs) )
			{
				setCursorState( MOVE );
				return true;
			}
		}
	}
	else
	{
		if ( (m_Style & EFFECT_HIGHLIGHT) )
			m_Flags &= ~HIGHLIGHT;
	}

	return onCursorMove( rLocalWindow, localPosition, delta );
}

bool NodeWindow::mouseWheel( const PointInt & position, int delta )
{
	PointInt localPosition( screenToWindow( position ) );

	RectInt rLocalWindow( localWindow() );
	if ( rLocalWindow.inRect( localPosition ) )
	{
		if ( m_Style & CAN_SCROLL && m_Flags & VSCROLL )
		{
			SizeInt content( contentSize() );
			SizeInt windowSize( m_Window.size() );

			int y = (int)(WINDOW_WHEEL_SCROLL_SPEED * delta);
			int yy = (y * content.height) / windowSize.height;
			m_Offset.m_Y = Clamp( m_Offset.m_Y - yy, 0, content.height - windowSize.height );
		}
	}

	return false;
}

bool NodeWindow::leftDown( const PointInt & position )
{
	PointInt localPosition( screenToWindow( position ) );
	RectInt rLocalWindow( localWindow() );
	//PointInt realPosition( localPosition - m_Offset );		// remove the scroll of this window

	if ( rLocalWindow.inRect( localPosition ) )
	{
		int bs = borderSize();
		if ( m_Style & CAN_SCROLL )
		{
			if ( (m_Flags & VSCROLL) && localPosition.m_X <= (rLocalWindow.right - bs) && 
				localPosition.m_X >= (rLocalWindow.right - bs - windowStyle()->scrollSize()) )
			{
				// over vertical scroll bar
				m_Flags |= SCROLLING;
				m_Flags &= ~HIGHLIGHT_SCROLL;
				return true;
			}
		}

		if ( m_Style & CAN_SIZE )
		{
			if ( localPosition.m_X <= (rLocalWindow.left + bs) )
			{
				m_Flags |= SIZE_LEFT;
				return true;
			}
			if ( localPosition.m_X >= (rLocalWindow.right - bs) )
			{
				m_Flags |= SIZE_RIGHT;
				return true;
			}
			if ( localPosition.m_Y >= (rLocalWindow.bottom - bs) )
			{
				m_Flags |= SIZE_BOTTOM;
				return true;
			}
		}
		if ( m_Style & CAN_MOVE )
		{
			if ( localPosition.m_Y <= (rLocalWindow.top + bs) )
			{
				m_Flags |= MOVING;
				return true;
			}
		}
	}

	return onLeftDown( rLocalWindow, localPosition );
}

bool NodeWindow::leftUp( const PointInt & position )
{
	return onLeftUp( localWindow(), screenToWindow( position ) );
}

//----------------------------------------------------------------------------

void NodeWindow::onShow( BaseNode * pNode )
{
	NodeWindow * pWindow = WidgetCast<NodeWindow>( pNode );
	if ( pWindow != NULL )
	{
		if ( (pWindow->flags() & VISIBLE) == 0 )
			return;		// window not visible, go no further up the nodes

		pWindow->onShow();
	}

	// recurse into the children
	for(int i=0;i<pNode->childCount();i++)
		onShow( pNode->child(i) );
}

void NodeWindow::onHide( BaseNode * pNode )
{
	NodeWindow * pWindow = WidgetCast<NodeWindow>( pNode );
	if ( pWindow != NULL )
	{
		if ( (pWindow->flags() & VISIBLE) == 0 )
			return;		// window isn't visible

		pWindow->onHide();
	}

	// recurse into the children
	for(int i=0;i<pNode->childCount();i++)
		onHide( pNode->child(i) );
}

void NodeWindow::onEnable( BaseNode * pNode )
{
	NodeWindow * pWindow = WidgetCast<NodeWindow>( pNode );
	if ( pWindow != NULL )
	{
		if ( (pWindow->flags() & DISABLED) != 0 )
			return;		// window not enabled, go no further up the nodes

		pWindow->onEnable();
	}

	// recurse into the children
	for(int i=0;i<pNode->childCount();i++)
		onEnable( pNode->child(i) );
}

void NodeWindow::onDisable( BaseNode * pNode )
{
	NodeWindow * pWindow = WidgetCast<NodeWindow>( pNode );
	if ( pWindow != NULL )
	{
		if ( (pWindow->flags() & DISABLED) != 0 )
			return;		// window already disabled, return

		pWindow->onDisable();
	}

	// recurse into the children
	for(int i=0;i<pNode->childCount();i++)
		onDisable( pNode->child(i) );
}

//----------------------------------------------------------------------------
// EOF
