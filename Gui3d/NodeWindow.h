/*
	NodeWindow.h
	
	This node is used to display 2D information in the 3d space.
	(c)2005 Palestar, Richard Lyle
*/

#ifndef NODE_WINDOW_H
#define NODE_WINDOW_H

#include "Draw/Font.h"
#include "System/LocalizedString.h"
#include "System/Messages.h"
#include "Display/PrimitiveWindow.h"

#include "NodeInterfaceClient.h"
#include "Cursor.h"
#include "WindowStyle.h"
#include "GUI3DDll.h"

//----------------------------------------------------------------------------

class DLL NodeWindow : public NodeInterfaceClient
{
public:
	DECLARE_WIDGET_CLASS();
	DECLARE_PROPERTY_LIST();

	// Types / Constants
	typedef Reference<NodeWindow>			Ref;
	typedef WeakReference<NodeWindow>		WeakRef;
	typedef WidgetReference<NodeWindow>		wRef;

	enum Constants
	{
		SCROLL_TAB_INSET			= 2,
	};

	enum Style
	{
		HAS_BACK					= 0x00000001,		// display window background and border
		CAN_MOVE					= 0x00000004,		// window can be moved
		CAN_SIZE					= 0x00000008,		// window can be resized
		CAN_SCROLL					= 0x00000010,		// window can be scrolled
		CAN_DOCK					= 0x00000020,		// window can be docked/undocked to the parent
		DOCUMENT					= 0x00000040,		// window is linked to the document
		CAN_FOCUS					= 0x00000080,		// window can be focused

		SMOOTH_UL					= 0x00000100,		// smooth upper-left corner
		SMOOTH_UR					= 0x00000200,		// upper-right
		SMOOTH_LR					= 0x00000400,		// lower-right
		SMOOTH_LL					= 0x00000800,		// lower-left

		ANIM_BEAM					= 0x00001000,		// enable beam animation
		ANIM_BACK					= 0x00002000,		// animate background
		ANIM_FLARE					= 0x00004000,		// flare background color

		EFFECT_FADEIN				= 0x00010000,		// fade the alpha of this window in when activated
		EFFECT_HIGHLIGHT			= 0x00020000,		// highlight with alpha when cursor is over this window
		EFFECT_ROLLUV				= 0x00040000,		// roll the background UV's
		
		USER1						= 0x01000000,
		USER2						= 0x02000000,
		USER3						= 0x04000000,
		USER4						= 0x08000000,
		USER5						= 0x10000000,
		USER6						= 0x20000000,
		USER7						= 0x40000000,
		USER8						= 0x80000000,
	};

	enum Flags
	{
		VISIBLE						= 0x00000001,		// is this window visible
		NOCLIP						= 0x00000002,		// disables child clipping
		MOVING						= 0x00000004,		// window is being moved
		SIZE_LEFT					= 0x00000008,		// window is being resized
		SIZE_RIGHT					= 0x00000010,
		SIZE_TOP					= 0x00000020,
		SIZE_BOTTOM					= 0x00000040,
		DOCKED						= 0x00000080,		// window is docked
		DOCK_X						= 0x00000100,	
		DOCK_Y						= 0x00000200,		
		HIGHLIGHT					= 0x00000400,		// cursor is over the window
		HSCROLL						= 0x00000800,		// window horzontal scroll bar is active
		VSCROLL						= 0x00001000,		// window vertical scroll bar
		SCROLLING					= 0x00002000,		// window is being scrolled
		PARENT_SIZE					= 0x00004000,		// window stays the same size as it's parent (obsolete)
		HCENTER						= 0x00008000,		// center window horizontal
		VCENTER						= 0x00010000,		// center window vertical
		DOCK_Z						= 0x00020000,
		DOCK_OUTSIDE				= 0x00040000,		// dock on the outside of the parent, instead of inside the parent window
		PARENT_WIDTH				= 0x00080000,
		PARENT_HEIGHT				= 0x00100000,

		DEPRESSED					= 0x00200000,
		HIGHLIGHT_SCROLL			= 0x00400000,

		DISABLED					= 0x80000000,		// is window disabled

		RESIZING					= SIZE_LEFT | SIZE_RIGHT | SIZE_TOP | SIZE_BOTTOM,
		DOCK_LEFT					= 0x0,
		DOCK_TOP_LEFT				= DOCK_Z,
		DOCK_TOP					= DOCK_X,
		DOCK_TOP_RIGHT				= DOCK_X | DOCK_Z,
		DOCK_RIGHT					= DOCK_Y,
		DOCK_BOTTOM_RIGHT			= DOCK_Y | DOCK_Z,
		DOCK_BOTTOM					= DOCK_X | DOCK_Y,
		DOCK_BOTTOM_LEFT			= DOCK_X | DOCK_Y | DOCK_Z,
		DOCK_MASK					= DOCK_X | DOCK_Y | DOCK_Z,
	};

	// Construction
	NodeWindow();
	// Widget Interface
	bool				read( const InStream & );
	// BaseNode Interface
	void				preRender( RenderContext &context, 
							const Matrix33 & frame, 
							const Vector3 & position );
	void				render( RenderContext &context, 
							const Matrix33 & frame, 
							const Vector3 & position );

	// NodeInterfaceClient interface
	bool				enabled() const;
	void				onActivate();
	void				onUpdate( float t );			
	bool				onMessage( const Message & msg );

	// Accessors
	dword				style() const;						// window style
	dword				flags() const;						// window flags
	const RectInt &		window() const;						// get this windows rect in pixels
	SizeInt				size() const;						// get the size of this window
	const PointInt &	offset() const;						// get the content offset
	float				alpha() const;						// window alpha
	const char *		help() const;						// window help text
	const char *		script() const;						// script code associated with window
	WindowStyle *		windowStyle() const;				// window style

	float				activeTime() const;					// how long as this window been active
	float				visibleTime() const;				// how long has this window been visible
	float				targetAlpha() const;

	SizeInt				contentSize() const;				// calculate the size of this windows contents
	Resource *			resource( const char * id,
							bool a_bBlocking = true ) const;	// this allows additional resources to be attached to this window
															// without writing special mutators or accessors

	// Mutators
	void				setStyle( dword style );
	void				setFlags( dword flags );
	
	void				setWindow( const RectInt & window );
	void				setWindowSize( const SizeInt & size );	// change window size, leave upper left corner in current position
	void				setWindowWidth( int width );
	void				setWindowHeight( int height );
	void				checkPlacement();

	void				setOffset( const PointInt & offset );
	void				setAlpha( float alpha );
	void				setTargetAlpha( float alpha );		
	void				setHelp( const char * pHelp );
	void				setScript( const char * pScript );
	void				setWindowStyle( WindowStyle::Link pStyle );

	void				addResource( const char * id, 
							Resource::Link pResource );			// add resource to this window

	void				destroy();							// close this window
	
	void				setActiveTime( float t );			// set the active time only on this window
	void				resetActiveTime();					// zeros the active time on this window and all child windows
	void				setVisibleTime( float t );

	// client interface
	virtual void		onShow();							// called after the window/parent is made visible
	virtual void		onHide();							// called before the window/parent is hidden

	virtual void		onEnable();							// called after the window/parent is enabled
	virtual void		onDisable();						// called before the window/parent is disabled

	virtual void		onRender( RenderContext & context,
							const RectInt & window );		// client render, window is in screen coordinates
	virtual bool		onCursorMove( const RectInt & window, 
							const PointInt & position,		// window and position are local coordinates
							const PointInt & delta );
	virtual bool		onLeftDown( const RectInt & window, 
							const PointInt & position );
	virtual bool		onLeftUp( const RectInt & window, 
							const PointInt & position );
	virtual bool		onRightDown( const RectInt & window,
							const PointInt & position );
	virtual bool		onRightUp( const RectInt & window,
							const PointInt & position );
	
	virtual Color		backColor() const;					// window background color
	virtual Color		borderColor() const;				// window border color
	virtual int			borderSize() const;					// border size for this window

	// Helpers
	bool				visible() const;					// is this window visible

	void				playSound( const char * id );
	void				hoverSound();
	void				selectSound();
	void				setCursorState( CursorState state );
	void				setCursorTip( const char * pTip );
	void				setVisible( bool visible );			// make this window visible or not visible
	void				setEnable( bool enable );			// enable or disable this window
	
	void				hideWindow( bool enable = false );	// make this window not visible but enabled if true, or disabled if false (default)
	void				showWindow( bool show = true );		// make this window enabled and visible if true, or disabled and not visible if false
	
	void				hideChildren();						// make all children windows of this window hidden and disabled
	void				destroyChildren();					// destroy all children windows
	void				cullChildren( int keep );			// remove any children over keep

	RectInt				localWindow();						// get the local window, left and top are zero
	RectInt				screenWindow();						// get the window location in screen space
	SizeInt				contentSize();
	NodeWindow *		parentWindow() const;
	NodeWindow *		rootWindow() const;
	
	PointInt			screenToWindow( const PointInt & point );
	PointInt			windowToScreen( const PointInt & point );

	void				renderGlow( RenderContext & context );

	// Static
	static void			pushBackground( RenderContext & context, const RectInt & window, 
							Color color, dword style, int bs );
	static void			pushBorder( RenderContext & context, const RectInt & window, 
							Color lightColor, Color shadeColor, dword style, int bs );
	static void			pushGlow( RenderContext & context, const RectInt & window, int size,
							Color innerColor, Color outerColor, dword style, int bs );
		
	// Static data
	static float		s_Alpha;							// affect the alpha of all windows
	static float		s_RollSpeed;						// set the UV roll speed

protected:
	// Types
	typedef Hash< dword, ResourceLink<Resource> >	
											ResourceHash;
	typedef ResourceHash::Iterator			ResourceHashIt;

	// Data
	dword				m_Style;							// window style
	dword				m_Flags;							// state flags
	RectInt				m_Window;							// window position
	PointInt			m_Offset;							// offset contents 
	float				m_Alpha;							// window alpha
	CharString			m_Help;								// help text
	CharString			m_sScript;							// script code

	WindowStyle::Link	m_WindowStyle;						// window style resources
	ResourceHash		m_Resources;						// additional window resources ( sounds, images, etc.. )

	// non-serialized
	float				m_ActiveTime;						// how long has this window been enabled
	float				m_VisibleTime;						// how long has this window been visible
	float				m_ActionTime;
	float				m_Flare;
	float				m_TargetAlpha;			

	// Mutators
	bool				cursorMove( const PointInt & position,
							const PointInt & delta );
	bool				mouseWheel( const PointInt & position,
							int delta );
	bool				leftDown( const PointInt & position );
	bool				leftUp( const PointInt & position );

	// Static
	static void			onShow( BaseNode * pNode );
	static void			onHide( BaseNode * pNode );
	static void			onEnable( BaseNode * pNode );
	static void			onDisable( BaseNode * pNode );
};

//-------------------------------------------------------------------------------

inline dword NodeWindow::style() const
{
	return( m_Style );
}

inline dword NodeWindow::flags() const
{
	return( m_Flags );
}

inline const RectInt & NodeWindow::window() const
{
	return( m_Window );
}

inline SizeInt NodeWindow::size() const
{
	return m_Window.size();
}

inline const PointInt & NodeWindow::offset() const
{
	return( m_Offset );
}

inline float NodeWindow::alpha() const
{
	return( m_Alpha );
}

inline const char * NodeWindow::help() const
{
	return m_Help;
}

inline const char * NodeWindow::script() const
{
	return m_sScript;
}

inline float NodeWindow::activeTime() const
{
	return m_ActiveTime;
}

inline float NodeWindow::visibleTime() const
{
	return m_VisibleTime;
}

inline float NodeWindow::targetAlpha() const
{
	return m_TargetAlpha;
}

//----------------------------------------------------------------------------

inline void NodeWindow::setStyle( dword style )
{
	m_Style = style;
}

inline void NodeWindow::setFlags( dword flags )
{
	m_Flags = flags;
}

inline void NodeWindow::setWindow( const RectInt & window )
{
	m_Window = window;
}

inline void NodeWindow::setWindowSize( const SizeInt & size )
{
	m_Window.setWidth( size.width );
	m_Window.setHeight( size.height );
}

inline void NodeWindow::setWindowWidth( int width )
{
	m_Window.setWidth( width );
}

inline void NodeWindow::setWindowHeight( int height )
{
	m_Window.setHeight( height );
}

inline void NodeWindow::setOffset( const PointInt & offset )
{
	m_Offset = offset;
}

inline void NodeWindow::setAlpha( float alpha )
{
	m_TargetAlpha = m_Alpha = alpha;
}

inline void NodeWindow::setTargetAlpha( float alpha )
{
	m_TargetAlpha = alpha;
}

inline void NodeWindow::setHelp( const char * pHelp )
{
	m_Help = pHelp;
}

inline void NodeWindow::setScript( const char * pScript )
{
	m_sScript = pScript;
}

inline void NodeWindow::setWindowStyle( WindowStyle::Link pStyle )
{
	m_WindowStyle = pStyle;
}

inline void NodeWindow::setActiveTime( float t )
{
	m_ActiveTime = t;
}

inline void NodeWindow::setVisibleTime( float t )
{
	m_VisibleTime = t;
}

//----------------------------------------------------------------------------



#endif

//----------------------------------------------------------------------------
// EOF

